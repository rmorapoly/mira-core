#pragma once

#include "Meta.h"
#include "TestRegistry.h"

/**
 * @brief Utility macros for control flow in bool-returning functions.
 *
 * These cover the "accumulate outcomes, stop at the first failure" pattern
 * that shows up wherever a sequence of fallible steps must not continue past
 * an error, while still executing every step that has to run for cleanup.
 */
namespace Mira::Utils
{
    /**
     * @brief Accumulates an outcome into @p result and returns false when it fails.
     *
     * Use it in place of a bare `result &= ...` so a failed step returns
     * immediately instead of letting the following statements run against a
     * state that was never reached.
     *
     * Note that the expression is always evaluated exactly once, even when
     * @p result is already false. This is deliberate: steps with side effects
     * that must still run, such as releasing a handshake, are executed as
     * written. Combine with `&&=` semantics only when a later step may safely
     * be skipped.
     *
     * @code
     * bool result {true};
     * CHECK_AND_RETURN_IF_NOT(result, handshake.prepare());
     * CHECK_AND_RETURN_IF_NOT(result, controller.write(entries));
     * CHECK_AND_RETURN_IF_NOT(result, handshake.complete());
     * return result;
     * @endcode
     *
     * @param result Accumulated outcome, updated with `&=` so earlier failures stick.
     * @param ... Expression yielding something convertible to bool.
     * @return From the enclosing function with false when @p result became false.
     */
    #define CHECK_AND_RETURN_IF_NOT(result, ...) \
        do { \
            if (!(result &= (__VA_ARGS__))) \
                return false; \
        } while (false)

    #define RETURN_IF_NOT(...) \
        CHECK_AND_RETURN_IF_NOT(true, (__VA_ARGS__))

    /**
     * @brief Auto-register a Qt test class in the global TestRegistry.
     *
     * This macro creates an unnamed static object whose constructor runs
     * at program startup. The constructor:
     *
     *  - Registers the test with Qt's internal test runner
     *  - Adds the test to TestRegistry for centralized execution
     *
     * Usage:
     * @code
     * REGISTER_QT_TEST(MyTest)
     * @endcode
     *
     * Requirements:
     *  - TestClass must inherit from QObject
     *  - TestClass must use Q_OBJECT
     *  - TestClass must be default-constructible
     *  - TestClass must declares test functions as private slots
     */
    #define REGISTER_QT_TEST(TestClass) \
    namespace { \
        struct TestClass##_AutoRegister { \
            TestClass##_AutoRegister() { \
                TESTLIB_SELFCOVERAGE_START(TestClass) \
                QT_PREPEND_NAMESPACE(QTest::Internal::callInitMain)<TestClass>(); \
                Mira::Utils::TestRegistry::getInstance()->add(#TestClass, \
                         [](int argc, char** argv) { \
                                 TestClass test; \
                                 return QTest::qExec(&test, argc, argv); \
                         }); \
            } \
        } TestClass##_AutoRegister_Instance; \
    }

    /**
     * @brief Run all registered Qt tests.
     *
     * Can be called as RUN_QT_TESTS() or RUN_QT_TESTS(argc, argv).
     *
     * @return Exit code from test execution
     */
    #define __RUN_QT_TESTS_GET(_1, _2, NAME, ...) NAME
    #define __RUN_QT_TESTS_0() Mira::Utils::TestRegistry::getInstance()->runAll(0, nullptr)
    #define __RUN_QT_TESTS_2(argc, argv) Mira::Utils::TestRegistry::getInstance()->runAll(argc, argv)
    #define RUN_QT_TESTS(...) __RUN_QT_TESTS_GET(__VA_ARGS__, __RUN_QT_TESTS_2, __RUN_QT_TESTS_0)(__VA_ARGS__)
}
