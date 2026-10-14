#include "Test_Meta.h"
#include "TestRegistery.h"
#include "mira/Meta.h"

REGISTER_QT_TEST(Test_Meta)

Test_Meta::Test_Meta()
{

}

Test_Meta::~Test_Meta()
{

}

void Test_Meta::initTestCase()
{

}

void Test_Meta::cleanupTestCase()
{

}

void Test_Meta::test_stringify()
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

void Test_Meta::test_className()
{
    class Local {
        __NAME_METHOD_DEF
            class Local_Nested {
                __NAME_METHOD_DEF
            };
            Local_Nested ln;
    };
#undef __NAME_METHOD_DEF

    // Struct (runtime demangle)
    A a;
    QCOMPARE(a.name(), "A");

    // Class namespace
    S::B_b b;
    QCOMPARE(b.name(), "S::B_b");

    // Class
    QCOMPARE(__CLASS_NAME__, "Test_Meta");

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

    // Compile-time names (RTTI-free), expected to match demangle for these
    QCOMPARE(Mira::className<A>(), QStringLiteral("A"));
    QCOMPARE(Mira::className<S::B_b>(), QStringLiteral("S::B_b"));
    QCOMPARE(Mira::className<Inherit>(), QStringLiteral("Inherit"));
    QCOMPARE(Mira::className<Inherit::Nested>(), QStringLiteral("Inherit::Nested"));
    QCOMPARE(Mira::className<Local>(), QStringLiteral("Local"));
    QCOMPARE(Mira::className<Local::Local_Nested>(), QStringLiteral("Local::Local_Nested"));
}

void Test_Meta::test_toStr()
{
    QCOMPARE(Mira::toStr(QtDebugMsg), QStringLiteral("Debug"));
    QCOMPARE(Mira::toStr(QtInfoMsg), QStringLiteral("Info"));
    QCOMPARE(Mira::toStr(QtWarningMsg), QStringLiteral("Warning"));
    QCOMPARE(Mira::toStr(QtCriticalMsg), QStringLiteral("Critical"));
    QCOMPARE(Mira::toStr(QtFatalMsg), QStringLiteral("Fatal"));
}
