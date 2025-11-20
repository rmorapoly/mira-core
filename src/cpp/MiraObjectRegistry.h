#pragma once

#include <QObject>
#include <QMap>
#include "MiraMessage.h"

namespace Mira {

    class Object;

    /**
     * @class ObjectRegistry
     * @brief Maintains a mapping of Mira::Object instances for lookup and management.
     *
     * `ObjectRegistry` provides:
     * - Counting objects by name.
     * - Retrieving objects by name and instance ID.
     * - Internal tracking of object lifetimes.
     *
     * The registry is typically managed by `Mira::Core`, and `Mira::Object`
     * is declared as a friend so that it can register/unregister itself automatically.
     *
     * ### Example:
     * @code
     * auto& registry = Mira::Core::getInstance()->getObjectRegistry();
     * Object* obj = new Object("MyObject");
     * int total = registry.count("MyObject");       // Get number of instances
     * Object* sameObj = registry.get("MyObject");   // Retrieve first instance
     * @endcode
     */
    class MIRA_CORE_EXPORT ObjectRegistry : public QObject
    {
        Q_OBJECT
        friend class Object;

    public:
        /**
         * @brief Constructs an ObjectRegistry.
         * @param parent Optional QObject parent.
         */
        explicit ObjectRegistry(QObject *parent = nullptr);

        /**
         * @brief Returns the number of objects with the given name.
         *
         * @param objName The object name to count (default: "Mira::Object").
         * @return The number of registered objects with the specified name.
         */
        int count(const QString& objName = STRINGIFY(Mira::Object)) const;

        /**
         * @brief Retrieves a specific object by name and instance ID.
         *
         * @param objName The object name to search for.
         * @param instanceId The instance id (default: 0).
         * @return Pointer to the object if found, nullptr otherwise.
         */
        Object* get(const QString& objName, int instanceId = 0) const;

        /**
         * @brief Retrieves an object by its unique instance ID and default name (Mira::Object).
         *
         * @param instanceId The unique instance ID.
         * @return Pointer to the object if found, nullptr otherwise.
         */
        Object* get(int instanceId) const;

    private:
        QMap<QString, QMap<int, Object*>> objNameToObjMap; ///< Internal mapping of objects by name and instance ID.

        /**
         * @brief Generates a new instance ID for a given object name.
         * @param objName The name of the object.
         * @return A unique integer ID for this object instance.
         */
        int generateInstanceId(const QString& objName) const;

        /**
         * @brief Registers a new object into the registry.
         *
         * Called by `Mira::Object` when it is constructed.
         * @param obj Pointer to the object to add.
         */
        void add(Object* obj);

        /**
         * @brief Removes an object from the registry.
         *
         * Called by `Mira::Object` when it is destructed.
         * @param obj Pointer to the object to remove.
         */
        void remove(Object* obj);
    };

} // namespace Mira
