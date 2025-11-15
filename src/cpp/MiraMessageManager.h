#pragma once

#include <QObject>
#include "MiraMessage.h"

namespace Mira {
    class Object;
    class MessageManager : public QObject
    {
        Q_OBJECT
        friend class Object;
    public:
        explicit MessageManager(QObject *parent = nullptr);
        void report(const Message& msg) const;
    private:
        void reportDestructed(Object* obj) const;
        void reportConstructed(Object* obj) const;
    signals:
        void reported(const Message& msg) const;
    };
}
