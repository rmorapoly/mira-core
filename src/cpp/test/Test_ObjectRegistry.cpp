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

    {
        // Create a few objects (they auto-register)
        Object obj0, obj1;

        // After creation, registry should contain entries
        QCOMPARE(registry.count(), 2);

        // Get should return the correct pointers
        QCOMPARE(registry.get(0), &obj0);
        QCOMPARE(registry.get(1), &obj1);
        QCOMPARE(obj0 == obj1, false);
    }

    // After destruction (end of scope), registry should be empty
    QCOMPARE(registry.count(), 0);

    // Add dynamically allocated object
    Object *obj2 = new Object("Obj");
    Object *obj3 = new Object("Obj");
    QCOMPARE(registry.count("Obj"), 2);
    QCOMPARE(registry.get("Obj"), obj2);
    QCOMPARE(*registry.get("Obj"), *obj2);
    QCOMPARE(*registry.get("Obj", 1), *obj3);

    delete obj2;
    delete obj3;
    QCOMPARE(registry.count("Obj"), 0);
}
