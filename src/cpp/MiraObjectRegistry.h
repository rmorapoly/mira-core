#pragma once

#include <QObject>
#include <QMap>
#include "MiraMessage.h"

namespace Mira {
    class Object;
    class MIRA_CORE_EXPORT ObjectRegistry : public QObject
    {
        Q_OBJECT
        friend class Object;
    public:
        explicit ObjectRegistry(QObject *parent = nullptr);
        int count(const QString& objName = STRINGIFY(Mira::Object)) const;
        Object* get(const QString& objName, int instanceId = 0) const;
        Object* get(int instanceId) const;
    private:
        QMap<QString, QMap<int, Object*>> objNameToObjMap;
        int generateInstanceId(const QString& objName) const;
        void add(Object* obj);
        void remove(Object* obj);
    };
}
