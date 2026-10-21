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
    // Mira::Core::installLogHandler();

    Mira::Core::installLogHandler();
    Mira::Core::installLogHandler();
    Mira::Core::installLogHandler();
    Mira::Core::reinstallLogHandler();
    Mira::Core::reinstallLogHandler();
    Mira::Core::reinstallLogHandler();
    Mira::Core::reinstallLogHandler();
    Mira::Core::reinstallLogHandler();
}

void Test::testCase1()
{
    class MObject : public Mira::Core::Object {
    public:
        MObject() : Mira::Core::Object("MObject", "ins1") {
            mInfo() << "my mobject msg";
        }
    };

    mInfo() << "log pattern" << Mira::Core::logPattern();
    MObject mObj;
    mInfo() << "test mInfo";
    (::Mira::Core::LogObjectScope(miraLogObject()),
        QMessageLogger(QT_MESSAGELOG_FILE, QT_MESSAGELOG_LINE, QT_MESSAGELOG_FUNC).critical());
}

void Test::cleanupTestCase()
{
    Mira::Core::uninstallLogHandler();

}
