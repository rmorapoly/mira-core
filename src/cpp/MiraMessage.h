#pragma once

#include <QtGlobal>
#include <QDateTime>

// double expansion (most useful version works fine for macros)
#define STRINGIFY_IMPL(x) #x
#define STRINGIFY(x) STRINGIFY_IMPL(x)

#define __CLASS_NAME__ Mira::demangleClassName(typeid (*this).name())

namespace Mira {
    class Object;
    extern QString toStr(QtMsgType msgType);
    extern QString demangleClassName(const char* mangledName);
    struct Message {
        QtMsgType type;
        QDateTime dateTimeStamp;
        const Object& obj;
        QString text;
        operator QString() const;
    };
}

