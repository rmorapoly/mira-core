#include "MiraMessage.h"
#include "MiraObject.h"
#ifdef __GNUG__
#include <cxxabi.h>
#include <typeinfo>
#endif

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

    return str;
#endif
}

Message::Message(QString text, QtMsgType type, Object* obj, QDateTime dateTimeStamp)
    : text(std::move(text))
    , type(type)
    , obj(obj)
    , dateTimeStamp(std::move(dateTimeStamp))
{}

Mira::Message::operator QString() const {
    QString objNameIdMsg = "";
    if (obj) {
        const auto& objName = obj->hasName() ? obj->name : demangleClassName(typeid (*obj).name());
        objNameIdMsg = QString(" [%1#%2]").arg(objName).arg(obj->instanceId);
    }
    const auto& instanceNameMsg = (obj && obj->hasInstanceName()) ? QString(" [%1]").arg(obj->instanceName) : "";
    return QString("[%1] [%2]%3%4 %5")
            .arg(dateTimeStamp.toString("yyyy-MM-dd hh:mm:ss.zzz"))
            .arg(toStr(type))
            .arg(objNameIdMsg)
            .arg(instanceNameMsg)
            .arg(text);
}
