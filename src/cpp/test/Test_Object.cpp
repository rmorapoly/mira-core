#include "Test_Object.h"
#include "mira/core/Object.h"
#include "TestRegistery.h"

REGISTER_QT_TEST(Test_Object)

using namespace Mira::Core;

Test_Object::Test_Object(QObject *parent)
    : QObject{parent}
{}

void Test_Object::initTestCase()
{
    Mira::Core::reinstallLogHandler();
}

void Test_Object::test()
{
    Object obj("");
    QCOMPARE(obj.hasName(), false);
    QCOMPARE(obj.instanceId, 0);
    QCOMPARE(obj.hasInstanceName(), false);
    Object obj2(STRINGIFY(Mira::Core::Object));
    QCOMPARE(obj2.hasName(), false);
    QCOMPARE(obj2.instanceId, 1);
    QCOMPARE(obj2.hasInstanceName(), false);
    Object obj3("MyObj", "MyIns");
    QVERIFY(obj3.hasName());
    QVERIFY(obj3.hasInstanceName());
    QCOMPARE(obj3.name, "MyObj");
    QCOMPARE(obj3.instanceName, "MyIns");
    class ChildObj : public Object {
    public:
        explicit ChildObj() MIRA_DECLARE_OBJECT {}
        ~ChildObj() override {}
    };
    const ChildObj child0, child1;
    QCOMPARE(child0.name, STRINGIFY(ChildObj));
    QCOMPARE(child0.instanceId, 0);
    QCOMPARE(child1.instanceId, 1);
}

void Test_Object::cleanupTestCase() {
    Mira::Core::uninstallLogHandler();
}
