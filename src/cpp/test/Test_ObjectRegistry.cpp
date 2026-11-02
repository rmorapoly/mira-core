#include "Test_ObjectRegistry.h"
#include "mira/Root.h"
#include "mira/core/Object.h"
#include "TestRegistery.h"

REGISTER_QT_TEST(Test_ObjectRegistry)

using namespace Mira;
using namespace Mira::Core;

Test_ObjectRegistry::Test_ObjectRegistry(QObject *parent)
    : QObject{parent}
{}

void Test_ObjectRegistry::test()
{
    // The registry is owned by the Core singleton
    auto& registry = Root::getInstance()->registry();

    // Initially empty
    QCOMPARE(registry.count(), 0);
    QCOMPARE(registry.countAll(), 0);

    {
        // Create a few objects (they auto-register)
        Object obj0, obj1;

        // After creation, registry should contain entries
        QCOMPARE(registry.count(), 2);
        QCOMPARE(registry.countAll(), 2);

        // Get should return the correct pointers
        QCOMPARE(registry.get(0), &obj0);
        QCOMPARE(registry.get(1), &obj1);
        QCOMPARE(obj0 == obj1, false);
    }

    // After destruction (end of scope), registry should be empty
    QCOMPARE(registry.count(), 0);
    QCOMPARE(registry.countAll(), 0);

    // Add dynamically allocated object
    Object *obj2 = new Object("Obj");
    Object *obj3 = new Object("Obj");
    Object *other = new Object("Other");
    QCOMPARE(registry.count("Obj"), 2);
    QCOMPARE(registry.count("Other"), 1);
    QCOMPARE(registry.countAll(), 3);
    QCOMPARE(registry.get("Obj"), obj2);
    QCOMPARE(*registry.get("Obj"), *obj2);
    QCOMPARE(*registry.get("Obj", 1), *obj3);

    // Removing an instance must not hand its id to a later object of the same
    // name while another object still holds it, and countAll() sees every name.
    delete obj2;
    QCOMPARE(registry.count("Obj"), 1);
    QCOMPARE(registry.countAll(), 2);

    Object *obj4 = new Object("Obj");
    QCOMPARE(obj4->instanceId, 2);
    QCOMPARE(registry.get("Obj", 2), obj4);
    QCOMPARE(registry.countAll(), 3);

    delete obj4;
    delete obj3;
    delete other;
    QCOMPARE(registry.count("Obj"), 0);
    QCOMPARE(registry.count("Other"), 0);
    QCOMPARE(registry.countAll(), 0);
}
