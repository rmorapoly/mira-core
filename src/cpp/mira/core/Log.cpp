#include "Log.h"
#include "Object.h"

#include <QByteArray>
#include <QMutex>
#include <QMutexLocker>
#include <QVector>

#include <cstdio>
#include <cstdlib>

namespace Mira {
    namespace Core {

        namespace {

            /// Default pattern: time, severity, location, object context and message.
            const QString defaultPattern()
            {
                return QStringLiteral(
                            "[%{time yyyy-MM-dd hh:mm:ss.zzz}] "
                            "[%{type}] "
                            // "[%{object}] "
                            "%{if-object}[%{object}] %{endif}"
                            // "[%{objectName}:%{objectInstanceId}] [%{objectInstanceName}] "
                            "%{message}");
            }

            /// Qt's built-in default message pattern, hard-coded in Qt's
            /// `qlogging.cpp` (Qt offers no getter for it). An *empty* pattern is not
            /// equivalent: it makes `qFormatLogMessage()` return an empty string, so
            /// nothing is printed at all.
            const QString qtDefaultPattern()
            {
                return QStringLiteral("%{if-category}%{category}: %{endif}%{message}");
            }

            /// Mira's pattern, with the `%{object*}` placeholders still unresolved.
            QString miraPattern = defaultPattern();

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
            bool isMiraCondition(const QString& token)
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
            bool miraCondition(const QString& token, const ObjectFields& fields)
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

            /// Resolves one Mira `%{object*}` placeholder into @p value. Returns false
            /// for any other placeholder, which is then left for Qt.
            bool miraField(const QString& token, const ObjectFields& fields, QString* value)
            {
                if (token == QLatin1String("object"))
                    *value = fields.object;
                else if (token == QLatin1String("objectName"))
                    *value = fields.name;
                else if (token == QLatin1String("objectInstanceId"))
                    *value = fields.instanceId;
                else if (token == QLatin1String("objectInstanceName"))
                    *value = fields.instanceName;
                else
                    return false;
                return true;
            }

            /// Expands the Mira placeholders of @p pattern: the `%{object*}` tokens and
            /// the Mira `%{if-object*}...%{endif}` conditional blocks. Every other
            /// placeholder (Qt's own, including `%{if-category}...%{endif}`) is left
            /// untouched for `qFormatLogMessage()` to expand.
            ///
            /// Mira's conditions are evaluated here, at pattern-selection time, because
            /// Qt has no knowledge of them. Their markers are consumed; Qt's own
            /// conditional markers are preserved, so both can be mixed in one pattern.
            QString expandObjectTokens(const QString& pattern, const QObject* obj)
            {
                const ObjectFields fields = resolveObjectFields(obj);

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
                        const bool ours = isMiraCondition(token);
                        // Foreign conditions are not ours to evaluate; assume true and
                        // preserve the markers so Qt evaluates them itself.
                        stack.append(ConditionFrame{ours ? miraCondition(token, fields) : true, ours});

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
                        if (miraField(token, fields, &value)) {
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
            /// The write is skipped while the same object stays current, so a run of
            /// messages (from one object, or from none at all) never touches Qt's state.
            /// `Mira::Core::Object`'s fields are const, so one pointer identifies one
            /// resolved form.
            void selectQtPattern(const QString& pattern, const QObject* obj)
            {
                if (qtPatternValid && obj == qtPatternObject)
                    return;

                qSetMessagePattern(expandObjectTokens(pattern, obj));
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

                insideHandler = false;

                if (type == QtFatalMsg)
                    abort();
            }

        } // namespace

        void setLogPattern(const QString& pattern)
        {
            QMutexLocker locker(&patternMutex);

            miraPattern = pattern;

            // The stored pattern changed, so any cached expansion is stale. Push the
            // placeholder-free form so foreign handlers always have a valid pattern.
            qtPatternValid = false;
            selectQtPattern(pattern, nullptr);
        }

        QString logPattern()
        {
            QMutexLocker locker(&patternMutex);
            return miraPattern;
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

            // Mira fills the object placeholders, then Qt fills the rest. Qt's pattern
            // is rewritten only when the emitting object changes (see selectQtPattern).
            selectQtPattern(miraPattern, object);
            return qFormatLogMessage(type, context, message);
        }

        void installLogHandler()
        {
            setLogPattern(defaultPattern());

            QMutexLocker locker(&handlerMutex);
            if (handlerInstalled)
                return;

            qInfo() << "installed";
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

            // Reverse installLogHandler(): hand Qt its own default pattern back, so its
            // handler (and any other) behaves as if Mira had never touched the pattern.
            QMutexLocker patternLocker(&patternMutex);
            qSetMessagePattern(qtDefaultPattern());
            qtPatternObject = nullptr;
            qtPatternValid = false;
        }
    } // namespace Core
} // namespace Mira
