#include "MiraObjectRegistry.h"
#include "MiraObject.h"

using namespace Mira;

ObjectRegistry::ObjectRegistry(QObject* parent) :
    QObject(parent)
{}

int ObjectRegistry::generateInstanceId(const QString& objName) const
{
    return objNameToObjMap[objName].size();
}

int ObjectRegistry::count(const QString& objName) const
{
    return objNameToObjMap[objName].size();
}

void ObjectRegistry::add(Object* obj)
{
    objNameToObjMap[obj->name][obj->instanceId] = obj;
}

void ObjectRegistry::remove(Object* obj)
{
    objNameToObjMap[obj->name].remove(obj->instanceId);
}

Object* ObjectRegistry::get(const QString& objName, int instanceId) const
{
    return objNameToObjMap[objName][instanceId];
}

Object *ObjectRegistry::get(int instanceId) const
{
    return get(STRINGIFY(Mira::Object), instanceId);
}
