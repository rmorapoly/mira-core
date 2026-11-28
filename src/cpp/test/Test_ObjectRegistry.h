#pragma once

#include <QObject>

#include "LogStateGuard.h"

class Test_ObjectRegistry : public QObject
{
    Q_OBJECT

public:
    explicit Test_ObjectRegistry(QObject *parent = nullptr);

private slots:
    void init();
    void test();
    void concurrentRegistration();
    void cleanup();

private:
    LogStateGuard logStateGuard;
};
