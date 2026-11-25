#pragma once

#include <QObject>

#include "LogStateGuard.h"

class Test : public QObject
{
    Q_OBJECT

public:
    explicit Test(QObject *parent = nullptr);

private slots:
    void init();
    void test_multipleObjects();
    void test_multiThread();
    void cleanup();

private:
    LogStateGuard logStateGuard;
};
