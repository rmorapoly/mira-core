#include "Log.h"
#include "Object.h"

#include <QByteArray>
#include <QMutex>
#include <QMutexLocker>

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
                            "[%{object}] "
                            // "[%{objectName}:%{objectInstanceId}] [%{objectInstanceName}] "
                            "%{message}");
            }

            QString g_pattern = defaultPattern();
            QMutex g_patternMutex;

            /// Message handler that was active before Mira installed its own.
            QtMessageHandler g_previousHandler = nullptr;

            /// Whether Mira's message handler is currently installed.
            bool g_handlerInstalled = false;

            /// Guards the handler install/uninstall state.
            QMutex g_handlerMutex;

            /// The `QObject` emitting the message currently being dispatched (per thread).
            thread_local const QObject* g_currentObject = nullptr;

            /// Guards against recursion when `qFormatLogMessage()` reports a bad pattern.
            thread_local bool g_inHandler = false;

            /// Expands the Mira-specific placeholders of @p pattern using @p obj.
            QString expandObjectTokens(const QString& pattern, const QObject* obj)
            {
                const Object* object = dynamic_cast<const Object*>(obj);

                const QString name = object ? object->name : "unknown";
                const QString instanceId = object ? QString::number(object->instanceId) : "unknown";
                const QString instanceName = object ? object->instanceName : "unknown";

                QString objectField = "unknown";
                if (object) {
                    objectField = QStringLiteral("%1:%2").arg(name, instanceId);
                    if (!instanceName.isEmpty())
                        objectField += QStringLiteral("#%1").arg(instanceName);
                }

                QString expanded = pattern;
                expanded.replace(QLatin1String("%{object}"), objectField);
                expanded.replace(QLatin1String("%{objectName}"), name);
                expanded.replace(QLatin1String("%{objectInstanceId}"), instanceId);
                expanded.replace(QLatin1String("%{objectInstanceName}"), instanceName);
                return expanded;
            }

            void messageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
            {
                // Re-entrancy guard: qFormatLogMessage() may qWarning() about an invalid
                // pattern, which would otherwise call us back.
                if (g_inHandler) {
                    const QByteArray raw = msg.toLocal8Bit();
                    fprintf(stderr, "%s\n", raw.constData());
                    return;
                }

                g_inHandler = true;

                const QString formatted = formatLogMessage(type, context, msg, g_currentObject);
                const QByteArray out = formatted.toLocal8Bit();
                fprintf(stderr, "%s\n", out.constData());
                fflush(stderr);

                g_inHandler = false;

                if (type == QtFatalMsg)
                    abort();
            }

        } // namespace

        void setLogPattern(const QString& pattern)
        {
            QMutexLocker locker(&g_patternMutex);
            g_pattern = pattern;
        }

        QString logPattern()
        {
            QMutexLocker locker(&g_patternMutex);
            return g_pattern;
        }

        const QObject* currentLogObject()
        {
            return g_currentObject;
        }

        QString formatLogMessage(QtMsgType type,
                                 const QMessageLogContext& context,
                                 const QString& message,
                                 const QObject* object)
        {
            QMutexLocker locker(&g_patternMutex);

            // Substitute the Mira placeholders in the pattern, then let Qt expand the
            // remaining standard placeholders.
            qSetMessagePattern(expandObjectTokens(g_pattern, object));
            const QString formatted = qFormatLogMessage(type, context, message);

            // Restore a sanitized pattern so no per-message values leak into subsequent
            // calls. The Mira placeholders are stripped here: Qt's qSetMessagePattern()
            // emits a warning for placeholders it does not know.
            qSetMessagePattern(expandObjectTokens(g_pattern, nullptr));

            return formatted;
        }

        void installLogHandler()
        {
            setLogPattern(defaultPattern());

            QMutexLocker locker(&g_handlerMutex);
            if (g_handlerInstalled)
                return;

            g_previousHandler = qInstallMessageHandler(messageHandler);
            g_handlerInstalled = true;
        }

        void reinstallLogHandler()
        {
            uninstallLogHandler();
            installLogHandler();
        }

        void uninstallLogHandler()
        {
            QMutexLocker locker(&g_handlerMutex);
            if (!g_handlerInstalled)
                return;

            qInstallMessageHandler(g_previousHandler);
            g_previousHandler = nullptr;
            g_handlerInstalled = false;
        }

        LogStream::LogStream(QtMsgType type, const QObject* obj) :
            dbg(new QDebug(&buffer)),
            type(type)
        {
            dbg->noquote();
            g_currentObject = obj;
        }

        LogStream::~LogStream()
        {
            // Destroying the QDebug flushes the internal text stream into `buffer`.
            delete dbg;
            dbg = nullptr;

            if (hasContent)
                dispatch(buffer.trimmed());

            g_currentObject = nullptr;
        }

        void LogStream::dispatch(const QString& message) const
        {
            switch (type) {
                case QtDebugMsg:
                    qDebug().noquote() << message;
                    break;
                case QtInfoMsg:
                    qInfo().noquote() << message;
                    break;
                case QtWarningMsg:
                    qWarning().noquote() << message;
                    break;
                case QtCriticalMsg:
                    qCritical().noquote() << message;
                    break;
                case QtFatalMsg:
                    /// aborts the program immediately (via the message handler)
                    qFatal("%s", message.toUtf8().constData());
                    break;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
                case QtSystemMsg:
                    qInfo().noquote() << message;
                    break;
#endif
                default:
                    qInfo().noquote() << message;
                    break;
            }
        }
    } // namespace Core
} // namespace Mira
