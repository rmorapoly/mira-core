#pragma once

#include <QObject>

class Test : public QObject
{
    Q_OBJECT
public:
    explicit Test(QObject *parent = nullptr);

    /// Should be private slots
private slots:
    void initTestCase();
    void testCase1();
    void cleanupTestCase();
};
