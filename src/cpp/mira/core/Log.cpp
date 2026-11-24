#include "Log.h"
#include "Object.h"

#include <QByteArray>
#include <QMutex>
#include <QMutexLocker>
#include <QVector>

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <utility>

namespace Mira {
    namespace Core {

        const QString& defaultMessagePattern()
        {
            // Mira only contributes its own part of the line: the row count, the
            // object context and the message. Time, severity and every other Qt
            // token belong to Qt's process-wide pattern, which the application sets
            // itself with qSetMessagePattern() / QT_MESSAGE_PATTERN.
            static const QString pattern = QStringLiteral(
                        "[%{messageIndex}] "
                        "%{if-object}[%{object}] %{endif}"
                        "%{message}");
            return pattern;
        }

        const QString& qtDefaultMessagePattern()
        {
            // Qt's built-in default message pattern, hard-coded in Qt's
            // `qlogging.cpp` (Qt offers no getter for it). It is what Qt renders
            // around Mira's part until the application replaces it, and Mira never
            // touches it.
            static const QString pattern = QStringLiteral("%{if-category}%{category}: %{endif}%{message}");
            return pattern;
        }

        namespace {

            /// Mira's pattern: the part of the line Mira expands. A function-local
            /// static, so no QString is constructed during static initialization;
            /// built on first use (initially @ref defaultMessagePattern).
            /// @ref patternMutex guards it.
            QString& patternStorage()
            {
                static QString pattern = defaultMessagePattern();
                return pattern;
            }

            /// Serialises reading and replacing Mira's pattern.
            ///
            /// Deliberately not held while `qFormatLogMessage()` runs: an invalid
            /// Qt pattern makes Qt report an error, which re-enters the handler, and
            /// that must never wait on this mutex.
            QMutex patternMutex;

            /// Message handler that was active before Mira installed its own.
            QtMessageHandler previousHandler = nullptr;

            /// Whether Mira's message handler is currently installed.
            bool handlerInstalled = false;

            /// Guards the handler install/uninstall state.
            QMutex handlerMutex;

            /// Number of messages printed so far: the row count behind
            /// `%{messageIndex}`. Advanced exactly once for every message Mira
            /// formats - whether it goes through the handler or through a direct
            /// @ref formatMessage call - so a number is never issued twice. Shared
            /// across threads; a relaxed fetch_add is enough, because each message
            /// only needs a unique value out of it.
            std::atomic<quint64> messageCounter{0};

            /// Guards @ref extraSink. Kept separate from the handler and pattern
            /// mutexes so a sink that takes its time (or a log arriving while the
            /// handler is installed/uninstalled) can never lock up in two
            /// different orders.
            QMutex sinkMutex;

            /// The sink registered through `setLogSink()`; null when unset.
            /// Copied under @ref sinkMutex, invoked without it.
            std::shared_ptr<const LogSink> extraSink;

            /// Whether a sink is registered at all: one relaxed load keeps the
            /// sink-less hot path free of any locking.
            std::atomic<bool> sinkEnabled{false};

            /// Guards against recursion while Qt reports an invalid pattern.
            thread_local bool insideHandler = false;

            /// The Mira fields rendered into the pattern for one emitting object.
            /// Without a `Mira::Core::Object` every field reads `unknown`.
            struct ObjectFields
            {
                QString name;
                QString instanceId;
                QString instanceName;
                QString object;
                bool hasObject = false;
            };

            ObjectFields resolveObjectFields(const QObject* obj)
            {
                const Object* object = dynamic_cast<const Object*>(obj);

                ObjectFields fields;
                fields.name = QStringLiteral("unknown");
                fields.instanceId = QStringLiteral("unknown");
                fields.instanceName = QStringLiteral("unknown");
                fields.object = QStringLiteral("unknown");

                if (object) {
                    fields.hasObject = true;
                    fields.name = object->name;
                    fields.instanceId = QString::number(object->instanceId);
                    fields.instanceName = object->instanceName;
                    fields.object = QStringLiteral("%1:%2").arg(fields.name, fields.instanceId);
                    if (!fields.instanceName.isEmpty())
                        fields.object += QStringLiteral("#%1").arg(fields.instanceName);
                }
                return fields;
            }

