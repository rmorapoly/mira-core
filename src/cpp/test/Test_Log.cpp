#include "Test_Log.h"
#include "TestRegistery.h"
#include "mira/core/Log.h"
#include "mira/core/Object.h"

#include <QStringList>
#include <QList>
#include <QtCore/qlogging.h>

REGISTER_QT_TEST(Test_Log)

namespace {

QStringList* capturedMessages = nullptr;
QList<QtMsgType>* capturedTypes = nullptr;
QList<const QObject*>* capturedObjects = nullptr;

void captureMessage(QtMsgType type, const QMessageLogContext&, const QString& msg)
{
    if (capturedMessages)
        *capturedMessages << msg;
    if (capturedTypes)
        *capturedTypes << type;
    if (capturedObjects)
        *capturedObjects << Mira::Core::currentLogObject();
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
        capturedMessages = &messages;
        capturedTypes = &types;
        capturedObjects = &objects;
        previous = qInstallMessageHandler(captureMessage);
    }

    ~MessageCapture()
    {
        qInstallMessageHandler(previous);
        capturedMessages = nullptr;
        capturedTypes = nullptr;
        capturedObjects = nullptr;
    }
};

} // namespace

void Test_Log::initTestCase()
{

}

void Test_Log::handlerPatternReverse()
{
    // installLogHandler() makes Qt use Mira's default pattern; uninstallLogHandler()
    // gives Qt its own built-in default back.
    Mira::Core::installLogHandler();
    QVERIFY(qFormatLogMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("m")) != QStringLiteral("m"));

    Mira::Core::uninstallLogHandler();
    QCOMPARE(qFormatLogMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("m")), QStringLiteral("m"));

    // Qt's default prefixes the logging category when there is one.
    const QMessageLogContext ctx("f.cpp", 1, "fn", "cat");
    QCOMPARE(qFormatLogMessage(QtInfoMsg, ctx, QStringLiteral("m")), QStringLiteral("cat: m"));
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
        void formatting() {
            // The mInfo family returns a real QDebug, so its whole API applies and
            // its quoting rules match qInfo() exactly.
            miraInfo().nospace() << "no" << "space";
            miraInfo() << QStringLiteral("quoted");
            miraInfo().noquote() << QStringLiteral("raw");
            mWarning().nospace() << "m" << 1 << 2;
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

    // QDebug formatting and modifiers behave exactly like qInfo()/qDebug()/...
    tracked.formatting();
    QVERIFY(capture.messages.contains(QStringLiteral("nospace")));
    QVERIFY(capture.messages.contains(QStringLiteral("\"quoted\"")));
    QVERIFY(capture.messages.contains(QStringLiteral("raw")));
    QVERIFY(!capture.messages.contains(QStringLiteral("\"raw\"")));
    QVERIFY(capture.messages.contains(QStringLiteral("m12")));
    QCOMPARE(capture.types.at(capture.messages.indexOf(QStringLiteral("nospace"))), QtInfoMsg);
    QCOMPARE(capture.types.at(capture.messages.indexOf(QStringLiteral("m12"))), QtWarningMsg);

    // The object context is scoped to the mInfo statement: a plain qInfo() that
    // follows has no object.
    qInfo() << "from-plain-qt";
    const int plainIndex = capture.messages.indexOf(QStringLiteral("from-plain-qt"));
    QVERIFY(plainIndex >= 0);
    QCOMPARE(capture.objects.at(plainIndex), static_cast<const QObject*>(nullptr));
    QCOMPARE(capture.objects.at(capture.messages.indexOf(QStringLiteral("nospace"))),
             static_cast<const QObject*>(&tracked));

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

    // Mira's conditional tokens render their block only when the matching
    // placeholder is more than the `unknown` fallback.
    Mira::Core::setLogPattern(QStringLiteral("%{if-object}[%{object}] %{endif}%{message}"));
    QCOMPARE(Mira::Core::formatLogMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("hello"), &tagged),
             QStringLiteral("[MyObj:0#MyIns] hello"));
    QCOMPARE(Mira::Core::formatLogMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("hello"), nullptr),
             QStringLiteral("hello"));

    Mira::Core::setLogPattern(QStringLiteral("%{if-objectName}N=%{objectName} %{endif}%{message}"));
    QCOMPARE(Mira::Core::formatLogMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("hello"), &tagged),
             QStringLiteral("N=MyObj hello"));
    QCOMPARE(Mira::Core::formatLogMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("hello"), nullptr),
             QStringLiteral("hello"));

    Mira::Core::setLogPattern(QStringLiteral("%{if-objectInstanceId}ID=%{objectInstanceId} %{endif}%{message}"));
    QCOMPARE(Mira::Core::formatLogMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("hello"), &tagged),
             QStringLiteral("ID=0 hello"));
    QCOMPARE(Mira::Core::formatLogMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("hello"), nullptr),
             QStringLiteral("hello"));

    // `if-objectInstanceName` is false when the object has no instance name.
    Mira::Core::Object bare(QStringLiteral("Bare"), QString());
    Mira::Core::setLogPattern(QStringLiteral("%{if-objectInstanceName}#%{objectInstanceName}%{endif}%{message}"));
    QCOMPARE(Mira::Core::formatLogMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("hello"), &tagged),
             QStringLiteral("#MyInshello"));
    QCOMPARE(Mira::Core::formatLogMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("hello"), &bare),
             QStringLiteral("hello"));

    // Mira's conditions can enclose Qt's own conditions (and vice versa).
    const QMessageLogContext cat("f.cpp", 1, "fn", "cat");
    Mira::Core::setLogPattern(
                QStringLiteral("%{if-category}C=%{category}%{if-object}[%{object}]%{endif} %{endif}%{message}"));
    QCOMPARE(Mira::Core::formatLogMessage(QtInfoMsg, cat, QStringLiteral("hello"), &tagged),
             QStringLiteral("C=cat[MyObj:0#MyIns] hello"));
    QCOMPARE(Mira::Core::formatLogMessage(QtInfoMsg, cat, QStringLiteral("hello"), nullptr),
             QStringLiteral("C=cat hello"));

    // A Qt-only conditional still works untouched.
    Mira::Core::setLogPattern(QStringLiteral("%{if-category}%{category}: %{endif}%{message}"));
    QCOMPARE(Mira::Core::formatLogMessage(QtInfoMsg, cat, QStringLiteral("hello"), nullptr),
             QStringLiteral("cat: hello"));
    QCOMPARE(Mira::Core::formatLogMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("hello"), nullptr),
             QStringLiteral("hello"));

    // Mira fills %{object*}, Qt fills the rest, and Qt's process-wide pattern is
    // left in the placeholder-free form so foreign handlers never see Mira tokens.
    Mira::Core::setLogPattern(QStringLiteral("[%{objectName}] %{message}"));
    QCOMPARE(qFormatLogMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("m")),
             QStringLiteral("[unknown] m"));

    // Formatting for an object hands Qt the object-specific pattern...
    QCOMPARE(Mira::Core::formatLogMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("m"), &tagged),
             QStringLiteral("[MyObj] m"));
    QCOMPARE(qFormatLogMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("m")),
             QStringLiteral("[MyObj] m"));

    // ...and formatting without an object restores the placeholder-free form, so the
    // object never leaks into the next (possibly foreign) message.
    QCOMPARE(Mira::Core::formatLogMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("m"), nullptr),
             QStringLiteral("[unknown] m"));
    QCOMPARE(qFormatLogMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("m")),
             QStringLiteral("[unknown] m"));

    // Qt placeholders keep Qt's own semantics.
    Mira::Core::setLogPattern(QStringLiteral("[%{type}][%{file}][%{line}][%{function}][%{category}]"));
    const QMessageLogContext ctx("f.cpp", 7, "fn", "cat");
    QCOMPARE(Mira::Core::formatLogMessage(QtWarningMsg, ctx, QStringLiteral("m"), nullptr),
             qFormatLogMessage(QtWarningMsg, ctx, QStringLiteral("m")));
    QCOMPARE(Mira::Core::formatLogMessage(QtWarningMsg, QMessageLogContext(), QStringLiteral("m"), nullptr),
             QStringLiteral("[warning][unknown][0][unknown][]"));
}
