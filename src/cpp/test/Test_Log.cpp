#include "Test_Log.h"
#include "mira/utils/Macros.h"
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

    void logFromFreeFunction()
    {
        miraInfo() << "from free function";
    }

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
    // Nothing has set a pattern yet: messagePattern() reports the default. Only
    // setMessagePattern() ever changes it - install and uninstall never do.
    QCOMPARE(Mira::Core::messagePattern(), Mira::Core::defaultMessagePattern());
}

void Test_Log::init()
{
    logStateGuard.reset();
}

void Test_Log::uninstallKeepsPattern()
{
    // Qt's process-wide pattern is the application's: Mira never writes it, so
    // installing or uninstalling the handler cannot change what Qt renders.
    qSetMessagePattern(QStringLiteral("[%{type}] %{message}"));
    QCOMPARE(qFormatLogMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("m")),
             QStringLiteral("[info] m"));

    Mira::Core::installMessageHandler();
    Mira::Core::uninstallMessageHandler();
    QCOMPARE(qFormatLogMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("m")),
             QStringLiteral("[info] m"));

    // Mira's own pattern is untouched by install/uninstall as well.
    Mira::Core::setMessagePattern(QStringLiteral("<%{message}>"));
    Mira::Core::installMessageHandler();
    Mira::Core::uninstallMessageHandler();
    QCOMPARE(Mira::Core::messagePattern(), QStringLiteral("<%{message}>"));

    // The two patterns compose: Qt renders its pattern, and its %{message} is
    // Mira's expansion.
    QCOMPARE(Mira::Core::formatMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("m"), nullptr),
             QStringLiteral("[info] <m>"));

    // Qt's own formatting stays available through its own token.
    Mira::Core::setMessagePattern(Mira::Core::defaultMessagePattern());
    qSetMessagePattern(Mira::Core::qtDefaultMessagePattern());
    const QMessageLogContext ctx("f.cpp", 1, "fn", "cat");
    QCOMPARE(qFormatLogMessage(QtInfoMsg, ctx, QStringLiteral("m")), QStringLiteral("cat: m"));
}

void Test_Log::installKeepsUserPattern()
{
    // Qt's pattern is only the wrapper; keep the expected lines deterministic.
    qSetMessagePattern(QStringLiteral("%{message}"));

    // A pattern set by the user survives install/reinstall: the install must not
    // fall back to the default pattern.
    Mira::Core::setMessagePattern(QStringLiteral("user-pattern %{message}"));
    Mira::Core::installMessageHandler();
    Mira::Core::installMessageHandler();
    QCOMPARE(Mira::Core::messagePattern(), QStringLiteral("user-pattern %{message}"));

    Mira::Core::reinstallMessageHandler();
    QCOMPARE(Mira::Core::messagePattern(), QStringLiteral("user-pattern %{message}"));

    QCOMPARE(Mira::Core::formatMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("m"), nullptr),
             QStringLiteral("user-pattern m"));

    Mira::Core::uninstallMessageHandler();
}

void Test_Log::messageIndexIncrements()
{
    // Qt's pattern only wraps Mira's part, so it is set to the message alone.
    qSetMessagePattern(QStringLiteral("%{message}"));

    // %{messageIndex} is Mira's row count: it advances by one per formatted
    // message. Qt has no token of its own here, so Mira fills it in.
    Mira::Core::setMessagePattern(QStringLiteral("%{messageIndex}|%{message}"));

    const QString first = Mira::Core::formatMessage(QtInfoMsg, QMessageLogContext(),
                                                    QStringLiteral("one"), nullptr);
    const QString second = Mira::Core::formatMessage(QtInfoMsg, QMessageLogContext(),
                                                     QStringLiteral("two"), nullptr);

    QVERIFY(first.endsWith(QStringLiteral("|one")));
    QVERIFY(second.endsWith(QStringLiteral("|two")));
    QVERIFY(!first.contains(QStringLiteral("%{messageIndex}")));

    const int firstBar = first.indexOf(QLatin1Char('|'));
    const int secondBar = second.indexOf(QLatin1Char('|'));
    QVERIFY(firstBar > 0);
    QCOMPARE(second.mid(0, secondBar).toULongLong(),
             first.mid(0, firstBar).toULongLong() + 1);

    // Without the token the index takes no part in the output.
    Mira::Core::setMessagePattern(QStringLiteral("%{message}"));
    QCOMPARE(Mira::Core::formatMessage(QtInfoMsg, QMessageLogContext(),
                                       QStringLiteral("plain"), nullptr),
             QStringLiteral("plain"));
}

