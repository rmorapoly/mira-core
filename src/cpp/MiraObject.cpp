#include "MiraObject.h"
#include "MiraCore.h"

using namespace Mira;

Object::Object(const QString& name,
               const QString& instanceName,
               QObject* parent) :
    QObject(parent),
    name(name.isEmpty() ? __CLASS_NAME__ : name),
    instanceName(instanceName),
    instanceId(Core::getInstance()->getObjectRegistery().generateInstanceId(this->name))
{
    Core::getInstance()->getObjectRegistery().add(this);
    miraDebug() << "Constructed";
}

Object::~Object()
{
    Core::getInstance()->getObjectRegistery().remove(this);
    miraDebug() << "Destructed";
}

bool Object::hasName() const
{
    return name != __CLASS_NAME__ && name.size() > 0;
}

bool Object::hasInstanceName() const
{
    return instanceName.size() > 0;
}

bool Mira::operator==(const Object &obj1, const Object &obj2)
{
    return obj1.name == obj2.name && obj1.instanceName == obj2.instanceName && obj1.instanceId == obj2.instanceId;
}

bool Mira::operator!=(const Object &obj1, const Object &obj2)
{
    return !(obj1 == obj2);
}

MessageHelper Object::makeMessageHelper(QtMsgType type) const {
    return MessageHelper(this, type);
}
