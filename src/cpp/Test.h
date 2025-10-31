#pragma once

#include <QtTest>
#include <QCoreApplication>

class Test : public QObject
{
    Q_OBJECT

public:
    Test();
    ~Test();

private slots:
    void initTestCase();
    void testCase1();
    void test_case2();
    void cleanupTestCase();
};
