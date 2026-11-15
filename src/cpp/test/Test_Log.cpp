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
    // Nothing has set a pattern yet: logPattern() reports the default. Only
    // setLogPattern() ever changes it - install and uninstall never do.
    QCOMPARE(Mira::Core::logPattern(), Mira::Core::defaultPattern());
}

void Test_Log::uninstallKeepsPattern()
{
    // setLogPattern() leaves Mira's placeholder-free expansion of the pattern in
    // Qt's process-wide pattern; installLogHandler() does not touch the pattern,
    // so foreign handlers keep rendering it.
    Mira::Core::setLogPattern(QStringLiteral("[%{type}] %{message}"));
    Mira::Core::installLogHandler();
    QCOMPARE(qFormatLogMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("m")),
             QStringLiteral("[info] m"));

    // uninstallLogHandler() leaves the pattern exactly as it was: neither the
    // pattern Mira reports nor the one Qt renders is reset behind the
    // application's back.
    Mira::Core::uninstallLogHandler();
    QCOMPARE(Mira::Core::logPattern(), QStringLiteral("[%{type}] %{message}"));
    QCOMPARE(qFormatLogMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("m")),
             QStringLiteral("[info] m"));

    // Handing Qt's own formatting back is the application's explicit choice.
    Mira::Core::setLogPattern(Mira::Core::qtDefaultPattern());
    QCOMPARE(qFormatLogMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("m")), QStringLiteral("m"));

    // Qt's default prefixes the logging category when there is one.
    const QMessageLogContext ctx("f.cpp", 1, "fn", "cat");
    QCOMPARE(qFormatLogMessage(QtInfoMsg, ctx, QStringLiteral("m")), QStringLiteral("cat: m"));
}

void Test_Log::installKeepsUserPattern()
{
    // A pattern set by the user survives install/reinstall: the install must not
    // fall back to the default pattern.
    Mira::Core::setLogPattern(QStringLiteral("user-pattern %{message}"));
    Mira::Core::installLogHandler();
    Mira::Core::installLogHandler();
    QCOMPARE(Mira::Core::logPattern(), QStringLiteral("user-pattern %{message}"));
    QCOMPARE(qFormatLogMessage(QtInfoMsg, QMessageLogContext(), QStringLiteral("m")),
             QStringLiteral("user-pattern m"));

    Mira::Core::reinstallLogHandler();
    QCOMPARE(Mira::Core::logPattern(), QStringLiteral("user-pattern %{message}"));

    Mira::Core::uninstallLogHandler();
}

void Test_Log::messageIndexIncrements()
{
    // %{messageIndex} is Mira's row count: it advances by one per formatted
    // message. Qt has no token of its own here, so Mira consumes it.
    Mira::Core::setLogPattern(QStringLiteral("%{messageIndex}|%{message}"));

    const QString first = Mira::Core::formatLogMessage(QtInfoMsg, QMessageLogContext(),
                                                       QStringLiteral("one"), nullptr);
    const QString second = Mira::Core::formatLogMessage(QtInfoMsg, QMessageLogContext(),
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
    Mira::Core::setLogPattern(QStringLiteral("%{message}"));
    QCOMPARE(Mira::Core::formatLogMessage(QtInfoMsg, QMessageLogContext(),
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
    Mira::Core::setLogPattern(QStringLiteral("%{message}"));
    Mira::Core::installLogHandler();

    QStringList sunk;
    Mira::Core::setLogSink([&sunk](QtMsgType, const QMessageLogContext&, const QString& message) {
        sunk << message;
    });

    miraInfo() << "sink-me";

    // Removed before any assertion, so an early return cannot leave a sink
    // pointing at the dead local.
    Mira::Core::setLogSink(nullptr);
    Mira::Core::uninstallLogHandler();

    QVERIFY(sunk.contains(QStringLiteral("sink-me")));

    // Without a sink nothing is collected anymore.
    Mira::Core::installLogHandler();
    miraInfo() << "after-sink";
    Mira::Core::uninstallLogHandler();
    QVERIFY(!sunk.contains(QStringLiteral("after-sink")));
}

void Test_Log::patternAccessors()
{
    // Both defaults are public, so they can be inspected, extended or restored.
    QVERIFY(Mira::Core::defaultPattern().contains(QStringLiteral("%{message}")));
    QVERIFY(Mira::Core::defaultPattern().contains(QStringLiteral("%{messageIndex}")));
    QCOMPARE(Mira::Core::qtDefaultPattern(),
             QStringLiteral("%{if-category}%{category}: %{endif}%{message}"));

    Mira::Core::setLogPattern(Mira::Core::defaultPattern());
    QCOMPARE(Mira::Core::logPattern(), Mira::Core::defaultPattern());
}

void Test_Log::cleanupTestCase()
{
    // Restore the default pattern and handler so later tests and the app keep them.
    Mira::Core::setLogPattern(Mira::Core::defaultPattern());
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
