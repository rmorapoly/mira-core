#pragma once

#include <QDebug>
#include <QString>
#include <QtCore/qlogging.h>
#include <functional>

#include "MiraCoreImport.h"

class QObject;

namespace Mira {
    namespace Core {

        /**
         * @brief Returns Mira's default message pattern.
         *
         * Renders time, severity, the message sequence number
         * (`%{messageIndex}`), the emitting object's context
         * (`%{if-object}[%{object}] %{endif}`) and the message itself.
         *
         * Intended for use with @ref setLogPattern, e.g. to return to the
         * default after a custom pattern:
         * @code
         * Mira::Core::setLogPattern(Mira::Core::defaultPattern());
         * @endcode
         *
         * @return The default pattern (never empty).
         */
        MIRA_CORE_EXPORT const QString& defaultPattern();

        /**
         * @brief Returns Qt's built-in default message pattern.
         *
         * Hard-coded in Qt's `qlogging.cpp` (Qt offers no getter for it). An
         * *empty* pattern is not equivalent: it makes `qFormatLogMessage()`
         * return an empty string, so nothing is printed at all.
         *
         * Mira never applies it on its own - install and uninstall leave the
         * pattern alone - so hand Qt's own formatting back explicitly when wanted:
         * @code
         * Mira::Core::setLogPattern(Mira::Core::qtDefaultPattern());
         * @endcode
         *
         * @return Qt's default pattern (`%{if-category}%{category}: %{endif}%{message}`).
         */
        MIRA_CORE_EXPORT const QString& qtDefaultPattern();

        /**
         * @brief Sets the message pattern used by the Mira log handler.
         *
         * Every placeholder is resolved by Qt through `qFormatLogMessage()`
         * (`%{time}`, `%{type}`, `%{file}`, `%{line}`, `%{function}`, `%{category}`,
         * `%{pid}`, `%{appname}`, `%{message}`, ...), so their semantics are exactly
         * Qt's. On top of those, Mira fills in the following placeholders from the
         * `Mira::Core::Object` that emitted the message:
         *  - `%{objectName}`         the object's `name`
         *  - `%{objectInstanceId}`   the object's `instanceId`
         *  - `%{objectInstanceName}` the object's `instanceName`
         *  - `%{object}`             the composed `name:instanceId`, plus
         *                            `#instanceName` when the object has one.
         *
         * In addition, Mira maintains a message sequence number, a row count
         * over every message it formats, and exposes it as:
         *  - `%{messageIndex}`       1-based index of the message (optional;
         *                            Qt has no token of its own here, so Mira
         *                            consumes it before Qt renders the pattern).
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
         * and Qt's own conditions may be mixed in one pattern.
         *
         * This is the only function that changes the pattern: it is remembered as
         * given, and @ref installLogHandler / @ref uninstallLogHandler never touch
         * it, so a pattern set before installing stays in effect - and stays in
         * effect after uninstalling. The application owns the pattern; use
         * @ref defaultPattern or @ref qtDefaultPattern to restore a known one.
         *
         * Because `qFormatLogMessage()` always reads Qt's process-wide pattern, Mira
         * keeps that pattern in sync with the emitting object. It is written once per
         * call here, and per message only when the emitting object changes - never for
         * every message, and never for messages that have no object. The only
         * exception is a pattern that uses `%{messageIndex}`: the index changes per
         * message, so such a pattern is re-rendered for every message (that is the
         * price of the token; patterns without it keep the cached, allocation-free
         * path). @ref installLogHandler and @ref uninstallLogHandler never change
         * the pattern at all: Qt's process-wide pattern keeps the last
         * placeholder-free form Mira rendered, so foreign handlers (QtTest, a
         * user-installed handler, Qt's default handler) always format a pattern
         * that contains no Mira placeholders.
         *
         * @param pattern The message pattern.
         */
        MIRA_CORE_EXPORT void setLogPattern(const QString& pattern);

