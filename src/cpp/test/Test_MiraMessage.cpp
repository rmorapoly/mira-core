#include "Test_MiraMessage.h"
#include "MiraMessage.h"
#include "MiraObject.h"
#include "TestRegistery.h"
#include "MiraCore.h"
#include "MiraMessageManager.h"

REGISTER_QT_TEST(Test_MiraMessage)

Test_MiraMessage::Test_MiraMessage()
{

}

Test_MiraMessage::~Test_MiraMessage()
{

}

void Test_MiraMessage::initTestCase()
{

}

void Test_MiraMessage::cleanupTestCase()
{

}

void Test_MiraMessage::test_stringify()
{
    // testIdentifiers
    QCOMPARE(QString(STRINGIFY(abc)), QStringLiteral("abc"));
    QCOMPARE(QString(STRINGIFY(my_identifier)), QStringLiteral("my_identifier"));
    // numeric token becomes characters in the string
    QCOMPARE(QString(STRINGIFY(12345)), QStringLiteral("12345"));
    // arithmetic tokens are stringified as-token (if passed as token sequence)
    QCOMPARE(QString(STRINGIFY(1+2)), QStringLiteral("1+2"));
    // string literal when stringified includes the quotes
    QCOMPARE(QString(STRINGIFY("hello")), QStringLiteral("\"hello\""));
    // Define a macro and ensure STRINGIFY expands it before stringifying
#define __EXPR 1+2
    QCOMPARE(QString(STRINGIFY(__EXPR)), QStringLiteral("1+2"));
    // Macro that expands to a string literal — expansion happens first,
    // so the result includes the quotes from the literal
#define QUOTED "hi"
    QCOMPARE(QString(STRINGIFY(QUOTED)), QStringLiteral("\"hi\""));
    // Define a macro and show that STRINGIFY_IMPL does NOT expand the macro name
#define B 42
    // STRINGIFY_IMPL(B) -> "B" (no expansion)
    QCOMPARE(QString(STRINGIFY_IMPL(B)), QStringLiteral("B"));
    // But STRINGIFY(B) -> "42" (because STRINGIFY forces expansion first)
    QCOMPARE(QString(STRINGIFY(B)), QStringLiteral("42"));
    // clean-up local macros
#undef EXPR
#undef QUOTED
#undef B
}

#define __NAME_METHOD_DEF \
    public: \
    QString name() {return __CLASS_NAME__;}

struct A {
    __NAME_METHOD_DEF
};

namespace S {
    class B_b {
    public:
        __NAME_METHOD_DEF
    };
}

class Inherit : public A {
    __NAME_METHOD_DEF
    class Nested {
    __NAME_METHOD_DEF
    };
    Nested n;
};

void Test_MiraMessage::test_className()
{
    class Local {
        __NAME_METHOD_DEF
            class Local_Nested {
                __NAME_METHOD_DEF
            };
            Local_Nested ln;
    };
#undef __NAME_METHOD_DEF

    // Struct
    A a;
    QCOMPARE(a.name(), "A");

    // Class namespace
    S::B_b b;
    QCOMPARE(b.name(), "S::B_b");

    // Class
    QCOMPARE(__CLASS_NAME__, "Test_MiraMessage");

    // Inheritance
    Inherit h;
    QCOMPARE(h.name(), "Inherit");

    // Nested
    Inherit nes;
    QCOMPARE(nes.n.name(), "Inherit::Nested");

    // Local class
    Local l;
    QCOMPARE(l.name(), "Local");

    // Local nested class
    Local l2;
    QCOMPARE(l2.ln.name(), "Local::Local_Nested");
}

void Test_MiraMessage::test_message()
{
    using namespace Mira;

    Object obj("LocalObj");

    Message msg{
        "Debug msg",
        QtDebugMsg,
        &obj,
        QDateTime::currentDateTime()
    };

    // Just see
    qDebug().noquote() << msg;

    class L : public Object {
    public:
        L() : Object("") {}
    };

    L l;

    // Info message
    Message msg2{
        "Info msg",
        QtInfoMsg,
        &l,
        QDateTime::currentDateTime()
    };

    // Just see
    qDebug().noquote() << msg2;

    // Warning, critical, fatal
    Message msg3{
        "Warning msg",
        QtWarningMsg,
        &l,
        QDateTime::currentDateTime()
    };

    Message msg4{
        "Critical msg",
        QtCriticalMsg,
        &l,
        QDateTime::currentDateTime()
    };

    Message msg5{
        "Fatal msg",
        QtFatalMsg,
        &l,
        QDateTime::currentDateTime()
    };

    Core::getInstance()->getMessageManager().report({"Only Text"});

    // Report messages
    Core::getInstance()->getMessageManager().report(msg3);
    Core::getInstance()->getMessageManager().report(msg4);
    // cause fatal
//    Core::getInstance()->getMessageManager().report(msg5);

}

void Test_MiraMessage::test_messageManager()
{
    using namespace Mira;

    miraInfo() << "test msg";

    class I : public Object {
    public:
        I() MIRA_DECLARE_OBJECT {};
        void callee() {
            makeMessageHelper(QtInfoMsg) << "inside I";
            miraInfo() << "inside I";
        }
    };

    I i;
    i.callee();

    // miraLog("hello");
}

