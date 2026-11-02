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
            // The `scope` member is initialized before this body, so the object is
            // in scope from here on: every message logged while it lives is
            // attributed to it.
            Root::getInstance()->registry().add(this);
            miraDebug() << "Constructed";
        }

        Object::~Object()
        {
            Root::getInstance()->registry().remove(this);
            miraDebug() << "Destructed";

            // The `scope` member is destroyed after this body, so the message above
            // is still attributed to the object; it restores the previous scope
            // when it goes away.
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
