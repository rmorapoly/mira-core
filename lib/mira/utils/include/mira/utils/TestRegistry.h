#pragma once

#include <QTest>
#include <functional>
#include <vector>
#include "Singleton.h"

namespace Mira::Utils {
    /**
     * @brief Function type used to execute a registered test.
     *
     * The function receives command-line arguments and returns
     * the exit code produced by QTest::qExec().
     */
    using TestEntry = std::function<int(int, char**)>;

    /**
     * @brief Central registry for all Qt test cases.
     *
     * Test classes register themselves automatically using
     * REGISTER_QT_TEST(). The registry stores each test's
     * name and execution function and allows running them
     * all from a single main().
     *
     * This class follows the singleton pattern.
     */
    class TestRegistry : public Singleton<TestRegistry> {

        friend class Singleton<TestRegistry>;
        /**
         * @brief Private constructor to enforce singleton usage.
         */
        TestRegistry();

    public:

        /**
         * @brief Add a test to the registry.
         *
         * Normally called automatically by REGISTER_QT_TEST().
         *
         * @param name   Human-readable test name (usually the class name)
         * @param entry  Callable that executes the test and returns its exit code
         */
        void add(const QString& name, TestEntry entry);

        /**
         * @brief Run all registered tests.
         *
         * Executes each registered test sequentially and returns
         * a combined exit code (implementation-defined).
         *
         * @param argc Command-line argument count
         * @param argv Command-line argument values
         * @return Exit code
         */
        int runAll(int argc, char* argv[]);

    private:
        /**
         * @brief Internal structure describing a single test.
         */
        struct TestInfo {
            QString name;   ///< Test name
            TestEntry entry; ///< Function executing the test
        };

        std::vector<TestInfo> tests; ///< List of registered tests
    };
}