            /// Whether @p token is one of Mira's `%{if-*}` conditional tokens.
            bool isCondition(const QString& token)
            {
                return token == QLatin1String("if-object")
                        || token == QLatin1String("if-objectName")
                        || token == QLatin1String("if-objectInstanceId")
                        || token == QLatin1String("if-objectInstanceName");
            }

            /// Evaluates one of Mira's `%{if-*}` conditional tokens. A token is true
            /// when the matching `%{object*}` placeholder resolves to more than the
            /// `unknown` fallback: an object context must exist, and for the two name
            /// tokens the field must additionally be non-empty.
            bool evalCondition(const QString& token, const ObjectFields& fields)
            {
                if (token == QLatin1String("if-object"))
                    return fields.hasObject;
                if (token == QLatin1String("if-objectName"))
                    return fields.hasObject && !fields.name.isEmpty();
                if (token == QLatin1String("if-objectInstanceId"))
                    return fields.hasObject;
                if (token == QLatin1String("if-objectInstanceName"))
                    return fields.hasObject && !fields.instanceName.isEmpty();

                return false;
            }

            /// Resolves one Mira placeholder into @p value: the `%{object*}` tokens
            /// and `%{messageIndex}`. Returns false for any other placeholder, which
            /// is then left alone.
            bool resolveField(const QString& token,
                              const ObjectFields& fields,
                              const QString& messageIndex,
                              QString* value)
            {
                if (token == QLatin1String("object"))
                    *value = fields.object;
                else if (token == QLatin1String("objectName"))
                    *value = fields.name;
                else if (token == QLatin1String("objectInstanceId"))
                    *value = fields.instanceId;
                else if (token == QLatin1String("objectInstanceName"))
                    *value = fields.instanceName;
                else if (token == QLatin1String("messageIndex"))
                    *value = messageIndex;
                else
                    return false;
                return true;
            }

            /// Expands Mira's part of the line: the `%{object*}` tokens,
            /// `%{messageIndex}`, `%{message}` and the Mira
            /// `%{if-object*}...%{endif}` conditional blocks.
            ///
            /// Every other placeholder is left as written: Qt's tokens are not
            /// Mira's to expand (they belong to Qt's own pattern), and Qt would
            /// print them literally here, so a Qt token in Mira's pattern is
            /// visible in the output instead of silently doing nothing.
            ///
            /// Mira's conditions are evaluated here, at expansion time, because Qt
            /// never sees them.
            QString expandTokens(const QString& pattern,
                                 const ObjectFields& fields,
                                 const QString& messageIndex,
                                 const QString& message)
            {
                // One entry per open `%{if-*}`: whether the condition is true, and
                // whether the markers are Mira's (consumed here) or a Qt token
                // left in the pattern (printed literally).
                struct ConditionFrame
                {
                    bool enabled;
                    bool ours;
                };
                QVector<ConditionFrame> stack;

                auto emitting = [&stack]() {
                    for (const ConditionFrame& frame : stack)
                        if (!frame.enabled)
                            return false;
                    return true;
                };

                QString expanded;
                expanded.reserve(pattern.size() + message.size());

                // Whether `%{message}` was part of the emitted text. A pattern that
                // leaves it out is a prefix, and the message is appended below.
                bool messageFilled = false;

                const int size = pattern.size();
                int i = 0;
                while (i < size) {
                    const QChar c = pattern.at(i);

                    if (c != QLatin1Char('%') || i + 1 >= size || pattern.at(i + 1) != QLatin1Char('{')) {
                        if (emitting())
                            expanded += c;
                        ++i;
                        continue;
                    }

                    const int close = pattern.indexOf(QLatin1Char('}'), i + 2);
                    if (close < 0) {
                        // Unterminated placeholder: keep the rest as literal text.
                        if (emitting())
                            expanded += pattern.mid(i);
                        break;
                    }

                    const QString token = pattern.mid(i + 2, close - i - 2);
                    const QString marker = pattern.mid(i, close - i + 1);
                    i = close + 1;

                    if (token.startsWith(QLatin1String("if-"))) {
                        const bool ours = isCondition(token);
                        stack.append(ConditionFrame{ours ? evalCondition(token, fields) : true, ours});

                        // Qt's own conditions are not Mira's: they are printed
                        // literally, like any other Qt token in Mira's pattern.
                        if (!ours && emitting())
                            expanded += marker;
                    } else if (token == QLatin1String("endif")) {
                        if (!stack.isEmpty()) {
                            const bool ours = stack.last().ours;
                            stack.removeLast();

                            if (!ours && emitting())
                                expanded += marker;
                        } else if (emitting()) {
                            expanded += marker;    // stray `%{endif}`: keep it visible
                        }
                    } else if (token == QLatin1String("message")) {
                        if (emitting()) {
                            expanded += message;
                            messageFilled = true;
                        }
                    } else {
                        QString value;
                        if (resolveField(token, fields, messageIndex, &value)) {
                            if (emitting())
                                expanded += value;
                        } else if (emitting()) {
                            expanded += marker;
                        }
                    }
                }

                if (!messageFilled)
                    expanded += message;

                return expanded;
            }

