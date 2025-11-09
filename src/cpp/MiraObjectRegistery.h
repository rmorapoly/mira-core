#pragma once

#include <QObject>
#include <QMap>

namespace Mira {
    class Object;
    class ObjectRegistery : public QObject
    {
        Q_OBJECT
    public:
        explicit ObjectRegistery(QObject *parent = nullptr);
        int generateInstanceId(const QString& objName) const;
        int count(const QString& objName) const;
        void add(Object* obj);
        void remove(Object* obj);
        Object* get(const QString& objName, int instanceId = 0);
    private:
        QMap<QString, QMap<int, Object*>> objNameToObjMap;
    };

}

