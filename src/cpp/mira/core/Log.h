#pragma once

#include <QDebug>
#include <QString>
#include <QtCore/qlogging.h>
#include "MiraCoreImport.h"

class QObject;

namespace Mira {
    namespace Core {

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
         * When no `Mira::Core::Object` is emitting (plain `qDebug()`, or a
         * `QObject` that is not a Mira `Object`), the Mira placeholders expand to
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
         * Because `qFormatLogMessage()` always reads Qt's process-wide pattern, Mira
         * keeps that pattern in sync with the emitting object. It is written once per
         * call here, and per message only when the emitting object changes - never for
         * every message, and never for messages that have no object. The same applies
         * to @ref installLogHandler and @ref uninstallLogHandler, which leave the
         * placeholder-free (`unknown`) form in place so foreign handlers (QtTest, a
         * user-installed handler, Qt's default handler) always format a pattern that
         * contains no Mira placeholders.
         *
         * @param pattern The message pattern.
         */
        MIRA_CORE_EXPORT void setLogPattern(const QString& pattern);

        /**
         * @brief Returns the message pattern currently used by the Mira log handler.
         * @return The active pattern.
         */
        MIRA_CORE_EXPORT QString logPattern();

        /**
         * @brief Installs the Mira message handler and selects the default pattern.
         *
         * The default pattern renders time, severity, then the emitting object's
         * context (`%{object}`, `%{objectName}:%{objectInstanceId}` and
         * `%{objectInstanceName}`) and finally the message itself.
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
         * @ref uninstallLogHandler reverses this by restoring Qt's own default
         * message pattern.
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
         * was called (Qt's default handler, or `nullptr` if none was installed) and
         * gives Qt its own default message pattern back
         * (`%{if-category}%{category}: %{endif}%{message}`), so Qt behaves as if Mira
         * had never been installed. Safe to call when the Mira handler is not installed.
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
         * the emitting object changes (see @ref setLogPattern).
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
         * The `miraInfo()` family creates one of these *before* the `QDebug` stream.
         * Temporaries are destroyed in reverse order of construction, so the object is
         * still published when the stream flushes the message to the handler, and it is
         * guaranteed to be restored afterwards - even when nothing was streamed, and
         * regardless of which handler is installed. Nested scopes restore the previous
         * value.
         *
         * Both members are inline: the whole scope compiles down to a couple of pointer
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

    } // namespace Core
} // namespace Mira

/**
 * @brief Fallback attribution used by the `miraInfo()` family of macros.
 *
 * The macros expand to `miraLogObject()`. Inside a `Mira::Core::Object` member
 * function (including derived classes) the homonymous member defined in
 * `Object` hides this free function and returns the object itself; anywhere
 * else (`main()`, free functions, plain `QObject`s) this fallback is found and
 * returns `nullptr`, so the Mira pattern placeholders expand to `unknown`.
 *
 * @return Always `nullptr`.
 */
inline const QObject* miraLogObject() noexcept { return nullptr; }

/**
 * @def miraInfo()
 * @brief Logs an info message; behaves exactly like `qInfo()`.
 *
 * Usable anywhere: inside a `Mira::Core::Object` member function the message is
 * attributed to that object; elsewhere it has no object context. The result is a
 * `QDebug`, so the whole `QDebug` API (`miraInfo().noquote() << ...`,
 * `miraInfo().nospace() << ...`, ...) is available with `qInfo()`'s semantics.
 * @code
 * miraInfo().nospace() << "Some" << "info" << "message";
 * @endcode
 */
#define miraInfo() (\
    ::Mira::Core::LogObjectScope(miraLogObject()), \
    QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).info())

/** @def miraDebug() @brief Logs a debug message; behaves exactly like `qDebug()`. */
#define miraDebug() (\
    ::Mira::Core::LogObjectScope(miraLogObject()), \
    QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).debug())

/** @def miraWarning() @brief Logs a warning; behaves exactly like `qWarning()`. */
#define miraWarning() (\
    ::Mira::Core::LogObjectScope(miraLogObject()), \
    QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).warning())

/** @def miraCritical() @brief Logs a critical message; behaves exactly like `qCritical()`. */
#define miraCritical() (\
    ::Mira::Core::LogObjectScope(miraLogObject()), \
    QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).critical())

/**
 * @def miraFatal()
 * @brief Logs a fatal message; behaves exactly like `qFatal()`.
 *
 * Qt 5.14 has no streaming fatal (`QMessageLogger::fatal()` is printf-style), so
 * this uses the context-less `QDebug(QtFatalMsg)` form.
 *
 * @note Fatal messages terminate the application.
 */
#define miraFatal() (\
    ::Mira::Core::LogObjectScope(miraLogObject()), \
    QDebug(QtFatalMsg))

/**
 * @brief Shorter names for easier call
 */
#define mInfo()     miraInfo()
#define mDebug()    miraDebug()
#define mWarning()  miraWarning()
#define mCritical() miraCritical()
#define mFatal()    miraFatal()

