#pragma once

#include <QObject>
#include <QMap>
#include "MiraSingleton.h"
#include "MiraMessageManager.h"
#include "MiraObjectRegistery.h"

namespace Mira {
    class Core : public QObject, public Singleton<Core>
    {
        Q_OBJECT
        friend class Singleton<Core>;
        explicit Core(QObject *parent = nullptr);
    public:
        const MessageManager& getMessageManager() const;
        ObjectRegistery& getObjectRegistery();
    private:
        const MessageManager messageManager;
        ObjectRegistery objectRegistery;
        QMap<QString, int> objNameToInstanceId;
    };
}

