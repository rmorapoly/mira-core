#pragma once

#include <QtTest>
#include <QCoreApplication>
#include "MiraObject.h"


class Child : public Mira::Object {
    Q_OBJECT
public:
    explicit Child() MIRA_DECLARE_OBJECT
    {}
    ~Child() override {

    }
};

class Test : public QObject
{
    Q_OBJECT

public:
    Test();
    ~Test();

private slots:
    void initTestCase();
    void testCase1();
    void testCase2();
    void cleanupTestCase();
};
