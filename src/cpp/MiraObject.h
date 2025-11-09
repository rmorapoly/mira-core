#pragma once

#include <QObject>
#include <QString>
#include <QHash>
#include <QDebug>
#include "MiraMessage.h"

#define MIRA_DECLARE_OBJECT : Mira::Object(__CLASS_NAME__)

namespace Mira {
    class Object : public QObject
    {
        Q_OBJECT
    public:
        explicit Object(const QString& name,
                        const QString& instanceName = "",
                        QObject* parent = nullptr);
        virtual ~Object();
        bool hasName() const;
        bool hasInstanceName() const;
        const QString name;
        const QString instanceName;
        const int instanceId;
    private:
        void reportMsg(QtMsgType msgType, const QString& msg);
    signals:
    };
} // namespace Mira
