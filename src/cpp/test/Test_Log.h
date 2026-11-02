#pragma once

#include <QObject>

class Test_Log : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void test();
    void uninstallKeepsPattern();
    void installKeepsUserPattern();
    void messageIndexIncrements();
    void logSinkReceivesMessages();
    void patternAccessors();
    void cleanupTestCase();
};
