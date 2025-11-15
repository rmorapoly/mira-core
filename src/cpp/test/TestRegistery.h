#pragma once

#include <QTest>
#include <functional>
#include <vector>

// Auto-register a test class
#define REGISTER_QT_TEST(TestClass) \
namespace { \
    struct TestClass##_AutoRegister { \
        TestClass##_AutoRegister() { \
            TESTLIB_SELFCOVERAGE_START(TestClass) \
            QT_PREPEND_NAMESPACE(QTest::Internal::callInitMain)<TestClass>(); \
            TestRegistry::instance().add(#TestClass, \
                     [](int argc, char** argv) { \
                             TestClass test; \
                             return QTest::qExec(&test, argc, argv); \
                     }); \
        } \
    } TestClass##_AutoRegister_Instance; \
}

// Function pointer type for creating and running a test
using TestEntry = std::function<int(int, char**)>;

class TestRegistry {
    TestRegistry();
public:
    static TestRegistry& instance();
    void add(const QString& name, TestEntry entry);
    int runAll(int argc, char* argv[]);

private:
    struct TestInfo {
        QString name;
        TestEntry entry;
    };
    std::vector<TestInfo> tests;
};