            void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
            {
                // Re-entrancy guard: an invalid Qt pattern makes Qt report an error,
                // which would otherwise call us back.
                if (insideHandler) {
                    const QByteArray raw = msg.toLocal8Bit();
                    fprintf(stderr, "%s\n", raw.constData());
                    fflush(stderr);
                    return;
                }

                insideHandler = true;

                const QString formatted = formatMessage(type, context, msg, Details::currentObject());
                const QByteArray out = formatted.toLocal8Bit();
                fprintf(stderr, "%s\n", out.constData());
                fflush(stderr);

                // The extra sink sees exactly what was written to the console. The
                // shared pointer is copied under the mutex and invoked without it,
                // so a sink can take its time without holding up other threads.
                if (sinkEnabled.load(std::memory_order_relaxed)) {
                    std::shared_ptr<const LogSink> sink;
                    {
                        QMutexLocker locker(&sinkMutex);
                        sink = extraSink;
                    }
                    if (sink)
                        (*sink)(type, context, formatted);
                }

                insideHandler = false;

                if (type == QtFatalMsg)
                    abort();
            }

        } // namespace

        void setMessagePattern(const QString& pattern)
        {
            QMutexLocker locker(&patternMutex);
            patternStorage() = pattern;
        }

        QString messagePattern()
        {
            QMutexLocker locker(&patternMutex);
            return patternStorage();
        }

        const QObject* currentLogObject()
        {
            return Details::currentObject();
        }

        QString formatMessage(QtMsgType type,
                              const QMessageLogContext& context,
                              const QString& message,
                              const QObject* object)
        {
            // One call is one printed message: it takes the next number.
            const QString index = QString::number(++messageCounter);

            QString miraPart;
            {
                QMutexLocker locker(&patternMutex);
                miraPart = expandTokens(patternStorage(), resolveObjectFields(object), index, message);
            }

            // Mira's expansion becomes the message text for Qt, so Qt's own
            // process-wide pattern renders the line around it: {Qt pattern}, with
            // its %{message} replaced by {Mira pattern} + message. Qt's pattern is
            // read, never written - it belongs to the application.
            return qFormatLogMessage(type, context, miraPart);
        }

        void setLogSink(LogSink sink)
        {
            std::shared_ptr<const LogSink> next;
            if (sink)
                next = std::make_shared<const LogSink>(std::move(sink));

            QMutexLocker locker(&sinkMutex);
            extraSink = std::move(next);
            sinkEnabled.store(static_cast<bool>(extraSink), std::memory_order_relaxed);
        }

        void installMessageHandler()
        {
            QMutexLocker locker(&handlerMutex);
            if (handlerInstalled)
                return;

            qInfo() << "installed";

            // Neither Mira's pattern nor Qt's process-wide pattern is touched:
            // both belong to the application, and neither install nor uninstall
            // ever changes them.
            previousHandler = qInstallMessageHandler(messageHandler);
            handlerInstalled = true;
        }

        void reinstallMessageHandler()
        {
            uninstallMessageHandler();
            installMessageHandler();
        }

        void uninstallMessageHandler()
        {
            QMutexLocker locker(&handlerMutex);
            if (!handlerInstalled)
                return;

            qInfo() << "uninstalled";
            qInstallMessageHandler(previousHandler);
            previousHandler = nullptr;
            handlerInstalled = false;

            // Both patterns are deliberately left alone, just like on install: the
            // application owns them, so a later install picks up where it left off.
        }
    } // namespace Core
} // namespace Mira
