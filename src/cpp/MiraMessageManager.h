#pragma once

#include <QObject>
#include "MiraMessage.h"
#include <QTextStream>

/**
 * @def miraInfo()
 * @brief Creates a `MessageHelper` for an Info message.
 *
 * Usage:
 * @code
 * miraInfo() << "Some info message";
 * @endcode
 */
#define miraInfo()      makeMessageHelper(QtInfoMsg)

/**
 * @def miraDebug()
 * @brief Creates a `MessageHelper` for a Debug message.
 */
#define miraDebug()     makeMessageHelper(QtDebugMsg)

/**
 * @def miraWarning()
 * @brief Creates a `MessageHelper` for a Warning message.
 */
#define miraWarning()   makeMessageHelper(QtWarningMsg)

/**
 * @def miraCritical()
 * @brief Creates a `MessageHelper` for a Critical message.
 */
#define miraCritical()  makeMessageHelper(QtCriticalMsg)

/**
 * @def miraFatal()
 * @brief Creates a `MessageHelper` for a Fatal message.
 *
 * @note Fatal messages will typically terminate the application.
 */
#define miraFatal()     makeMessageHelper(QtFatalMsg)

namespace Mira {

    class Object;

    /**
     * @class MessageManager
     * @brief Central manager responsible for routing and dispatching Mira messages.
     *
     * `MessageManager` provides the core functionality for logging and reporting messages.
     * It emits the `reported()` signal whenever a message is processed.
     *
     * ### Example:
     * @code
     * const auto& manager = Mira::Core::getInstance()->getMessageManager();
     * connect(&manager, &MessageManager::reported, [](const Mira::Message& msg){
     *     qDebug() << msg.toStr();
     * });
     * miraInfo() << "Hello, world!";
     * @endcode
     *
     * But typically there is no need instantiation, just use mira macros so the messages are reported
     */
    class MIRA_CORE_EXPORT MessageManager : public QObject
    {
        Q_OBJECT

    public:
        /**
         * @brief Constructs a new MessageManager.
         * @param parent Optional QObject parent.
         */
        explicit MessageManager(QObject *parent = nullptr);

        /**
         * @brief Reports a message to all registered sinks (sinks will be added in the new version).
         *
         * This method emits the `reported()` signal and can be extended to
         * print to console, log to a file, or send over network (should be set via sinks).
         *
         * @param msg The message to report.
         */
        void report(const Message& msg) const;

    signals:
        /**
         * @brief Signal emitted whenever a message is reported.
         *
         * Users can connect to this signal to implement custom message handling.
         *
         * @param msg The reported message.
         */
        void reported(const Mira::Message& msg) const;
    };

    /**
     * @class MessageHelper
     * @brief Helper class to construct a `Mira::Message` using the stream operator.
     *
     * Example usage:
     * @code
     * Mira::MessageHelper helper(nullptr, QtInfoMsg);
     * helper << "Message part 1" << 42 << "part 3";
     * @endcode
     */
    class MIRA_CORE_EXPORT MessageHelper
    {
    public:
        /**
         * @brief Constructs a MessageHelper.
         * @param obj Optional originating object.
         * @param type The QtMsgType for the message.
         */
        MessageHelper(const Object* obj, QtMsgType type);

        /**
         * @brief Destructor.
         *
         * Typically sends the constructed message to `MessageManager` when
         * the object goes out of scope.
         */
        ~MessageHelper();

        /**
         * @brief Stream operator for building message text.
         *
         * Appends the value to the internal buffer followed by a space.
         *
         * @tparam T Type of the value being appended.
         * @param value The value to append.
         * @return Reference to this MessageHelper.
         */
        template<typename T>
        MessageHelper& operator<<(const T& value) {
            QTextStream ts(&buffer);
            ts << value << " ";
            return *this;
        }

    private:
        /**
         * @var obj Optional originating object.
         * @var type The QtMsgType for the message.
         * @var buffer Includes the appendes msgs for reporting
         */
        const Object* obj; ///< Optional originating object.
        QtMsgType type;    ///< Message type.
        QString buffer;    ///< Internal buffer storing the message text.
    };

} // namespace Mira

/**
 * @brief Creates a `MessageHelper` for the given message type.
 *
 * @param type The type of message (Info, Debug, Warning, etc.).
 * @return A temporary `MessageHelper` object for streaming message content.
 */
inline Mira::MessageHelper makeMessageHelper(QtMsgType type)
{
    return Mira::MessageHelper(nullptr, type);
}
