#include "TestRegistery.h"

static auto ins = TestRegistry::instance();

TestRegistry::TestRegistry() {}

TestRegistry &TestRegistry::instance() {
    static TestRegistry reg;
    return reg;
}

void TestRegistry::add(const QString &name, TestEntry entry) {
    tests.push_back({name, entry});
}

int TestRegistry::runAll(int argc, char *argv[]) {
    int exitCode = 0;
    for (auto& t : tests) {
        exitCode |= t.entry(argc, argv);
    }
    return exitCode;
}
