#include "Root.h"

using namespace Mira;

static auto instance = Root::getInstance();

Root::Root(QObject* parent) :
    QObject(parent),
    objectRegistry(this)
{}

Core::ObjectRegistry& Root::registry()
{
    return objectRegistry;
}
