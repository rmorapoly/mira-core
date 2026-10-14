#pragma once

#include <QCoreApplication>

class Test_Object : public QObject
{
    Q_OBJECT
public:
    explicit Test_Object(QObject *parent = nullptr);

private slots:
    void initTestCase() {};
    void test();
    void cleanupTestCase() {};
};
