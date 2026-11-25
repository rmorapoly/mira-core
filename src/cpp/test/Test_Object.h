#pragma once

#include <QCoreApplication>

#include "LogStateGuard.h"

class Test_Object : public QObject
{
    Q_OBJECT

public:
    explicit Test_Object(QObject *parent = nullptr);

private slots:
    void init();
    void test();
    void cleanup();

private:
    LogStateGuard logStateGuard;
};
