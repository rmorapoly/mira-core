#pragma once

#include <QObject>

class Test_MiraMessageManager : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void test();
    void cleanupTestCase();
};
