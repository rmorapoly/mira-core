#pragma once

#include <QtGlobal>
#include <QDateTime>
#include "MiraCoreImport.h"

// double expansion (most useful version works fine for macros)
#define STRINGIFY_IMPL(x) #x
#define STRINGIFY(x) STRINGIFY_IMPL(x)

#define __CLASS_NAME__ Mira::demangleClassName(typeid (*this).name())

namespace Mira {
    class Object;
    extern MIRA_CORE_EXPORT QString toStr(QtMsgType msgType);
    extern MIRA_CORE_EXPORT QString demangleClassName(const char* mangledName);
    struct MIRA_CORE_EXPORT Message {
        QString text;
        QtMsgType type;
        Object* obj;
        QDateTime dateTimeStamp;
        Message(QString text = "",
                QtMsgType type = QtInfoMsg,
                Object* obj = nullptr,
                QDateTime dateTimeStamp = QDateTime::currentDateTime());
        operator QString() const;
        inline QString toStr() const {
            return operator QString();
        }
    };
}

