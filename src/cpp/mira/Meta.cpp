#include "Meta.h"

#ifdef __GNUG__
#include <cxxabi.h>
#include <typeinfo>
#endif

namespace Mira {

QString toStr(QtMsgType msgType)
{
    switch (msgType) {
        case QtDebugMsg:    return QStringLiteral("Debug");
        case QtInfoMsg:     return QStringLiteral("Info");
        case QtWarningMsg:  return QStringLiteral("Warning");
        case QtCriticalMsg: return QStringLiteral("Critical");
        case QtFatalMsg:    return QStringLiteral("Fatal");
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        case QtSystemMsg:   return QStringLiteral("System");
#endif
        default:            return QStringLiteral("Unknown");
    }
}

QString demangleClassName(const char* mangledName)
{
#ifdef __GNUG__
    int status = -1;
    char* demangled = abi::__cxa_demangle(mangledName, nullptr, nullptr, &status);
    QString result = (status == 0 && demangled) ? QString::fromUtf8(demangled) : QString::fromUtf8(mangledName);
    free(demangled);

    // Handling pointers
    result.remove('*');

    // Handling local definitions
    if (result.contains(")::")) {
        auto inx = result.lastIndexOf(")::");
        return result.mid(inx + 3);
    }

    return result;
#else
    // MSVC already returns human readable name
    auto str = QString::fromUtf8(mangledName);

    // Handling pointers
    auto starInx = str.lastIndexOf(" * ");
    if (starInx != -1) {
        str = str.mid(0, starInx);
    }

    // Handling local definitions
    auto inx = str.lastIndexOf("'::");
    if (inx == -1)
        return str.split(' ').last();
    else {
        return str.mid(inx + 3);
    }
#endif
}

} // namespace Mira
