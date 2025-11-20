#pragma once

#include <QObject>

class Test_MiraObjectRegistry : public QObject
{
    Q_OBJECT
public:
    explicit Test_MiraObjectRegistry(QObject *parent = nullptr);
private slots:
    void test();
};
