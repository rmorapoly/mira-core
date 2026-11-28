#pragma once

#include <QDebug>
#include <QString>
#include <QtCore/qlogging.h>
#include <functional>

#include "mira/core/Import.h"

class QObject;

namespace Mira {
    namespace Core {

        /**
         * @brief Returns Mira's default message pattern.
         *
         * Mira's pattern is only the Mira part of the line: the message sequence
         * number (`%{messageIndex}`), the object context
         * (`%{if-object}[%{object}] %{endif}`) and the message itself. Everything
         * Qt-specific - `%{time}`, `%{type}`, `%{file}`, `%{category}`, ... - is
         * rendered by Qt from the pattern the application installed with
         * `qSetMessagePattern()` / `QT_MESSAGE_PATTERN`.
         *
         * Intended for use with @ref setMessagePattern, e.g. to return to the
         * default after a custom pattern:
         * @code
         * Mira::Core::setMessagePattern(Mira::Core::defaultMessagePattern());
         * @endcode
         *
         * @return The default pattern (never empty).
         */
        MIRA_CORE_EXPORT const QString& defaultMessagePattern();

        /**
         * @brief Returns Qt's built-in default message pattern.
         *
         * Hard-coded in Qt's `qlogging.cpp` (Qt offers no getter for it). It is
         * what Qt renders around Mira's part until the application replaces it,
         * and it is *not* Mira's pattern: pass it to `qSetMessagePattern()` to
         * restore Qt's own default after changing it:
         * @code
         * qSetMessagePattern(Mira::Core::qtDefaultMessagePattern());
         * @endcode
         *
         * @return Qt's default pattern (`%{if-category}%{category}: %{endif}%{message}`).
         */
        MIRA_CORE_EXPORT const QString& qtDefaultMessagePattern();

        /**
         * @brief Sets Mira's message pattern.
         *
         * A line has two independent parts:
         * @code
         * {Qt pattern}{Mira pattern} message
         * @endcode
         *
         * `{Qt pattern}` is Qt's process-wide pattern, which Mira never touches:
         * set it with `qSetMessagePattern()` (or `QT_MESSAGE_PATTERN`) when you
         * want time, severity, category, file/line and so on. `{Mira pattern}` is
         * the pattern this function sets, and it is the only part Mira expands.
         *
         * Mira formats by handing its expansion to Qt as the message text
         * (`qFormatLogMessage(type, context, miraExpansion)`), so Qt's
         * `%{message}` renders `{Mira pattern}` followed by the message. Qt's
         * pattern is read, never written: Mira does not call
         * `qSetMessagePattern()` at all, so installing it cannot overwrite what
         * the application set, and no per-message work is spent on Qt's state.
         *
         * Mira fills these placeholders:
         *  - `%{objectName}`         the object's `name`
         *  - `%{objectInstanceId}`   the object's `instanceId`
         *  - `%{objectInstanceName}` the object's `instanceName`
         *  - `%{object}`             the composed `name:instanceId`, plus
         *                            `#instanceName` when the object has one
         *  - `%{message}`            the message text
         *  - `%{messageIndex}`       1-based index of the message, never repeated.
         *                            Values are allocated in formatting order;
         *                            concurrent threads may write them out of order
         *
         * When no `Mira::Core::Object` is emitting (plain `qDebug()`, or a
         * `QObject` that is not a Mira `Object`), the object placeholders expand to
         * `unknown`, so the object part of the pattern is clearly marked as absent.
         *
         * Like Qt's `%{if-category}...%{endif}`, each Mira placeholder also has a
         * conditional form that renders its block only when the placeholder is more
         * than the `unknown` fallback:
         *  - `%{if-object}...%{endif}`
         *  - `%{if-objectName}...%{endif}`
         *  - `%{if-objectInstanceId}...%{endif}`
         *  - `%{if-objectInstanceName}...%{endif}`
         *
         * For example `%{if-object}[%{object}] %{endif}%{message}` omits the object
         * part entirely for messages that have no object context. Mira's conditions
         * cannot enclose Qt's own conditions: Qt's tokens belong to Qt's pattern.
         *
         * A pattern that leaves `%{message}` out is a prefix, and the message is
         * appended after it - so `"[%{messageIndex}] "` prints the index followed
         * by the text, and an empty pattern prints the message alone.
         *
         * Qt's placeholders (`%{time}`, `%{type}`, `%{message}`, ...) are *not*
         * expanded here. A Qt token left in Mira's pattern is printed literally;
         * put it in Qt's pattern instead.
         *
         * This is the only function that changes Mira's pattern: it is remembered
         * as given, and @ref installMessageHandler / @ref uninstallMessageHandler
         * never touch it, so a pattern set before installing stays in effect - and
         * stays in effect after uninstalling.
         *
         * @param pattern The pattern.
         */
        MIRA_CORE_EXPORT void setMessagePattern(const QString& pattern);

        /**
         * @brief Returns Mira's message pattern.
         *
         * Complete in every situation: @ref defaultMessagePattern until the
         * application sets its own pattern with @ref setMessagePattern, the given
         * pattern after that - the same value before, during and after
         * install/uninstall, which never change it.
         *
         * @return Mira's pattern.
         */
        MIRA_CORE_EXPORT QString messagePattern();

