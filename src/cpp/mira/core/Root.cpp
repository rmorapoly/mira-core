#include "Root.h"

using namespace Mira::Core;

Root::Root(QObject* parent) :
    QObject(parent),
    objectRegistry(this)
{}

ObjectRegistry& Root::registry()
{
    return objectRegistry;
}
