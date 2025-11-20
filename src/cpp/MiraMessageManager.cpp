#include "MiraMessageManager.h"
#include <QtDebug>
#include "MiraCore.h"

using namespace Mira;

MessageManager::MessageManager(QObject* parent) :
    QObject(parent)
{
    qRegisterMetaType<Mira::Message>("Mira::Message");
}

void MessageManager::report(const Message& msg) const
{
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
            qFatal("%s", msg.toStr().toUtf8().constData());
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

MessageHelper::MessageHelper(const Object* obj, QtMsgType type) :
    obj(obj),
    type(type)
{}

MessageHelper::~MessageHelper()
{
    if (!buffer.isEmpty()) {
        buffer.chop(1);
        Core::getInstance()->getMessageManager().report({buffer, type, const_cast<Object*>(obj)});
    }
}
