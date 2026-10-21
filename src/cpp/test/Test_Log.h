#pragma once

#include <QObject>

class Test_Log : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void test();
    void handlerPatternReverse();
    void cleanupTestCase();
};
