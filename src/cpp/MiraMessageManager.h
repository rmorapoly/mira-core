#pragma once

#include <QObject>
#include "MiraMessage.h"
#include <QTextStream>
#include <QDebug>

// Works everywhere
// #define miraLog(...)        Mira::makeMessageHelper(Mira::ObjectContext(this), QtInfoMsg) << __VA_ARGS__
// #define miraDebug(...)      Mira::makeMessageHelper(Mira::ObjectContext(this), QtDebugMsg) << __VA_ARGS__
// #define miraWarning(...)    Mira::makeMessageHelper(Mira::ObjectContext(this), QtWarningMsg) << __VA_ARGS__
// #define miraCritical(...)   Mira::makeMessageHelper(Mira::ObjectContext(this), QtCriticalMsg) << __VA_ARGS__
// #define miraFatal(...)      Mira::makeMessageHelper(Mira::ObjectContext(this), QtFatalMsg) << __VA_ARGS__

// #define miraLog()        Mira::makeMessageHelper(Mira::ObjectContext(this), QtInfoMsg)
// #define miraDebug()      Mira::makeMessageHelper(Mira::ObjectContext(this), QtDebugMsg)
// #define miraWarning()    Mira::makeMessageHelper(Mira::ObjectContext(this), QtWarningMsg)
// #define miraCritical()   Mira::makeMessageHelper(Mira::ObjectContext(this), QtCriticalMsg)
// #define miraFatal()      Mira::makeMessageHelper(Mira::ObjectContext(this), QtFatalMsg)

#define miraInfo() Mira::makeMessageHelper(QtInfoMsg)

namespace Mira {
    class Object;
    class MessageManager : public QObject
    {
        Q_OBJECT
        friend class Object;
    public:
        explicit MessageManager(QObject *parent = nullptr);
        void report(const Message& msg) const;
    private:
        void reportDestructed(Object* obj) const;
        void reportConstructed(Object* obj) const;
    signals:
        void reported(const Mira::Message& msg) const;
    };

    template <typename T>
    constexpr const Mira::Object* ObjectContext(T* ptr) {
        if constexpr (std::is_base_of<Mira::Object, T>::value)
            return static_cast<Mira::Object*>(ptr);
        else
            return nullptr;
    }

    class MessageHelper
    {
    public:
        MessageHelper(const Object* obj, QtMsgType type);
        ~MessageHelper();

        template<typename T>
        MessageHelper& operator<<(const T& value) {
            QTextStream ts(&buffer);
            ts << value;
            return *this;
        }

    private:
        const Object* obj;
        QtMsgType type;
        QString buffer;
    };

    // Factory: inside object → obj is "this"
    // outside → obj is nullptr
    inline MessageHelper makeMessageHelper(QtMsgType type) {
        return MessageHelper(nullptr, type);
    }
}
