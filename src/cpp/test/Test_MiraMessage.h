#pragma once

#include <QCoreApplication>

class Test_MiraMessage : public QObject
{
    Q_OBJECT

public:
    Test_MiraMessage();
    ~Test_MiraMessage();

private slots:
    void initTestCase();
    void test_stringify();
    void test_className();
    void test_message();
    void test_messageManager();
    void cleanupTestCase();
};
