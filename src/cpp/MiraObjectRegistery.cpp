#include "MiraObjectRegistery.h"
#include "MiraObject.h"

using namespace Mira;

ObjectRegistery::ObjectRegistery(QObject* parent) :
    QObject(parent)
{}

int ObjectRegistery::generateInstanceId(const QString& objName) const
{
//    if (objNameToObjMap.contains(objName)) {
        return objNameToObjMap[objName].size();
//    }
//    else
        //        return 0;
}

int ObjectRegistery::count(const QString& objName) const
{
    return objNameToObjMap[objName].size();
}

void ObjectRegistery::add(Object* obj)
{
    // Commented belowings for test
//    if (!objNameToObjMap.contains(obj->name)) {
//        objNameToObjMap[obj->name] = QMap<int, Object*>{};
//    }
    objNameToObjMap[obj->name][obj->instanceId] = obj;
}

void ObjectRegistery::remove(Object* obj)
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

Object* ObjectRegistery::get(const QString& objName, int instanceId)
{
    return objNameToObjMap[objName][instanceId];
//    if (objNameToObjMap.contains(objName))
//        if (objNameToObjMap[objName].contains(instanceId))
//            return objna
}
