#include "Test_Object.h"
#include "mira/core/Object.h"
#include "mira/utils/Macros.h"

REGISTER_QT_TEST(Test_Object)

using namespace Mira::Core;

Test_Object::Test_Object(QObject *parent)
    : QObject{parent}
{}

void Test_Object::init()
{
    logStateGuard.reset();
}

void Test_Object::test()
{
    Object obj("");
    QCOMPARE(obj.hasName(), false);
    QVERIFY(obj.instanceId >= 0);
    QCOMPARE(obj.hasInstanceName(), false);

    Object obj2(STRINGIFY(Mira::Core::Object));
    QCOMPARE(obj2.hasName(), false);
    QVERIFY(obj.instanceId != obj2.instanceId);
    QCOMPARE(obj2.hasInstanceName(), false);

    Object obj3("MyObj", "MyIns");
    QVERIFY(obj3.hasName());
    QVERIFY(obj3.hasInstanceName());
    QCOMPARE(obj3.name, QStringLiteral("MyObj"));
    QCOMPARE(obj3.instanceName, QStringLiteral("MyIns"));

    class ChildObj : public Object {
    public:
        explicit ChildObj() MIRA_DECLARE_OBJECT {}
        ~ChildObj() override {}
    };
    const ChildObj child0, child1;
    QCOMPARE(child0.name, QStringLiteral("ChildObj"));
    QVERIFY(child0.instanceId != child1.instanceId);
}

void Test_Object::cleanup()
{
    logStateGuard.reset();
}
