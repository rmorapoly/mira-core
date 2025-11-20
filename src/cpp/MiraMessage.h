#pragma once

#include <QtGlobal>
#include <QDateTime>
#include "MiraCoreImport.h"

/**
 * @def STRINGIFY(x)
 * @brief Converts a macro argument to a string literal.
 *
 * This is the public interface for macro stringification.
 * Expands the argument before stringifying it.
 */
#define STRINGIFY_IMPL(x) #x
#define STRINGIFY(x) STRINGIFY_IMPL(x)

/**
 * @def __CLASS_NAME__
 * @brief Retrieves the demangled C++ class name of the current object.
 *
 * Uses RTTI (`typeid(*this).name()`) and `Mira::demangleClassName()`
 * to obtain a human-readable class name at runtime.
 *
 * This macro must be used **only inside a class scope**, where `this` is valid.
 */
#define __CLASS_NAME__ Mira::demangleClassName(typeid (*this).name())

namespace Mira {

    class Object;

    /**
     * @brief Converts a Qt message type (QtMsgType) to a human-readable string.
     *
     * Example:
     * @code
     * QString s = Mira::toStr(QtWarningMsg);   // "Warning"
     * @endcode
     *
     * @param msgType The Qt message type.
     * @return A descriptive string representation.
     */
    extern MIRA_CORE_EXPORT QString toStr(QtMsgType msgType);

    /**
     * @brief Converts a compiler-specific mangled name to a readable class name.
     *
     * Uses platform-dependent demangling routines (GCC/Clang `abi::__cxa_demangle`,
     * MSVC `UnDecorateSymbolName`) to obtain a human-friendly class name.
     *
     * Example:
     * @code
     * QString name = Mira::demangleClassName(typeid(MyClass).name());
     * @endcode
     *
     * @param mangledName The raw name returned by `typeid().name()`.
     * @return A demangled, user-friendly type name.
     */
    extern MIRA_CORE_EXPORT QString demangleClassName(const char* mangledName);

    /**
     * @struct Message
     * @brief Represents a routed log message inside the Mira module.
     *
     * This struct encapsulates:
     * - the message text,
     * - its classification (`QtMsgType`),
     * - an optional originating object (`Mira::Object`),
     * - and a timestamp.
     *
     * Messages are primarily created through `MessageHelper`, or constructed
     * manually when routing custom log events.
     *
     * ### Example
     * @code
     * Message msg("Something happened", QtWarningMsg, this);
     * qInfo().noquote() << msg;
     * @endcode
     */
    struct MIRA_CORE_EXPORT Message {
        QString text;
        QtMsgType type;
        Object* obj;
        QDateTime dateTimeStamp;

        /**
         * @brief Constructs a new message.
         *
         * @param text           The message content.
         * @param type           Message category (default: QtInfoMsg).
         * @param obj            Optional originating object pointer.
         * @param dateTimeStamp  Timestamp (default: current time).
         */
        Message(QString text = "",
                QtMsgType type = QtInfoMsg,
                Object* obj = nullptr,
                QDateTime dateTimeStamp = QDateTime::currentDateTime());

        /**
         * @brief Converts the message to a human-readable string.
         *
         * Format example:
         * "[2025-11-15 12:10:05.123] [Info] [MyObjectName#3] [MyInstanceName] Message text"
         *
         * @return A formatted string representing the message.
         */
        operator QString() const;

        /**
         * @brief Shorthand for `operator QString()`.
         */
        inline QString toStr() const {
            return operator QString();
        }
    };

} // namespace Mira
