#pragma once

#include <QDebug>
#include <QString>
#include "MiraCoreImport.h"

class QObject;

namespace Mira {
    namespace Core {

        /**
         * @brief Sets the message pattern used by the Mira log handler.
         *
         * On top of every placeholder understood by Qt's `qFormatLogMessage()`
         * (`%{time}`, `%{type}`, `%{file}`, `%{line}`, `%{function}`, `%{message}`,
         * `%{category}`, ...), the following Mira placeholders are supported. They
         * are filled from the `Mira::Core::Object` that emitted the message:
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
         * was called (Qt's default handler, or `nullptr` if none was installed).
         * Safe to call when the Mira handler is not installed.
         */
        MIRA_CORE_EXPORT void uninstallLogHandler();

        /**
         * @brief Returns the `QObject` currently emitting a log message.
         *
         * Valid only while a message is being dispatched (i.e. inside a message
         * handler or during a `miraInfo()`-family expression). Used by @ref
         * formatLogMessage to resolve the Mira placeholders.
         *
         * @return The emitting `QObject`, or `nullptr`.
         */
        MIRA_CORE_EXPORT const QObject* currentLogObject();

        /**
         * @brief Formats @p message using the active pattern.
         *
         * Standard placeholders are expanded by Qt's `qFormatLogMessage()`, while
         * the Mira placeholders are filled from @p object (see @ref setLogPattern).
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

        /**
         * @class LogStream
         * @brief Lightweight `QDebug`-based stream used by the `miraInfo()` family.
         *
         * `LogStream` does not reimplement logging or formatting. It collects the
         * streamed values with `QDebug` and, on destruction, dispatches the raw
         * message through `qDebug()/qInfo()/qWarning()/qCritical()/qFatal()`. While
         * dispatching, the emitting `QObject` is made available to the message
         * handler, which expands the Mira pattern placeholders (see
         * @ref setLogPattern):
         *
         * @code
         * class MyObject : public Mira::Core::Object {
         * public:
         *     MyObject() MIRA_DECLARE_OBJECT {}
         *     void work() { miraInfo() << "doing work"; }
         * };
         * // -> "[2019-...] [info] ... doWork [MyObject#0] doing work"
         * @endcode
         */
        class MIRA_CORE_EXPORT LogStream
        {
        public:
            /**
             * @brief Constructs a stream bound to @p obj and @p type.
             * @param type The Qt message type used for dispatch.
             * @param obj The originating object, or `nullptr`.
             */
            LogStream(QtMsgType type, const QObject* obj);

            /**
             * @brief Flushes the buffer and dispatches it through the Qt logging pipeline.
             */
            ~LogStream();

            /**
             * @brief Streams a value using `QDebug` formatting.
             *
             * @tparam T Type of the value being appended.
             * @param value The value to append.
             * @return Reference to this LogStream.
             */
            template <typename T>
            LogStream& operator<<(const T& value)
            {
                if (dbg) {
                    (*dbg) << value;
                    hasContent = true;
                }
                return *this;
            }

        private:
            Q_DISABLE_COPY(LogStream)

            /**
             * @brief Emits the assembled message through the matching Qt logging function.
             * @param message The raw message text.
             */
            void dispatch(const QString& message) const;

            QString buffer;                 ///< Accumulates the `QDebug` formatted message.
            QDebug* dbg = nullptr;          ///< `QDebug` writing into @ref buffer.
            QtMsgType type = QtInfoMsg;     ///< Severity used at dispatch time.
            bool hasContent = false;        ///< Whether anything was streamed into the message.
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
 * returns `nullptr`, so the Mira pattern tokens expand to nothing.
 *
 * @return Always `nullptr`.
 */
inline const QObject* miraLogObject() noexcept { return nullptr; }

/**
 * @def miraInfo()
 * @brief Creates a `LogStream` for an Info message.
 *
 * Usable anywhere: inside a `Mira::Core::Object` member function the message is
 * attributed to that object; elsewhere it has no object context.
 * @code
 * miraInfo() << "Some info message";
 * @endcode
 */
#define miraInfo()     ::Mira::Core::LogStream(QtInfoMsg, miraLogObject())

/** @def miraDebug() @brief Creates a Debug `LogStream`. */
#define miraDebug()    ::Mira::Core::LogStream(QtDebugMsg, miraLogObject())

/** @def miraWarning() @brief Creates a Warning `LogStream`. */
#define miraWarning()  ::Mira::Core::LogStream(QtWarningMsg, miraLogObject())

/** @def miraCritical() @brief Creates a Critical `LogStream`. */
#define miraCritical() ::Mira::Core::LogStream(QtCriticalMsg, miraLogObject())

/**
 * @def miraFatal()
 * @brief Creates a Fatal `LogStream`.
 *
 * @note Fatal messages terminate the application (via `qFatal()`).
 */
#define miraFatal()    ::Mira::Core::LogStream(QtFatalMsg, miraLogObject())

/**
 * @brief Shorter names for easier call
 */
#define mInfo()     miraInfo()
#define mDebug()    miraDebug()
#define mWarning()  miraWarning()
#define mCritical() miraCritical()
#define mFatal()    miraFatal()

