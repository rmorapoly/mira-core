#include "Test.h"
#include "TestRegistery.h"
#include "mira/core/Log.h"
#include "mira/core/Object.h"
#include <algorithm>
#include <QThread>

REGISTER_QT_TEST(Test)

Test::Test(QObject *parent)
    : QObject{parent}
{}

void Test::initTestCase()
{
    Mira::Core::reinstallMessageHandler();
    qSetMessagePattern("[%{time yyyy-MM-dd hh:mm:ss.zzz}] %{message}");
}

void Test::test_multipleObjects()
{
    class MiraObject : public Mira::Core::Object {
    public:
        MiraObject() MIRA_DECLARE_OBJECT {}
        void func() {
            mInfo() << "func";
        }
    };
    MiraObject o1;
    MiraObject o2;
    MiraObject o3;
    Mira::Core::setLogSink([](QtMsgType, const QMessageLogContext&,
                              const QString& message){
        Mira::Core::LogSink empty;
        Mira::Core::setLogSink(empty);
        QVERIFY(message.contains("MiraObject:1"));
    });
    o2.func();
}

void Test::test_multiThread()
{
    class MiraObject : public Mira::Core::Object, public QThread {
    public:
        MiraObject(int index)
            MIRA_DECLARE_OBJECT,
            index(index) {
        }
        void func() {
            mInfo() << "func" << index;
        }
        void run() override {
            int i = 0;
            while (i++ < 10) {
                func();
            }
        }
        const int index;
    };
    MiraObject o1(0);
    MiraObject o2(1);
    MiraObject o3(2);
    MiraObject o4(3);
    Mira::Core::setLogSink([](QtMsgType, const QMessageLogContext&,
                              const QString& message){
        static int counter = 0;
        auto expected = QString(message.at(message.size() - 1)).toInt();
        auto actual = QString(message.split(':').last()[0]).toInt();
        QCOMPARE(actual, expected);
        counter++;
        if (counter >= 40) {
            Mira::Core::LogSink empty;
            Mira::Core::setLogSink(empty);
        }
    });
    o1.start();
    o2.start();
    o3.start();
    o4.start();
    o1.wait();
    o2.wait();
    o3.wait();
    o4.wait();
    o1.quit();
    o2.quit();
    o3.quit();
    o4.quit();
}

void Test::cleanupTestCase()
{
    Mira::Core::uninstallMessageHandler();
}
