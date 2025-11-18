#pragma once

#include <QCoreApplication>

class Test_MiraObject : public QObject
{
    Q_OBJECT
public:
    explicit Test_MiraObject(QObject *parent = nullptr);

private slots:
    void initTestCase() {};
    void test();
    void cleanupTestCase() {};
};
