#pragma once

#include <QObject>
#include "MiraMessage.h"
#include <QTextStream>
#include <QDebug>

#define miraInfo()      makeMessageHelper(QtInfoMsg)
#define miraDebug()     makeMessageHelper(QtDebugMsg)
#define miraWarning()   makeMessageHelper(QtWarningMsg)
#define miraCritical()  makeMessageHelper(QtCriticalMsg)
#define miraFatal()     makeMessageHelper(QtFatalMsg)

namespace Mira {
    class Object;

    /// For next version
    // class MessageSink : public QObject {
    //     Q_OBJECT
    //     friend class MessageManager;
    // public:
    //     explicit MessageSink(QObject *parent = nullptr);
    // private:
    //     void consume(const Message& msg);
    // };

    class MessageManager : public QObject
    {
        Q_OBJECT
    public:
        explicit MessageManager(QObject *parent = nullptr);
        void report(const Message& msg) const;
    signals:
        void reported(const Mira::Message& msg) const;
    };

    class MessageHelper
    {
    public:
        MessageHelper(const Object* obj, QtMsgType type);
        ~MessageHelper();

        template<typename T>
        MessageHelper& operator<<(const T& value) {
            QTextStream ts(&buffer);
            ts << value << " ";
            return *this;
        }

    private:
        const Object* obj;
        QtMsgType type;
        QString buffer;
    };
}

inline Mira::MessageHelper makeMessageHelper(QtMsgType type)
{
    return Mira::MessageHelper(nullptr, type);
}
