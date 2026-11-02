#include "ObjectRegistry.h"
#include "Object.h"

using namespace Mira::Core;

ObjectRegistry::ObjectRegistry(QObject* parent) :
    QObject(parent),
    objectCount(0)
{}

int ObjectRegistry::generateInstanceId(const QString& objName) const
{
    const QMap<int, Object*> instances = objNameToObjMap.value(objName);

    // Start at the number of live instances (the common case: no gap below),
    // then skip over ids that are still registered - removing an object must
    // not hand its id to the next one while another object still uses it.
    int id = instances.size();
    while (instances.contains(id))
        ++id;
    return id;
}

int ObjectRegistry::count(const QString& objName) const
{
    return objNameToObjMap.value(objName).size();
}

int ObjectRegistry::countAll() const
{
    return objectCount;
}

void ObjectRegistry::add(Object* obj)
{
    QMap<int, Object*>& instances = objNameToObjMap[obj->name];
    if (!instances.contains(obj->instanceId))
        ++objectCount;
    instances[obj->instanceId] = obj;
}

void ObjectRegistry::remove(Object* obj)
{
    QMap<QString, QMap<int, Object*>>::iterator named = objNameToObjMap.find(obj->name);
    if (named == objNameToObjMap.end())
        return;

    QMap<int, Object*>& instances = named.value();
    if (instances.contains(obj->instanceId)) {
        instances.remove(obj->instanceId);
        --objectCount;
    }

    // Names are cheap to drop and to recreate; keeping empty buckets would
    // only make count() and the map itself carry dead weight.
    if (instances.isEmpty())
        objNameToObjMap.erase(named);
}

Object* ObjectRegistry::get(const QString& objName, int instanceId) const
{
    return objNameToObjMap.value(objName).value(instanceId, nullptr);
}

Object* ObjectRegistry::get(int instanceId) const
{
    return get(STRINGIFY(Mira::Core::Object), instanceId);
}