        /**
         * @brief The extra sink registered with @ref setLogSink.
         *
         * @param type    The Qt message type.
         * @param context The Qt message log context (file/line/function/category).
         * @param message The formatted message, exactly as it was written to the
         *                console.
         */
        using LogSink = std::function<void(QtMsgType type,
                                           const QMessageLogContext& context,
                                           const QString& message)>;

        /**
         * @brief Registers an extra sink for the messages Mira's handler produces.
         *
         * Every message is still written to the console; the sink is an
         * *additional* consumer, e.g. to mirror the log into a file, a widget or
         * a network stream:
         * @code
         * Mira::Core::setLogSink([](QtMsgType, const QMessageLogContext&,
         *                           const QString& message) {
         *     myLogFile.write(message + QLatin1Char('\n'));
         * });
         * @endcode
         *
         * The sink is called from the logging thread after the console write,
         * while the object context of the message is still in effect. That
         * context must not be retained: the object may be destroyed immediately
         * after the call. It must not log through Mira itself (such messages
         * fall back to the raw re-entrancy path) and must not call back into
         * Mira's handler/pattern control functions. A sink must not throw: it
         * is called from Qt's C message callback, where an escaping exception
         * cannot be handled safely. Pass `nullptr` to remove the sink.
         *
         * The sink is only consulted while Mira's handler is installed
         * (@ref installMessageHandler) and is kept across install/uninstall cycles.
         *
         * @param sink The sink to call, or an empty `LogSink` to remove one.
         */
        MIRA_CORE_EXPORT void setLogSink(LogSink sink);

        /**
         * @brief Installs the Mira message handler.
         *
         * The handler writes every message as `qFormatLogMessage()` renders Qt's
         * pattern around Mira's expansion (see @ref setMessagePattern). It changes
         * neither Qt's process-wide pattern nor Mira's pattern: both belong to the
         * application, and repeated calls do nothing while the handler is
         * installed.
         */
        MIRA_CORE_EXPORT void installMessageHandler();

        /**
         * @brief Calls @ref uninstallMessageHandler then @ref installMessageHandler.
         */
        MIRA_CORE_EXPORT void reinstallMessageHandler();

        /**
         * @brief Uninstalls the message handler installed by @ref installMessageHandler.
         *
         * Restores the message handler that was active before
         * @ref installMessageHandler was called (Qt's default handler, or `nullptr`
         * if none was installed), so Qt behaves as if Mira had never been
         * installed. Safe to call when the Mira handler is not installed.
         *
         * Both patterns are deliberately left untouched - the application owns
         * them - so a later install picks up where it left off.
         */
        MIRA_CORE_EXPORT void uninstallMessageHandler();

        /**
         * @brief Returns the `QObject` currently emitting a log message.
         *
         * Valid while a message is being dispatched: inside a message handler, or
         * during the full expression of a `miraInfo()`-family statement.
         *
         * @return The emitting `QObject`, or `nullptr`.
         */
        MIRA_CORE_EXPORT const QObject* currentLogObject();

        /**
         * @brief Formats @p message the way Mira's handler would print it.
         *
         * Mira's pattern is expanded from @p object (see @ref setMessagePattern)
         * and the result is handed to Qt as the message text, so Qt's process-wide
         * pattern renders the line around it. One call is one printed message and
         * takes the next `%{messageIndex}`. The index reflects allocation order,
         * not necessarily the order in which concurrent threads write lines.
         *
         * @param type    The Qt message type.
         * @param context The Qt message log context (file/line/function/category).
         * @param message The raw message text.
         * @param object  The emitting `QObject`, or `nullptr`.
         * @return The fully formatted log line (without trailing newline).
         */
        MIRA_CORE_EXPORT QString formatMessage(QtMsgType type,
                                               const QMessageLogContext& context,
                                               const QString& message,
                                               const QObject* object);

        namespace Details {

            /**
             * @brief Reference to the object currently emitting a log message.
             *
             * A function-local `thread_local` inside an inline function has a single
             * instance across all translation units, so this stays one slot per thread
             * without exposing a global variable.
             *
             * @return A reference to the current thread's emitting-object slot.
             */
            inline const QObject*& currentObject() noexcept
            {
                thread_local const QObject* object = nullptr;
                return object;
            }

        } // namespace Details

        /**
         * @class LogObjectScope
         * @brief Publishes the emitting object for the current full expression (RAII).
         *
         * The `miraInfo()` family creates one of these for the current full
         * expression and hands it to `Details::logStream()`, which returns the
         * `QDebug` to log to. The scope is destroyed after that stream, so the
         * object is still published when the message is flushed to the handler, and
         * it is guaranteed to be restored afterwards - even when nothing was
         * streamed, and regardless of which handler is installed. Nested scopes
         * restore the previous value.
         *
         * The whole scope is inline: it compiles down to a couple of pointer
         * stores, so `miraInfo()` costs the same as a plain `qInfo()`.
         */
        class LogObjectScope
        {
        public:
            /**
             * @brief Publishes @p object until this scope is destroyed.
             * @param object The emitting object, or `nullptr`.
             */
            explicit LogObjectScope(const QObject* object) noexcept :
                previous(Details::currentObject())
            {
                Details::currentObject() = object;
            }

