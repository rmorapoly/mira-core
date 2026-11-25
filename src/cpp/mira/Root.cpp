#include "Root.h"

using namespace Mira;

Root::Root(QObject* parent) :
    QObject(parent),
    objectRegistry(this)
{}

Core::ObjectRegistry& Root::registry()
{
    return objectRegistry;
}
