#pragma once

#include <QObject>

class Test_ObjectRegistry : public QObject
{
    Q_OBJECT
public:
    explicit Test_ObjectRegistry(QObject *parent = nullptr);
private slots:
    void test();
};
