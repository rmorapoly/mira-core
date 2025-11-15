#include "Test.h"
#include "TestRegistery.h"

REGISTER_QT_TEST(Test)

Test::Test(QObject *parent)
    : QObject{parent}
{}

void Test::initTestCase()
{

}

void Test::testCase1()
{

}

void Test::cleanupTestCase()
{

}