void Test_Log::attributionUsesThis()
{
    MessageCapture capture;

    class Tracked : public Mira::Core::Object
    {
    public:
        Tracked() MIRA_DECLARE_OBJECT {}
        void who() { miraInfo() << "who"; }
    };

    // Several objects of one type alive at once: a message is attributed to the
    // object whose member function runs, not to the innermost/last one created.
    Tracked first;
    Tracked second;
    Tracked third;
    first.who();
    third.who();

    QCOMPARE(first.miraLogObject(), static_cast<const QObject*>(&first));

    const int firstIndex = capture.messages.indexOf(QStringLiteral("who"));
    QVERIFY(firstIndex >= 0);
    QCOMPARE(capture.objects.at(firstIndex), static_cast<const QObject*>(&first));

    const int thirdIndex = capture.messages.lastIndexOf(QStringLiteral("who"));
    QVERIFY(thirdIndex > firstIndex);
    QCOMPARE(capture.objects.at(thirdIndex), static_cast<const QObject*>(&third));
}

void Test_Log::attributionOutsideObject()
{
    // Outside any Mira object the macros fall back to the global
    // miraLogObject(), which attributes nothing.
    QCOMPARE(miraLogObject(), static_cast<const QObject*>(nullptr));

    MessageCapture capture;
    logFromFreeFunction();

    const int index = capture.messages.indexOf(QStringLiteral("from free function"));
    QVERIFY(index >= 0);
    QCOMPARE(capture.objects.at(index), static_cast<const QObject*>(nullptr));
}

void Test_Log::logSinkReceivesMessages()
{
    // The sink is an additional consumer: the message still goes to the console
    // and the same formatted line is handed to the sink.
    qSetMessagePattern(QStringLiteral("%{message}"));
    Mira::Core::setMessagePattern(QStringLiteral("%{message}"));
    Mira::Core::installMessageHandler();

    QStringList sunk;
    Mira::Core::setLogSink([&sunk](QtMsgType, const QMessageLogContext&, const QString& message) {
        sunk << message;
    });

    miraInfo() << "sink-me";

    // Removed before any assertion, so an early return cannot leave a sink
    // pointing at the dead local.
    Mira::Core::setLogSink(nullptr);
    Mira::Core::uninstallMessageHandler();

    QVERIFY(sunk.contains(QStringLiteral("sink-me")));

    // Without a sink nothing is collected anymore.
    Mira::Core::installMessageHandler();
    miraInfo() << "after-sink";
    Mira::Core::uninstallMessageHandler();
    QVERIFY(!sunk.contains(QStringLiteral("after-sink")));
}

