#pragma once

#include <QObject>
#include <QMap>
#include "MiraSingleton.h"
#include "MiraMessageManager.h"
#include "MiraObjectRegistry.h"

namespace Mira {
    class MIRA_CORE_EXPORT Core : public QObject, public Singleton<Core>
    {
        Q_OBJECT
        friend class Singleton<Core>;
        explicit Core(QObject *parent = nullptr);
    public:
        const MessageManager& getMessageManager() const;
        ObjectRegistry& getObjectRegistery();
    private:
        const MessageManager messageManager;
        ObjectRegistry objectRegistery;
        QMap<QString, int> objNameToInstanceId;
    };
}

