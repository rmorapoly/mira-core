#pragma once

#include <QCoreApplication>

class Test_Meta : public QObject
{
    Q_OBJECT

public:
    Test_Meta();
    ~Test_Meta();

private slots:
    void initTestCase();
    void test_stringify();
    void test_className();
    void test_toStr();
    void cleanupTestCase();
};
