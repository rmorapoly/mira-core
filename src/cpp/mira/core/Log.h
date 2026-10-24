#pragma once

#include <QDebug>
#include <QPointer>
#include <QString>
#include <QVector>
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

            /**
             * @brief The Mira objects that are in scope on this thread.
             *
             * The innermost (last) object is the one that the `miraInfo()` family
             * attributes its messages to. The objects are held weakly, so an object
             * that dies is dropped from the scope even if it was destroyed on
             * another thread.
             *
             * @note As for every `QObject`, an object is expected to be destroyed
             * on the thread that created it.
             *
             * @return A reference to the current thread's object-scope stack.
             */
            inline QVector<QPointer<QObject>>& miraObjectScopes() noexcept
            {
                thread_local QVector<QPointer<QObject>> scopes;
                return scopes;
            }

            /**
             * @brief Puts @p object in scope.
             * @param object The object to put in scope, or `nullptr`.
             */
            inline void pushMiraObject(const QObject* object) noexcept
            {
                // QPointer only tracks non-const pointers; the object itself is
                // never modified through it.
                miraObjectScopes().append(const_cast<QObject*>(object));
            }

            /**
             * @brief Takes @p object out of scope again.
             * @param object The object to take out of scope.
             */
            inline void popMiraObject(const QObject* object) noexcept
            {
                QVector<QPointer<QObject>>& scopes = miraObjectScopes();

                // The object may have been taken out of scope already, and the
                // scopes are not necessarily closed in reverse order, so the
                // matching entry is looked for.
                for (int i = scopes.size() - 1; i >= 0; --i) {
                    if (scopes.at(i).data() == object) {
                        scopes.remove(i);
                        return;
                    }
                }
            }

            /**
             * @brief The object a `miraInfo()`-family statement is attributed to.
             *
             * @return The innermost Mira object that is in scope, or `nullptr`.
             */
            inline const QObject* resolveLogObject() noexcept
            {
                const QVector<QPointer<QObject>>& scopes = miraObjectScopes();

                // Objects that were destroyed in the meantime are skipped.
                for (int i = scopes.size() - 1; i >= 0; --i) {
                    if (const QObject* object = scopes.at(i).data())
                        return object;
                }
                return nullptr;
            }

            /**
             * @brief The object a `miraInfo()`-family statement is attributed to.
             *
             * The explicit form overrides the object that is in scope, so a message
             * can be attributed to an object other than the one in scope.
             *
             * @param object The object to attribute the message to, or `nullptr`.
             * @return @p object.
             */
            inline const QObject* resolveLogObject(const QObject* object) noexcept
            {
                return object;
            }

        } // namespace Details

        /**
         * @class LogObjectScope
         * @brief Publishes the emitting object for the current full expression (RAII).
         *
         * The `miraInfo()` family creates one of these for the current full
         * expression and hands it to `Details::miraLogStream()`, which returns the
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

        /**
         * @class ObjectScope
         * @brief Makes @p object the Mira object in scope (RAII).
         *
         * While an object is in scope, every `miraInfo()`-family statement is
         * attributed to it, no matter where the statement is written: inside a
         * member function, in a free function, in a lambda, in a static member
         * function, ...
         *
         * A `Mira::Core::Object` opens such a scope for its own lifetime, so
         * logging from inside one of its member functions needs no help. This class
         * is for the cases where the object in scope should be a specific one
         * rather than the innermost living object.
         *
         * @code
         * Mira::Core::ObjectScope scope(myObject);
         * mInfo() << "attributed to myObject";
         * @endcode
         */
        class ObjectScope
        {
        public:
            /**
             * @brief Puts @p object in scope until this scope is destroyed.
             * @param object The object to attribute messages to, or `nullptr`.
             */
            explicit ObjectScope(const QObject* object) noexcept :
                object(object)
            {
                Details::pushMiraObject(object);
            }

            /** @brief Takes the object out of scope again. */
            ~ObjectScope() noexcept
            {
                Details::popMiraObject(object);
            }

        private:
            Q_DISABLE_COPY(ObjectScope)

            const QObject* object;    ///< Object taken out of scope on destruction.
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
            inline QDebug miraLogStream(const LogObjectScope&, QDebug stream)
            {
                return stream;
            }

        } // namespace Details

    } // namespace Core
} // namespace Mira

/**
 * @brief The Mira object that is currently in scope.
 *
 * A `Mira::Core::Object` is in scope for as long as it lives, so this is the
 * object that the `miraInfo()` family attributes its messages to. Outside any
 * Mira object scope it is `nullptr`, and the Mira pattern placeholders expand
 * to `unknown`.
 *
 * @return The Mira object in scope, or `nullptr`.
 */
inline const QObject* miraLogObject() noexcept
{
    return ::Mira::Core::Details::resolveLogObject();
}

