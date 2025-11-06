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
    void testCase2();
    void cleanupTestCase();
};
