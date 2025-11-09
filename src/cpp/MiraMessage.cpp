#include "MiraMessage.h"
#include "MiraObject.h"
#include <cxxabi.h>
#include <typeinfo>

using namespace Mira;

QString Mira::toStr(QtMsgType msgType) {
    switch (msgType) {
        case QtDebugMsg:    return "Debug";
        case QtInfoMsg:     return "Info";
        case QtWarningMsg:  return "Warning";
        case QtCriticalMsg: return "Critical";
        case QtFatalMsg:    return "Fatal";
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        case QtSystemMsg:   return "System";
#endif
        default:            return "Unknown";
    }
}

QString Mira::demangleClassName(const char* mangledName)
{
#ifdef __GNUG__
    int status = -1;
    char* demangled = abi::__cxa_demangle(mangledName, nullptr, nullptr, &status);
    QString result = (status == 0 && demangled) ? QString::fromUtf8(demangled) : QString::fromUtf8(mangledName);
    free(demangled);
    return result;
#else
    // MSVC already returns human readable name
    return QString::fromUtf8(mangledName);
#endif
}

Mira::Message::operator QString() const {
    const auto& objName = obj.hasName() ? obj.name : demangleClassName(typeid (obj).name());
    const auto& instanceNameMsg = obj.hasInstanceName() ? QString(" [%1]").arg(obj.instanceName) : "";
    return QString("[%1] [%2] [%3#%4]%5 %6")
            .arg(dateTimeStamp.toString("yyyy-MM-dd hh:mm:ss"))
            .arg(toStr(type))
            .arg(objName)
            .arg(obj.instanceId)
            .arg(instanceNameMsg)
            .arg(text);
}

