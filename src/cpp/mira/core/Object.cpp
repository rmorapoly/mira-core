#include "Object.h"
#include "mira/Root.h"

namespace Mira {
    namespace Core {

        Object::Object(const QString& name,
                       const QString& instanceName,
                       QObject* parent) :
            QObject(parent),
            name(name.isEmpty() ? __CLASS_NAME__ : name),
            instanceName(instanceName),
            instanceId(Root::getInstance()->registry().generateInstanceId(this->name))
        {
            Root::getInstance()->registry().add(this);
            mDebug() << "Constructed";
        }

        Object::~Object()
        {
            Root::getInstance()->registry().remove(this);
            mDebug() << "Destructed";
        }

        bool Object::hasName() const
        {
            return name != __CLASS_NAME__ && name.size() > 0;
        }

        bool Object::hasInstanceName() const
        {
            return instanceName.size() > 0;
        }

        bool operator==(const Object& obj1, const Object& obj2)
        {
            return obj1.name == obj2.name && obj1.instanceName == obj2.instanceName && obj1.instanceId == obj2.instanceId;
        }

        bool operator!=(const Object& obj1, const Object& obj2)
        {
            return !(obj1 == obj2);
        }

    } // namespace Core
} // namespace Mira
