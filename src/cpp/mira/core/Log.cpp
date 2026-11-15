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

        const QString& defaultPattern()
        {
            // The message sequence number is part of the default: every message
            // Mira writes carries its row count unless a custom pattern leaves
            // the token out again.
            static const QString pattern = QStringLiteral(
                        "[%{time yyyy-MM-dd hh:mm:ss.zzz}] "
                        "[%{type}] "
                        "[%{messageIndex}] "
                        "%{if-object}[%{object}] %{endif}"
                        "%{message}");
            return pattern;
        }

        const QString& qtDefaultPattern()
        {
            // Qt's built-in default message pattern, hard-coded in Qt's
            // `qlogging.cpp` (Qt offers no getter for it). An *empty* pattern is not
            // equivalent: it makes `qFormatLogMessage()` return an empty string, so
            // nothing is printed at all.
            static const QString pattern = QStringLiteral("%{if-category}%{category}: %{endif}%{message}");
            return pattern;
        }

        namespace {

            /// The placeholder Mira fills with the message sequence number.
            const QLatin1String messageIndexToken("messageIndex");

            /// The pattern currently in effect, and whether it uses
            /// `%{messageIndex}` (which decides if Qt's pattern has to be
            /// re-rendered per message or only per emitting object).
            struct PatternState
            {
                QString pattern;
                bool usesMessageIndex = false;
            };

            /// Mira's pattern, with the `%{object*}` and `%{messageIndex}`
            /// placeholders still unresolved. A function-local static: built on
            /// first use (initially @ref defaultPattern), so no QString is
            /// constructed during static initialization. @ref patternMutex guards
            /// it.
            PatternState& patternState()
            {
                static PatternState state = [] {
                    PatternState s;
                    s.pattern = defaultPattern();
                    s.usesMessageIndex = s.pattern.contains(messageIndexToken);
                    return s;
                }();
                return state;
            }

            /// Serialises updates of Qt's process-wide pattern with the calls that read
            /// it back through `qFormatLogMessage()`.
            ///
            /// Recursive because an invalid pattern makes Qt report an error, which
            /// re-enters `formatLogMessage()` through the handler.
            QRecursiveMutex patternMutex;

            /// Object whose `%{object*}` values are currently resolved into Qt's
            /// pattern; `nullptr` means the no-object (`unknown`) form is in place.
            const QObject* qtPatternObject = nullptr;

            /// Whether Qt's pattern really corresponds to @ref qtPatternObject.
            bool qtPatternValid = false;

            /// Message handler that was active before Mira installed its own.
            QtMessageHandler previousHandler = nullptr;

            /// Whether Mira's message handler is currently installed.
            bool handlerInstalled = false;

            /// Guards the handler install/uninstall state.
            QMutex handlerMutex;

            /// Sequence number of the last message Mira formatted: the row count
            /// behind `%{messageIndex}`. Advanced once per formatted message,
            /// shared across threads (relaxed - ordering only matters between
            /// the value and the pattern it is rendered into, which
            /// @ref patternMutex already serialises).
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

            /// Resolves one Mira placeholder into @p value: the `%{object*}`
            /// tokens and `%{messageIndex}`. Returns false for any other
            /// placeholder, which is then left for Qt.
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

            /// Expands the Mira placeholders of @p pattern: the `%{object*}` and
            /// `%{messageIndex}` tokens and the Mira `%{if-object*}...%{endif}`
            /// conditional blocks. Every other placeholder (Qt's own, including
            /// `%{if-category}...%{endif}`) is left untouched for
            /// `qFormatLogMessage()` to expand.
            ///
            /// Mira's conditions are evaluated here, at pattern-selection time, because
            /// Qt has no knowledge of them. Their markers are consumed; Qt's own
            /// conditional markers are preserved, so both can be mixed in one pattern.
            QString expandTokens(const QString& pattern,
                                 const ObjectFields& fields,
                                 const QString& messageIndex)
            {
                // One entry per open `%{if-*}`: the truth value of Mira's condition (or
                // `true` for Qt's, which are evaluated later by Qt), and whether the
                // markers belong to Mira (and are therefore consumed here).
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
                expanded.reserve(pattern.size());

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
                        // Unterminated placeholder: Qt keeps the rest as literal text.
                        if (emitting())
                            expanded += pattern.mid(i);
                        break;
                    }

                    const QString token = pattern.mid(i + 2, close - i - 2);
                    const QString marker = pattern.mid(i, close - i + 1);
                    i = close + 1;

                    if (token.startsWith(QLatin1String("if-"))) {
                        const bool ours = isCondition(token);
                        // Foreign conditions are not ours to evaluate; assume true and
                        // preserve the markers so Qt evaluates them itself.
                        stack.append(ConditionFrame{ours ? evalCondition(token, fields) : true, ours});

                        if (!ours && emitting())
                            expanded += marker;
                    } else if (token == QLatin1String("endif")) {
                        if (!stack.isEmpty()) {
                            const bool ours = stack.last().ours;
                            stack.removeLast();

                            if (!ours && emitting())
                                expanded += marker;
                        } else if (emitting()) {
                            // Stray `%{endif}`: leave it for Qt to report.
                            expanded += marker;
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

                return expanded;
            }

            /// Resolves the Mira placeholders of @p pattern for @p obj into Qt's
            /// process-wide pattern. @ref patternMutex must be held.
            ///
            /// Callers decide when this is needed: unchanged emitting objects reuse
            /// the cached expansion, and only a pattern with `%{messageIndex}`
            /// re-renders per message (the index cannot be cached).
            void selectQtPattern(const QString& pattern, const QObject* obj, const QString& messageIndex)
            {
                qSetMessagePattern(expandTokens(pattern, resolveObjectFields(obj), messageIndex));
                qtPatternObject = obj;
                qtPatternValid = true;
            }

            void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
            {
                // Re-entrancy guard: an invalid pattern makes Qt report an error, which
                // would otherwise call us back.
                if (insideHandler) {
                    const QByteArray raw = msg.toLocal8Bit();
                    fprintf(stderr, "%s\n", raw.constData());
                    fflush(stderr);

                    return;
                }

                insideHandler = true;

                const QString formatted = formatLogMessage(type, context, msg, Details::currentObject());
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

        void setLogPattern(const QString& pattern)
        {
            QMutexLocker locker(&patternMutex);

            PatternState& state = patternState();
            state.pattern = pattern;
            state.usesMessageIndex = pattern.contains(messageIndexToken);

            // The stored pattern changed, so any cached expansion is stale. Push the
            // placeholder-free form (with the index frozen at the last value Mira
            // issued) so foreign handlers always have a valid pattern.
            const QString index = state.usesMessageIndex
                    ? QString::number(messageCounter.load(std::memory_order_relaxed))
                    : QString();

            selectQtPattern(pattern, nullptr, index);
        }

        QString logPattern()
        {
            QMutexLocker locker(&patternMutex);
            return patternState().pattern;
        }

        const QObject* currentLogObject()
        {
            return Details::currentObject();
        }

        QString formatLogMessage(QtMsgType type,
                                 const QMessageLogContext& context,
                                 const QString& message,
                                 const QObject* object)
        {
            QMutexLocker locker(&patternMutex);

            const PatternState& state = patternState();

            if (state.usesMessageIndex) {
                // The index advances with every message, so nothing here can be
                // cached: Qt's process-wide pattern is re-rendered for this one
                // message. That is the price of the token.
                selectQtPattern(state.pattern, object, QString::number(++messageCounter));
            } else if (!qtPatternValid || object != qtPatternObject) {
                // Mira fills the object placeholders, then Qt fills the rest. Qt's
                // pattern is rewritten only when the emitting object changes, so a
                // run of messages never touches Qt's state.
                selectQtPattern(state.pattern, object, QString());
            }

            return qFormatLogMessage(type, context, message);
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

        void installLogHandler()
        {
            QMutexLocker locker(&handlerMutex);
            if (handlerInstalled)
                return;

            qInfo() << "installed";

            // The pattern is deliberately left alone: whatever setLogPattern last
            // set (defaultPattern until then) stays in effect, a pattern set
            // before installing is not overwritten. Neither install nor uninstall
            // ever changes the pattern - the application owns it.
            previousHandler = qInstallMessageHandler(messageHandler);
            handlerInstalled = true;
        }

        void reinstallLogHandler()
        {
            uninstallLogHandler();
            installLogHandler();
        }

        void uninstallLogHandler()
        {
            QMutexLocker locker(&handlerMutex);
            if (!handlerInstalled)
                return;

            qInfo() << "uninstalled";
            qInstallMessageHandler(previousHandler);
            previousHandler = nullptr;
            handlerInstalled = false;

            // The pattern is deliberately left alone, just like on install: the
            // application owns it. Qt's process-wide pattern keeps the last
            // placeholder-free form Mira rendered, so whatever handler runs next
            // formats a valid pattern. Use setLogPattern(qtDefaultPattern()) to
            // hand Qt's own formatting back explicitly.
        }
    } // namespace Core
} // namespace Mira
