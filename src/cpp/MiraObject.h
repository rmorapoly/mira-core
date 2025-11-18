#pragma once

#include <QObject>
#include <QString>
#include <QHash>
#include <QDebug>
#include "MiraMessageManager.h"

#define MIRA_DECLARE_OBJECT : Mira::Object(Mira::demangleClassName(typeid (this).name()))

namespace Mira {
    class Object : public QObject
    {
        Q_OBJECT
    public:
        explicit Object(const QString& name = "",
                        const QString& instanceName = "",
                        QObject* parent = nullptr);
        virtual ~Object();
        const QString name;
        const QString instanceName;
        const int instanceId;
        bool hasName() const;
        bool hasInstanceName() const;
        friend bool operator==(const Object& obj1, const Object& obj2);
        friend bool operator!=(const Object& obj1, const Object& obj2);
    protected:
        MessageHelper makeMessageHelper(QtMsgType type) const;
    };
}