        /**
         * @brief Returns the message pattern currently used by the Mira log handler.
         *
         * Complete in every situation: @ref defaultPattern until the application
         * sets its own pattern with @ref setLogPattern, the given pattern after
         * that - the same value before, during and after install/uninstall,
         * which never change it.
         *
         * @return The active pattern.
         */
        MIRA_CORE_EXPORT QString logPattern();

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
         * while the object context of the message is still in effect. It must
         * not log through Mira itself (such messages fall back to the raw
         * re-entrancy path) and must not call back into Mira's handler/pattern
         * control functions. Pass `nullptr` to remove the sink.
         *
         * The sink is only consulted while Mira's handler is installed
         * (@ref installLogHandler) and is kept across install/uninstall cycles.
         *
         * @param sink The sink to call, or an empty `LogSink` to remove one.
         */
        MIRA_CORE_EXPORT void setLogSink(LogSink sink);

        /**
         * @brief Installs the Mira message handler.
         *
         * The pattern is never touched: whatever @ref setLogPattern last set -
         * @ref defaultPattern until the application customizes it - stays in
         * effect, the application staying responsible for it. Repeated calls do
         * nothing while the handler is installed.
         *
         * The pattern is applied in two ways depending on the emitter:
         *  - From a `Mira::Core::Object`, the pattern is applied in full, so the
         *    `%{object}` (and `%{objectName}`, `%{objectInstanceId}`,
         *    `%{objectInstanceName}`) tokens are filled in with that object's
         *    context.
         *  - From any other `QObject` (or no object at all), the Mira object tokens
         *    expand to `unknown`.
         *
         * All standard Qt placeholders (`%{time}`, `%{type}`, `%{file}`, ...) apply
         * in both cases.
         *
         * Neither this function nor @ref uninstallLogHandler ever changes the
         * message pattern; @ref setLogPattern is the only way to do that.
         */
        MIRA_CORE_EXPORT void installLogHandler();

        /**
         * @brief call uninstallLogHandler then installLogHandler
         */
        MIRA_CORE_EXPORT void reinstallLogHandler();

        /**
         * @brief Uninstalls the message handler installed by @ref installLogHandler.
         *
         * Restores the message handler that was active before @ref installLogHandler
         * was called (Qt's default handler, or `nullptr` if none was installed), so
         * Qt behaves as if Mira had never been installed. Safe to call when the Mira
         * handler is not installed.
         *
         * The message pattern is deliberately left untouched - the application owns
         * it. Set @ref qtDefaultPattern with @ref setLogPattern to hand Qt's own
         * formatting back explicitly. The pattern set with @ref setLogPattern stays
         * in effect for the next message after a later install.
         */
        MIRA_CORE_EXPORT void uninstallLogHandler();

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
         * @brief Formats @p message using the active pattern.
         *
         * The Mira placeholders are expanded from @p object, then Qt's
         * `qFormatLogMessage()` expands every remaining placeholder, so Qt tokens
         * keep Qt's own semantics. Qt's process-wide pattern is updated only when
         * the emitting object changes (see @ref setLogPattern). When the pattern
         * uses `%{messageIndex}`, this call also advances the sequence number.
         *
         * @param type    The Qt message type.
         * @param context The Qt message log context (file/line/function/category).
         * @param message The raw message text.
         * @param object  The emitting `QObject`, or `nullptr`.
         * @return The fully formatted log line (without trailing newline).
         */
        MIRA_CORE_EXPORT QString formatLogMessage(QtMsgType type,
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
 */
#define MIRA_LOG_STREAM(stream) (\
    ::Mira::Core::Details::logStream(\
        ::Mira::Core::LogObjectScope(miraLogObject()), \
        stream))

/**
 * @def miraInfo()
 * @brief Logs an info message; behaves exactly like `qInfo()`.
 *
 * Usable anywhere: in a member function, in a free function, in a lambda, in a
 * static member function, ... The result is a `QDebug`, so the whole `QDebug`
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
 */
#define miraInfo() \
    MIRA_LOG_STREAM(QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).info())

/** @def miraDebug() @brief Logs a debug message; behaves exactly like `qDebug()`. */
#define miraDebug() \
    MIRA_LOG_STREAM(QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).debug())

/** @def miraWarning() @brief Logs a warning; behaves exactly like `qWarning()`. */
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
