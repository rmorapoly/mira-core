#include "MiraObjectRegistry.h"
#include "MiraObject.h"

using namespace Mira;

ObjectRegistry::ObjectRegistry(QObject* parent) :
    QObject(parent)
{}

int ObjectRegistry::generateInstanceId(const QString& objName) const
{
//    if (objNameToObjMap.contains(objName)) {
        return objNameToObjMap[objName].size();
//    }
//    else
        //        return 0;
}

int ObjectRegistry::count(const QString& objName) const
{
    return objNameToObjMap[objName].size();
}

void ObjectRegistry::add(Object* obj)
{
    // Commented belowings for test
//    if (!objNameToObjMap.contains(obj->name)) {
//        objNameToObjMap[obj->name] = QMap<int, Object*>{};
//    }
    objNameToObjMap[obj->name][obj->instanceId] = obj;
}

void ObjectRegistry::remove(Object* obj)
{
    // Commented belowings for test
//    if (objNameToObjMap.contains(obj->name)) {
//        if (objNameToObjMap[obj->name].contains(obj->instanceId)) {
            objNameToObjMap[obj->name].remove(obj->instanceId);
//            if (objNameToObjMap[obj->name].isEmpty()) {
//                objNameToObjMap.remove(obj->name);
//            }
//        }
//    }
}

Object* ObjectRegistry::get(const QString& objName, int instanceId) const
{
    return objNameToObjMap[objName][instanceId];
//    if (objNameToObjMap.contains(objName))
//        if (objNameToObjMap[objName].contains(instanceId))
//            return objna
}

Object *ObjectRegistry::get(int instanceId) const
{
    return get(STRINGIFY(Mira::Object), instanceId);
}
