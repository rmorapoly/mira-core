#pragma once

#include <QtGlobal>
#include <QString>
#include <type_traits>
#include <typeinfo>
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
     * MSVC human-readable names) to obtain a human-friendly class name.
     *
     * @param mangledName The raw name returned by `typeid().name()`.
     * @return A demangled, user-friendly type name.
     */
    extern MIRA_CORE_EXPORT QString demangleClassName(const char* mangledName);

    namespace Details {

        /**
         * @brief Compile-time (RTTI-free) extraction of a type name.
         *
         * Parses the compiler-provided `__PRETTY_FUNCTION__` (GCC/Clang) or
         * `__FUNCSIG__` (MSVC) to obtain the name of @p T without relying on
         * RTTI or the platform demangler.
         *
         * Function-local scopes are stripped, e.g.
         * `Ns::fn()::Local` becomes `Local`, while enclosing namespaces and
         * nested class names are preserved (`Ns::Outer::Inner`).
         *
         * @tparam T The type whose name should be extracted.
         * @return The extracted type name.
         */
        template <typename T>
        inline QString rawClassName()
        {
#if defined(__clang__) || defined(__GNUC__)
            const QString sig = QString::fromUtf8(__PRETTY_FUNCTION__);
            const int begin = sig.indexOf(QStringLiteral("T = "));
            const int end = sig.lastIndexOf(QLatin1Char(']'));
            if (begin < 0 || end < 0)
                return sig;
            QString name = sig.mid(begin + 4, end - begin - 4);
#elif defined(_MSC_VER)
            QString name = QString::fromUtf8(__FUNCSIG__);
            const int begin = name.indexOf(QLatin1Char('<'));
            const int end = name.lastIndexOf(QLatin1Char('>'));
            if (begin < 0 || end < 0)
                return name;
            name = name.mid(begin + 1, end - begin - 1);
            name.remove(QStringLiteral("class "));
            name.remove(QStringLiteral("struct "));
            name.remove(QStringLiteral("enum "));
#else
            return demangleClassName(typeid(T).name());
#endif
            // Strip function-local scope (e.g. "Ns::fn()::Local" -> "Local")
            const int local = name.lastIndexOf(QStringLiteral(")::"));
            if (local != -1)
                name = name.mid(local + 3);
            return name;
        }

    } // namespace Details

    /**
     * @brief Returns the readable name of a type at compile time.
     *
     * Unlike `demangleClassName()` this does not depend on RTTI and works for
     * any complete type, including function-local and nested classes.
     *
     * Example:
     * @code
     * struct Child : Mira::Core::Object {
     *     Child() MIRA_DECLARE_OBJECT {}
     * };
     * // Mira::className<Child>() == "Child"
     * @endcode
     *
     * @tparam T The type whose name is requested.
     * @return The readable type name.
     */
    template <typename T>
    inline QString className()
    {
        return Details::rawClassName<T>();
    }

} // namespace Mira
