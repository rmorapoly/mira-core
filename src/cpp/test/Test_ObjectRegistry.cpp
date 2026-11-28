#include "Test_ObjectRegistry.h"
#include "mira/core/Root.h"
#include "mira/core/Object.h"
#include "mira/utils/Macros.h"

#include <QThread>
#include <QVector>

REGISTER_QT_TEST(Test_ObjectRegistry)

using namespace Mira::Core;

namespace {

    class RegistrationWorker : public QThread
    {
    public:
        explicit RegistrationWorker(QObject* parent = nullptr) :
            QThread(parent)
        {}

    protected:
        void run() override
        {
            for (int i = 0; i < 50; ++i) {
                Object object(QStringLiteral("Threaded"));
                Q_UNUSED(object)
            }
        }
    };

} // namespace

Test_ObjectRegistry::Test_ObjectRegistry(QObject *parent)
    : QObject{parent}
{}

void Test_ObjectRegistry::init()
{
    logStateGuard.reset();
}

void Test_ObjectRegistry::test()
{
    auto& registry = Root::getInstance()->registry();
    const int initialAll = registry.countAll();
    const int initialDefault = registry.count();

    {
        Object obj0, obj1;
        QCOMPARE(registry.count(), initialDefault + 2);
        QCOMPARE(registry.countAll(), initialAll + 2);
        QCOMPARE(registry.get(obj0.instanceId), &obj0);
        QCOMPARE(registry.get(obj1.instanceId), &obj1);
        QVERIFY(obj0 != obj1);
    }

    QCOMPARE(registry.count(), initialDefault);
    QCOMPARE(registry.countAll(), initialAll);

    const int initialNamed = registry.count(QStringLiteral("Obj"));
    const int initialOther = registry.count(QStringLiteral("Other"));

    Object *obj2 = new Object("Obj");
    Object *obj3 = new Object("Obj");
    Object *other = new Object("Other");
    const int obj2Id = obj2->instanceId;
    const int obj3Id = obj3->instanceId;
    QCOMPARE(registry.count("Obj"), initialNamed + 2);
    QCOMPARE(registry.count("Other"), initialOther + 1);
    QCOMPARE(registry.countAll(), initialAll + 3);
    QCOMPARE(registry.get("Obj"), obj2);
    QCOMPARE(registry.get("Obj", obj2Id), obj2);
    QCOMPARE(registry.get("Obj", obj3Id), obj3);

    // Deleting an object releases its id for reuse and keeps the live range
    // gap-free from 0.
    delete obj2;
    QCOMPARE(registry.count("Obj"), initialNamed + 1);
    QCOMPARE(registry.countAll(), initialAll + 2);
    QCOMPARE(registry.get("Obj", obj2Id), nullptr);
    QCOMPARE(registry.get("Obj", obj3Id), obj3);

    Object *obj4 = new Object("Obj");
    QCOMPARE(obj4->instanceId, obj2Id);
    QVERIFY(obj4->instanceId != obj3Id);
    QCOMPARE(registry.get("Obj", obj4->instanceId), obj4);
    QCOMPARE(registry.countAll(), initialAll + 3);

    const int releasedId = obj4->instanceId;
    delete obj4;
    Object *obj5 = new Object("Obj");
    QCOMPARE(obj5->instanceId, releasedId);
    QCOMPARE(registry.get("Obj", obj5->instanceId), obj5);

    for (int id = 0; id < registry.count("Obj"); ++id)
        QVERIFY(registry.get("Obj", id) != nullptr);
    QVERIFY(registry.get("Obj", registry.count("Obj")) == nullptr);

    delete obj5;
    delete obj3;
    delete other;
    QCOMPARE(registry.count("Obj"), initialNamed);
    QCOMPARE(registry.count("Other"), initialOther);
    QCOMPARE(registry.countAll(), initialAll);
}

void Test_ObjectRegistry::concurrentRegistration()
{
    auto& registry = Root::getInstance()->registry();
    const int initialAll = registry.countAll();
    const int initialNamed = registry.count(QStringLiteral("Threaded"));

    QtMessageHandler previous = qInstallMessageHandler(
                                    [](QtMsgType, const QMessageLogContext&, const QString&) {});

    QVector<RegistrationWorker*> workers;
    for (int i = 0; i < 4; ++i) {
        auto* worker = new RegistrationWorker;
        workers.append(worker);
        worker->start();
    }
    for (RegistrationWorker* worker : workers) {
        worker->wait();
        delete worker;
    }

    qInstallMessageHandler(previous);

    QCOMPARE(registry.countAll(), initialAll);
    QCOMPARE(registry.count("Threaded"), initialNamed);
}

void Test_ObjectRegistry::cleanup()
{
    logStateGuard.reset();
}