/**
 * @def miraInfo()
 * @brief Logs an info message; behaves exactly like `qInfo()`.
 *
 * Usable anywhere: in a member function, in a free function, in a lambda, in a
 * static member function, ... The result is a `QDebug`, so the whole `QDebug`
 * API (`miraInfo().noquote() << ...`, `miraInfo().nospace() << ...`, ...) is
 * available with `qInfo()`'s semantics.
 * @code
 * miraInfo().nospace() << "Some" << "info" << "message";
 * @endcode
 *
 * The message is attributed to the Mira object that is in scope (see
 * @ref Mira::Core::ObjectScope). Use @ref miraInfoObject() to attribute it to
 * a specific object instead.
 */
#define miraInfo() (\
    ::Mira::Core::Details::miraLogStream(\
        ::Mira::Core::LogObjectScope(::Mira::Core::Details::resolveLogObject()), \
        QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).info()))

/**
 * @def miraInfoObject(object)
 * @brief Logs an info message attributed to @p object.
 *
 * Behaves exactly like @ref miraInfo(), but the message is attributed to
 * @p object instead of the object that is in scope.
 * @code
 * miraInfoObject(myObject) << "attributed to myObject";
 * @endcode
 */
#define miraInfoObject(object) (\
    ::Mira::Core::Details::miraLogStream(\
        ::Mira::Core::LogObjectScope(::Mira::Core::Details::resolveLogObject(object)), \
        QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).info()))

/** @def miraDebug() @brief Logs a debug message; behaves exactly like `qDebug()`. */
#define miraDebug() (\
    ::Mira::Core::Details::miraLogStream(\
        ::Mira::Core::LogObjectScope(::Mira::Core::Details::resolveLogObject()), \
        QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).debug()))

/**
 * @def miraDebugObject(object)
 * @brief Logs a debug message attributed to @p object.
 *
 * Behaves exactly like @ref miraDebug(), but the message is attributed to
 * @p object instead of the object that is in scope.
 */
#define miraDebugObject(object) (\
    ::Mira::Core::Details::miraLogStream(\
        ::Mira::Core::LogObjectScope(::Mira::Core::Details::resolveLogObject(object)), \
        QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).debug()))

/** @def miraWarning() @brief Logs a warning; behaves exactly like `qWarning()`. */
#define miraWarning() (\
    ::Mira::Core::Details::miraLogStream(\
        ::Mira::Core::LogObjectScope(::Mira::Core::Details::resolveLogObject()), \
        QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).warning()))

/**
 * @def miraWarningObject(object)
 * @brief Logs a warning attributed to @p object.
 *
 * Behaves exactly like @ref miraWarning(), but the message is attributed to
 * @p object instead of the object that is in scope.
 */
#define miraWarningObject(object) (\
    ::Mira::Core::Details::miraLogStream(\
        ::Mira::Core::LogObjectScope(::Mira::Core::Details::resolveLogObject(object)), \
        QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).warning()))

/** @def miraCritical() @brief Logs a critical message; behaves exactly like `qCritical()`. */
#define miraCritical() (\
    ::Mira::Core::Details::miraLogStream(\
        ::Mira::Core::LogObjectScope(::Mira::Core::Details::resolveLogObject()), \
        QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).critical()))

/**
 * @def miraCriticalObject(object)
 * @brief Logs a critical message attributed to @p object.
 *
 * Behaves exactly like @ref miraCritical(), but the message is attributed to
 * @p object instead of the object that is in scope.
 */
#define miraCriticalObject(object) (\
    ::Mira::Core::Details::miraLogStream(\
        ::Mira::Core::LogObjectScope(::Mira::Core::Details::resolveLogObject(object)), \
        QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).critical()))

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
    ::Mira::Core::Details::miraLogStream(\
        ::Mira::Core::LogObjectScope(::Mira::Core::Details::resolveLogObject()), \
        QDebug(QtFatalMsg)))

/**
 * @def miraFatalObject(object)
 * @brief Logs a fatal message attributed to @p object.
 *
 * Behaves exactly like @ref miraFatal(), but the message is attributed to
 * @p object instead of the object that is in scope.
 *
 * @note Fatal messages terminate the application.
 */
#define miraFatalObject(object) (\
    ::Mira::Core::Details::miraLogStream(\
        ::Mira::Core::LogObjectScope(::Mira::Core::Details::resolveLogObject(object)), \
        QDebug(QtFatalMsg)))

/**
 * @brief Shorter names for easier call
 */
#define mInfo()          miraInfo()
#define mInfoObject(o)   miraInfoObject(o)
#define mDebug()         miraDebug()
#define mDebugObject(o)  miraDebugObject(o)
#define mWarning()       miraWarning()
#define mWarningObject(o) miraWarningObject(o)
#define mCritical()      miraCritical()
#define mCriticalObject(o) miraCriticalObject(o)
#define mFatal()         miraFatal()
#define mFatalObject(o)  miraFatalObject(o)

