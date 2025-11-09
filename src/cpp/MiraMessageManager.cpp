#include "MiraMessageManager.h"
#include <QtDebug>

using namespace Mira;

MessageManager::MessageManager(QObject* parent) :
    QObject(parent)
{}

void MessageManager::report(const Message& msg) const
{
    qInfo().noquote() << msg;
    emit reported(msg);
}

void MessageManager::reportConstructed(const Object& obj) const
{
    Message msg{QtMsgType::QtDebugMsg, QDateTime::currentDateTime(), obj, "Constructed"};
    report(msg);
}

void MessageManager::reportDestructed(const Object& obj) const
{
    Message msg{QtMsgType::QtDebugMsg, QDateTime::currentDateTime(), obj, "Destructed"};
    report(msg);
}

