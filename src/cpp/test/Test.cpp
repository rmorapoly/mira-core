#include "Test.h"
#include "TestRegistery.h"
#include "mira/core/Log.h"
#include "mira/core/Object.h"

REGISTER_QT_TEST(Test)

Test::Test(QObject *parent)
    : QObject{parent}
{}

void Test::initTestCase()
{
    Mira::Core::reinstallLogHandler();
}

void Test::testCase1()
{
    class D : public Mira::Core::Object {
    public:
        D() : Mira::Core::Object("D", "i1") {
            mInfo() << "my d";

        }
    };

    mInfo() << "log pattern" << Mira::Core::logPattern();
    D d;
    mInfo() << "test mInfo";
}

void Test::cleanupTestCase()
{
    Mira::Core::uninstallLogHandler();

}
