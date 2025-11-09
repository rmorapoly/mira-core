#include "Test.h"
#include "MiraMessage.h"



Test::Test()
{

}

Test::~Test()
{

}

void Test::initTestCase()
{

}

void Test::cleanupTestCase()
{

}

void Test::testCase1()
{
    Child a, b, c;
    Mira::Message msg{QtMsgType::QtCriticalMsg, QDateTime::currentDateTime(), b, "My Msg"};
    qInfo() << "msg" << msg;
}

void Test::testCase2()
{

}
