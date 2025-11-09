#pragma once

#include <QObject>
#include "MiraMessage.h"

namespace Mira {
    class Object;
    class MessageManager : public QObject
    {
        Q_OBJECT
    public:
        explicit MessageManager(QObject *parent = nullptr);
        void report(const Message& msg) const;
        void reportConstructed(const Object& obj) const;
        void reportDestructed(const Object& obj) const;
    signals:
        void reported(const Message& msg) const;


    };
}

