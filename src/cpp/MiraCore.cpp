#include "MiraCore.h"

using namespace Mira;

static auto instance = Core::getInstance();

Core::Core(QObject* parent) :
    QObject(parent),
    messageManager(this)
{
}

const MessageManager& Core::getMessageManager() const
{
    return messageManager;
}

ObjectRegistery& Core::getObjectRegistery()
{
    return objectRegistery;
}