            /** @brief Restores the previously published object. */
            ~LogObjectScope() noexcept
            {
                Details::currentObject() = previous;
            }

        private:
            Q_DISABLE_COPY(LogObjectScope)

            const QObject* previous;    ///< Value restored on destruction.
        };

        namespace Details {

            /**
             * @brief Returns @p stream, keeping @p scope alive until it is flushed.
             *
             * The `miraInfo()` family expands to a call of this function, so that
             * the statement is a plain `QDebug` expression: a comma expression
             * would confuse the code models of IDEs, which take the type of their
             * first operand and would then offer the members of the scope instead
             * of the members of `QDebug`.
             *
             * The scope is passed in as an argument, so it is destroyed *after* the
             * returned stream: the message is flushed while the emitting object is
             * still published.
             *
             * @param scope  The scope that publishes the emitting object.
             * @param stream The stream to log to.
             * @return @p stream.
             */
            inline QDebug logStream(const LogObjectScope&, QDebug stream)
            {
                return stream;
            }

        } // namespace Details

    } // namespace Core
} // namespace Mira

/**
 * @brief The object a message is attributed to, outside a Mira object.
 *
 * `Mira::Core::Object` declares a member of the same name that returns `this`,
 * and unqualified name lookup finds that member first inside a member function
 * of a Mira object. So the `miraInfo()` family attributes a statement to the
 * object whose member function is running, and this fallback - what the same
 * macros resolve to anywhere else - attributes nothing: the Mira pattern
 * placeholders expand to `unknown`.
 *
 * It returns `nullptr` rather than remembering some earlier object, so a
 * message can never be attributed to an object that merely happens to be alive.
 *
 * @return Always `nullptr`; the member in `Mira::Core::Object` returns `this`.
 */
inline const QObject* miraLogObject() noexcept
{
    return nullptr;
}

/**
 * @def MIRA_LOG_STREAM(stream)
 * @brief (internal) Wraps a Qt logging statement in the object scope.
 *
 * All `miraInfo()`-family macros expand through this helper: it creates the
 * @ref Mira::Core::LogObjectScope for the current full expression and hands
 * @p stream to @ref Mira::Core::Details::logStream, which returns the `QDebug`
 * to stream into.
 *
 * `miraLogObject()` is deliberately unqualified: inside a member function of a
 * Mira object it resolves to `Mira::Core::Object::miraLogObject()`, so the
 * statement is attributed to that object, and anywhere else to the global
 * fallback.
 */
#define MIRA_LOG_STREAM(stream) (\
    ::Mira::Core::Details::logStream(\
        ::Mira::Core::LogObjectScope(miraLogObject()), \
        stream))

/**
 * @def miraInfo()
 * @brief Logs an info message; behaves exactly like `qInfo()`.
 *
 * Usable anywhere: in a member function, in a free function, in a capturing
 * lambda, ... The result is a `QDebug`, so the whole `QDebug`
 * API (`miraInfo().noquote() << ...`, `miraInfo().nospace() << ...`) is
 * available with `qInfo()`'s semantics.
 * @code
 * miraInfo().nospace() << "Some" << "info" << "message";
 * @endcode
 *
 * The message is attributed to the object whose member function is running:
 * `Mira::Core::Object` declares `miraLogObject()` returning `this`, which
 * shadows the global @ref miraLogObject returning `nullptr`. Outside a Mira
 * object the statement has no object context.
 *
 * A captureless lambda and a static member function have no `this`, so
 * `miraInfo()` does not resolve to the object member there: capture `this`, or
 * use plain `qInfo()` when no object should be attributed.
 */
#define miraInfo() \
    MIRA_LOG_STREAM(QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).info())

/** @def miraDebug() @brief Logs a debug message; behaves exactly like `qDebug()`. */
#define miraDebug() \
    MIRA_LOG_STREAM(QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).debug())

/** @def miraWarning() @brief Logs a warning message; behaves exactly like `qWarning()`. */
#define miraWarning() \
    MIRA_LOG_STREAM(QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).warning())

/** @def miraCritical() @brief Logs a critical message; behaves exactly like `qCritical()`. */
#define miraCritical() \
    MIRA_LOG_STREAM(QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).critical())

/**
 * @def miraFatal()
 * @brief Logs a fatal message; behaves exactly like `qFatal()`.
 *
 * Qt 5.14 has no streaming fatal (`QMessageLogger::fatal()` is printf-style), so
 * this uses the context-less `QDebug(QtFatalMsg)` form.
 *
 * @note Fatal messages terminate the application.
 */
#define miraFatal() MIRA_LOG_STREAM(QDebug(QtFatalMsg))

/**
 * @brief Shorter names for easier call
 */
#define mInfo()    miraInfo()
#define mDebug()   miraDebug()
#define mWarning() miraWarning()
#define mCritical() miraCritical()
#define mFatal()   miraFatal()
