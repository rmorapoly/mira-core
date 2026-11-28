#include "Test.h"
#include "mira/utils/Macros.h"
#include "mira/core/Log.h"
#include "mira/core/Object.h"

#include <QMutex>
#include <QMutexLocker>
#include <QThread>
#include <QVector>

REGISTER_QT_TEST(Test)

namespace {

struct LogRecord
{
    const QObject* object;
    QString message;
};

} // namespace

Test::Test(QObject *parent)
    : QObject{parent}
{}

void Test::init()
{
    logStateGuard.reset();
}

void Test::test_multipleObjects()
{
    class MiraObject : public Mira::Core::Object
    {
    public:
        MiraObject() MIRA_DECLARE_OBJECT {}
        void func() { mInfo() << "func"; }
    };

    MiraObject o1;
    MiraObject o2;
    MiraObject o3;

    QString captured;
    Mira::Core::installMessageHandler();
    Mira::Core::setLogSink([&captured](QtMsgType, const QMessageLogContext&,
                                       const QString& message) {
        captured = message;
    });
    o2.func();
    Mira::Core::setLogSink(nullptr);
    Mira::Core::uninstallMessageHandler();

    QVERIFY(captured.contains(QStringLiteral("MiraObject:%1").arg(o2.instanceId)));
}

void Test::test_multiThread()
{
    class MiraObject : public Mira::Core::Object, public QThread
    {
    public:
        explicit MiraObject(int index)
            MIRA_DECLARE_OBJECT,
            index(index)
        {}

        void func() { mInfo() << "func" << index; }

        void run() override
        {
            for (int i = 0; i < 10; ++i)
                func();
        }

    private:
        const int index;
    };

    QMutex recordsMutex;
    QVector<LogRecord> records;

    MiraObject o1(0);
    MiraObject o2(1);
    MiraObject o3(2);
    MiraObject o4(3);

    Mira::Core::installMessageHandler();
    Mira::Core::setLogSink([&recordsMutex, &records](QtMsgType, const QMessageLogContext&,
                                                    const QString& message) {
        QMutexLocker locker(&recordsMutex);
        records.append({ Mira::Core::currentLogObject(), message });
    });

    o1.start();
    o2.start();
    o3.start();
    o4.start();
    o1.wait();
    o2.wait();
    o3.wait();
    o4.wait();

    Mira::Core::setLogSink(nullptr);
    Mira::Core::uninstallMessageHandler();

    QCOMPARE(records.size(), 40);
    const Mira::Core::Object* objects[] = { &o1, &o2, &o3, &o4 };
    for (const LogRecord& record : records) {
        int objectIndex = -1;
        for (int i = 0; i < 4; ++i) {
            if (record.object == objects[i]) {
                objectIndex = i;
                break;
            }
        }
        QVERIFY(objectIndex >= 0);
        QVERIFY(record.message.contains(QStringLiteral("func %1").arg(objectIndex)));
    }
}

void Test::cleanup()
{
    logStateGuard.reset();
}
