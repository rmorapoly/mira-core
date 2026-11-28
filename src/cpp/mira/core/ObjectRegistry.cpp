#include "ObjectRegistry.h"
#include "Object.h"

#include <QMutexLocker>

using namespace Mira::Core;

ObjectRegistry::ObjectRegistry(QObject* parent) :
    QObject(parent),
    objectCount(0)
{}

int ObjectRegistry::generateInstanceId(const QString& objName)
{
    QMutexLocker locker(&mutex);

    const auto& instances = objNameToObjMap.value(objName, {});
    const auto& reserved = reservedInstanceIds.value(objName, {});

    // Pick the first free slot so deleting an object makes its id available
    // again and live objects keep a dense 0..count-1 range. Reserving the id
    // before returning keeps concurrent constructors from colliding.
    int id = 0;
    while (instances.contains(id)
           || reserved.contains(id))
        ++id;

    reservedInstanceIds[objName].insert(id);
    return id;
}

int ObjectRegistry::count(const QString& objName) const
{
    QMutexLocker locker(&mutex);
    return objNameToObjMap.value(objName, {}).size();
}

int ObjectRegistry::countAll() const
{
    QMutexLocker locker(&mutex);
    return objectCount;
}

void ObjectRegistry::add(Object* obj)
{
    QMutexLocker locker(&mutex);

    auto& reserved = reservedInstanceIds[obj->name];
    reserved.remove(obj->instanceId);
    if (reserved.isEmpty())
        reservedInstanceIds.remove(obj->name);

    QMap<int, Object*>& instances = objNameToObjMap[obj->name];
    if (!instances.contains(obj->instanceId))
        ++objectCount;
    instances[obj->instanceId] = obj;
}

void ObjectRegistry::remove(Object* obj)
{
    QMutexLocker locker(&mutex);
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
    QMutexLocker locker(&mutex);
    return objNameToObjMap.value(objName, {}).value(instanceId, nullptr);
}

Object* ObjectRegistry::get(int instanceId) const
{
    return get(STRINGIFY(Mira::Core::Object), instanceId);
}
