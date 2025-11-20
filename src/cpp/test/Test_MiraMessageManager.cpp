#include "Test_MiraMessageManager.h"
#include "TestRegistery.h"
#include "MiraMessageManager.h"
#include "MiraObject.h"
#include "MiraCore.h"

REGISTER_QT_TEST(Test_MiraMessageManager)

void Test_MiraMessageManager::initTestCase()
{

}

void Test_MiraMessageManager::test()
{
    miraInfo() << "call test";

    using namespace Mira;

    const auto& mm = Core::getInstance()->getMessageManager();
    int state = 0;

    connect(&mm, &MessageManager::reported, this, [&state](const Mira::Message& msg){
        switch (state) {
            case 0:
                QCOMPARE(msg.type, QtInfoMsg);
                QVERIFY(msg.toStr().contains(toStr(QtInfoMsg)));
                break;
            case 1:
                QCOMPARE(msg.type, QtDebugMsg);
                QVERIFY(msg.toStr().contains(toStr(QtDebugMsg)));
                break;
            case 2:
                QCOMPARE(msg.type, QtWarningMsg);
                QVERIFY(msg.toStr().contains(toStr(QtWarningMsg)));
                break;
            case 3:
                QCOMPARE(msg.type, QtCriticalMsg);
                QVERIFY(msg.toStr().contains(toStr(QtCriticalMsg)));
                break;
            case 4:
                QCOMPARE(msg.type, QtFatalMsg);
                QVERIFY(msg.toStr().contains(toStr(QtFatalMsg)));
                break;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
            case 5:
                QCOMPARE(msg.type, QtSystemMsg);
                QVERIFY(msg.toStr().contains(toStr(QtSystemMsg)));
                break;
#endif
            case 6:
                QVERIFY(msg.toStr().contains("I"));
                QVERIFY(msg.toStr().contains("Constructed"));
                break;
            case 7:
                QVERIFY(msg.toStr().contains("I"));
                break;
            case 8:
                QVERIFY(msg.toStr().contains("I"));
                QVERIFY(msg.toStr().contains("Destructed"));
                break;
            case 9:
                QVERIFY(msg.toStr().contains("msg1 msg2 msg3"));
                break;
            default:
                break;
        }
        ++state;
    }, Qt::DirectConnection);

    // emit messages in order to test
    miraInfo() << "info msg";             // state = 0
    miraDebug() << "debug msg";           // state = 1
    miraWarning() << "warning msg";       // state = 2
    miraCritical() << "critical msg";     // state = 3

    // fatal message may terminate the app; handle carefully in test
    // miraFatal() << "fatal msg";        // optional
    state++;

#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    miraSystem() << "system msg";         // state = 5
#else
    state++;
#endif

    class I : public Mira::Object {
    public:
        I() MIRA_DECLARE_OBJECT {};
        void callee() {
            miraInfo() << "inside I";
        }
    };

    {
        I i; // state = 6
        i.callee(); // state = 7
        // Destruction state = 8
    }

    miraInfo() << "msg1" << "msg2" << "msg3"; // state = 9
}

void Test_MiraMessageManager::cleanupTestCase()
{

}