void Test_Log::patternAccessors()
{
    // Both defaults are public, so they can be inspected, extended or restored.
    QVERIFY(Mira::Core::defaultMessagePattern().contains(QStringLiteral("%{message}")));
    QVERIFY(Mira::Core::defaultMessagePattern().contains(QStringLiteral("%{messageIndex}")));
    QCOMPARE(Mira::Core::qtDefaultMessagePattern(),
             QStringLiteral("%{if-category}%{category}: %{endif}%{message}"));

    qSetMessagePattern(Mira::Core::qtDefaultMessagePattern());
    QCOMPARE(qFormatLogMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("m")),
             QStringLiteral("m"));
    const QMessageLogContext categorized("f.cpp", 1, "fn", "cat");
    QCOMPARE(qFormatLogMessage(QtInfoMsg, categorized, QStringLiteral("m")),
             QStringLiteral("cat: m"));

    // Mira's default holds Mira's part only: Qt's tokens are Qt's to render.
    QVERIFY(!Mira::Core::defaultMessagePattern().contains(QStringLiteral("%{time")));
    QVERIFY(!Mira::Core::defaultMessagePattern().contains(QStringLiteral("%{type}")));

    Mira::Core::setMessagePattern(Mira::Core::defaultMessagePattern());
    QCOMPARE(Mira::Core::messagePattern(), Mira::Core::defaultMessagePattern());
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

            // A capturing lambda keeps `this`, so the statement is still
            // attributed to the object whose member function is running.
            [this]() { miraInfo() << "from capturing lambda"; }();
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

    const int capturingLambdaIndex = capture.messages.indexOf(QStringLiteral("from capturing lambda"));
    QVERIFY(capturingLambdaIndex >= 0);
    QCOMPARE(capture.objects.at(capturingLambdaIndex), static_cast<const QObject*>(&tracked));

    // Outside a Mira object nothing is attributed: the global fallback the
    // macros resolve to there returns `nullptr`.
    miraInfo() << "outside any object";
    const int outsideIndex = capture.messages.indexOf(QStringLiteral("outside any object"));
    QVERIFY(outsideIndex >= 0);
    QCOMPARE(capture.objects.at(outsideIndex), static_cast<const QObject*>(nullptr));
    QCOMPARE(miraLogObject(), static_cast<const QObject*>(nullptr));

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

    // Mira's pattern covers Mira's part of the line only. Qt's pattern is set
    // here so the expected strings are deterministic; Mira never writes it.
    qSetMessagePattern(QStringLiteral("%{message}"));
    Mira::Core::Object tagged(QStringLiteral("MyObj"), QStringLiteral("MyIns"));

    Mira::Core::setMessagePattern(QStringLiteral("%{objectName}|%{objectInstanceId}|%{objectInstanceName}|%{message}"));
    QCOMPARE(Mira::Core::formatMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("hello"), &tagged),
             QStringLiteral("MyObj|0|MyIns|hello"));

    // No Mira object -> the Mira placeholders expand to `unknown`.
    QCOMPARE(Mira::Core::formatMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("hello"), nullptr),
             QStringLiteral("unknown|unknown|unknown|hello"));

    // Mira's conditional tokens render their block only when the matching
    // placeholder is more than the `unknown` fallback.
    Mira::Core::setMessagePattern(QStringLiteral("%{if-object}[%{object}] %{endif}%{message}"));
    QCOMPARE(Mira::Core::formatMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("hello"), &tagged),
             QStringLiteral("[MyObj:0#MyIns] hello"));
    QCOMPARE(Mira::Core::formatMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("hello"), nullptr),
             QStringLiteral("hello"));

    Mira::Core::setMessagePattern(QStringLiteral("%{if-objectName}N=%{objectName} %{endif}%{message}"));
    QCOMPARE(Mira::Core::formatMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("hello"), &tagged),
             QStringLiteral("N=MyObj hello"));
    QCOMPARE(Mira::Core::formatMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("hello"), nullptr),
             QStringLiteral("hello"));

    Mira::Core::setMessagePattern(QStringLiteral("%{if-objectInstanceId}ID=%{objectInstanceId} %{endif}%{message}"));
    QCOMPARE(Mira::Core::formatMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("hello"), &tagged),
             QStringLiteral("ID=0 hello"));
    QCOMPARE(Mira::Core::formatMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("hello"), nullptr),
             QStringLiteral("hello"));

    // `if-objectInstanceName` is false when the object has no instance name.
    Mira::Core::Object bare(QStringLiteral("Bare"), QString());
    Mira::Core::setMessagePattern(QStringLiteral("%{if-objectInstanceName}#%{objectInstanceName}%{endif}%{message}"));
    QCOMPARE(Mira::Core::formatMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("hello"), &tagged),
             QStringLiteral("#MyInshello"));
    QCOMPARE(Mira::Core::formatMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("hello"), &bare),
             QStringLiteral("hello"));

    // A pattern without `%{message}` is a prefix: the message follows it.
    Mira::Core::setMessagePattern(QStringLiteral("[%{objectName}] "));
    QCOMPARE(Mira::Core::formatMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("hello"), &tagged),
             QStringLiteral("[MyObj] hello"));
    QCOMPARE(Mira::Core::formatMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("hello"), nullptr),
             QStringLiteral("[unknown] hello"));

    // An empty Mira pattern leaves the message alone.
    Mira::Core::setMessagePattern(QString());
    QCOMPARE(Mira::Core::formatMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("hello"), &tagged),
             QStringLiteral("hello"));

    // The two patterns compose: Qt's %{message} is Mira's expansion.
    qSetMessagePattern(QStringLiteral("QT<%{message}>"));
    Mira::Core::setMessagePattern(QStringLiteral("MIRA<%{message}>"));
    QCOMPARE(Mira::Core::formatMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("hello"), nullptr),
             QStringLiteral("QT<MIRA<hello>>"));

    // Qt's placeholders keep Qt's own semantics inside Qt's pattern, and Mira
    // leaves that pattern exactly as the application wrote it.
    const QMessageLogContext ctx("f.cpp", 7, "fn", "cat");
    qSetMessagePattern(QStringLiteral("[%{type}][%{file}][%{line}][%{function}][%{category}] %{message}"));
    Mira::Core::setMessagePattern(QStringLiteral("%{message}"));
    QCOMPARE(Mira::Core::formatMessage(QtWarningMsg, ctx, QStringLiteral("m"), nullptr),
             qFormatLogMessage(QtWarningMsg, ctx, QStringLiteral("m")));
    QCOMPARE(Mira::Core::formatMessage(QtWarningMsg, QMessageLogContext(), QStringLiteral("m"), nullptr),
             QStringLiteral("[warning][unknown][0][unknown][] m"));
    QCOMPARE(qFormatLogMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("plain")),
             QStringLiteral("[info][unknown][0][unknown][] plain"));

    // Restore both defaults for the tests that follow.
    qSetMessagePattern(Mira::Core::qtDefaultMessagePattern());
    Mira::Core::setMessagePattern(Mira::Core::defaultMessagePattern());
}

void Test_Log::cleanup()
{
    logStateGuard.reset();
}
