#pragma once

#include <QObject>

#include "LogStateGuard.h"

class Test_Log : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase();
    void init();
    void test();
    void uninstallKeepsPattern();
    void installKeepsUserPattern();
    void messageIndexIncrements();
    void attributionUsesThis();
    void attributionOutsideObject();
    void logSinkReceivesMessages();
    void patternAccessors();
    void cleanup();

private:
    LogStateGuard logStateGuard;
};
