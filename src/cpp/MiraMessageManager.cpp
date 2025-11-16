#include "MiraMessageManager.h"
#include <QtDebug>
#include "MiraCore.h"

using namespace Mira;

MessageManager::MessageManager(QObject* parent) :
    QObject(parent)
{}

void MessageManager::report(const Message& msg) const
{
//    qInfo().noquote() << msg;
    emit reported(msg);
    switch (msg.type) {
        case QtDebugMsg:
            qDebug().noquote() << msg;
            break;
        case QtInfoMsg:
            qInfo().noquote() << msg;
            break;
        case QtWarningMsg:
            qWarning().noquote() << msg;
            break;
        case QtCriticalMsg:
            qCritical().noquote() << msg;
            break;
        case QtFatalMsg:
            /// aborts the program immediately
            qFatal("%s", msg.operator QString().toUtf8().constData());
            break;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
        case QtSystemMsg:
            qDebug().noquote() << msg;
            break;
#endif
        default:
            qDebug().noquote() << msg;
            break;
    }
}

void MessageManager::reportConstructed(Object* obj) const
{
    Message msg{"Constructed", QtMsgType::QtDebugMsg, obj};
    report(msg);
}

void MessageManager::reportDestructed(Object* obj) const
{
    Message msg{"Destructed", QtMsgType::QtDebugMsg, obj};
    report(msg);
}

MessageHelper::MessageHelper(const Object* obj, QtMsgType type) :
    obj(obj),
    type(type)
{}

MessageHelper::~MessageHelper()
{
    if (!buffer.isEmpty()) {
        Core::getInstance()->getMessageManager().report({buffer, type, const_cast<Object*>(obj)});
    }
}
