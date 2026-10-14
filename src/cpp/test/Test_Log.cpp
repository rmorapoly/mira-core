#include "Test_Log.h"
#include "TestRegistery.h"
#include "mira/core/Log.h"
#include "mira/core/Object.h"

#include <QStringList>
#include <QList>

REGISTER_QT_TEST(Test_Log)

namespace {

QStringList* g_captured = nullptr;
QList<QtMsgType>* g_types = nullptr;
QList<const QObject*>* g_objects = nullptr;

void captureMessage(QtMsgType type, const QMessageLogContext&, const QString& msg)
{
    if (g_captured)
        *g_captured << msg;
    if (g_types)
        *g_types << type;
    if (g_objects)
        *g_objects << Mira::Core::currentLogObject();
}

/// Installs a message handler for the lifetime of the object and restores it
/// afterwards, even if a QTest assertion returns early.
struct MessageCapture
{
    QStringList messages;
    QList<QtMsgType> types;
    QList<const QObject*> objects;
    QtMessageHandler previous = nullptr;

    MessageCapture()
    {
        g_captured = &messages;
        g_types = &types;
        g_objects = &objects;
        previous = qInstallMessageHandler(captureMessage);
    }

    ~MessageCapture()
    {
        qInstallMessageHandler(previous);
        g_captured = nullptr;
        g_types = nullptr;
        g_objects = nullptr;
    }
};

} // namespace

void Test_Log::initTestCase()
{

}

void Test_Log::cleanupTestCase()
{
    // Restore the default handler/pattern so later tests and the app keep them.
    Mira::Core::installLogHandler();
}

void Test_Log::test()
{
    MessageCapture capture;

    class Tracked : public Mira::Core::Object {
    public:
        Tracked() MIRA_DECLARE_OBJECT {}
        void work() {
            miraInfo() << "working" << 42;
            miraWarning() << "careful";
        }
    };

    Tracked tracked;
    tracked.work();

    // The raw message carries only the user text; the object context is added
    // by the message handler through the pattern placeholders.
    const int infoIndex = capture.messages.indexOf(QStringLiteral("working 42"));
    QVERIFY(infoIndex >= 0);
    QCOMPARE(capture.types.at(infoIndex), QtInfoMsg);
    QCOMPARE(capture.objects.at(infoIndex), static_cast<const QObject*>(&tracked));

    const int warningIndex = capture.messages.indexOf(QStringLiteral("careful"));
    QVERIFY(warningIndex >= 0);
    QCOMPARE(capture.types.at(warningIndex), QtWarningMsg);
    QCOMPARE(capture.objects.at(warningIndex), static_cast<const QObject*>(&tracked));

    // Plain qDebug() (no Mira object) has no context object.
    qInfo() << "from-plain-qt";
    const int plainIndex = capture.messages.indexOf(QStringLiteral("from-plain-qt"));
    QVERIFY(plainIndex >= 0);
    QCOMPARE(capture.objects.at(plainIndex), static_cast<const QObject*>(nullptr));

    // The pattern expands the Mira placeholders from the emitting object.
    Mira::Core::Object tagged(QStringLiteral("MyObj"), QStringLiteral("MyIns"));

    Mira::Core::setLogPattern(QStringLiteral("%{objectName}|%{objectInstanceId}|%{objectInstanceName}|%{message}"));
    QCOMPARE(Mira::Core::formatLogMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("hello"), &tagged),
             QStringLiteral("MyObj|0|MyIns|hello"));

    Mira::Core::setLogPattern(QStringLiteral("[%{type}]%{object} %{message}"));
    QCOMPARE(Mira::Core::formatLogMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("hello"), &tagged),
             QStringLiteral("[info]MyObj:0#MyIns hello"));

    // No Mira object -> the Mira placeholders expand to `unknown`.
    QCOMPARE(Mira::Core::formatLogMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("hello"), nullptr),
             QStringLiteral("[info]unknown hello"));
}
