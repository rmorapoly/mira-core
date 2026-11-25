#pragma once

#include <QObject>
#include <QMap>
#include <QMutex>
#include <QSet>
#include "MiraCoreImport.h"
#include "mira/utils/Meta.h"

namespace Mira {
    namespace Core {

        class Object;

        /**
         * @class ObjectRegistry
         * @brief Maintains a mapping of `Mira::Core::Object` instances for lookup and management.
         *
         * `ObjectRegistry` provides:
         * - Counting objects by name.
         * - Retrieving objects by name and instance ID.
         * - Gap-free instance IDs: deleted IDs are reused so live objects of
         *   one name occupy 0..count-1.
         * - Internal tracking of object lifetimes.
         *
         * All public operations and the registration hooks used by `Object` are
         * mutex-guarded, so objects can be registered from different threads.
         *
         * The registry is typically managed by `Mira::Root`, and `Object`
         * is declared as a friend so that it can register/unregister itself automatically.
         *
         * ### Example:
         * @code
         * auto& registry = Mira::Root::getInstance()->registry();
         * Object* obj = new Object("MyObject");
         * int total = registry.count("MyObject");   // instances of one name
         * int all = registry.countAll();            // every registered object
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
             * @param objName The object name to count (default: "Mira::Core::Object").
             * @return The number of registered objects with the specified name.
             */
            int count(const QString& objName = STRINGIFY(Mira::Core::Object)) const;

            /**
             * @brief Returns the number of all registered objects.
             *
             * Counts every `Mira::Core::Object` the registry currently holds,
             * across all names - the counterpart of @ref count, which counts one
             * name only.
             *
             * @return The total number of registered objects.
             */
            int countAll() const;

            /**
             * @brief Retrieves a specific object by name and instance ID.
             *
             * @param objName The object name to search for.
             * @param instanceId The instance id (default: 0).
             * @return Pointer to the object if found, nullptr otherwise.
             */
            Object* get(const QString& objName, int instanceId = 0) const;

            /**
             * @brief Retrieves an object by its unique instance ID and default name (Mira::Core::Object).
             *
             * @param instanceId The unique instance ID.
             * @return Pointer to the object if found, nullptr otherwise.
             */
            Object* get(int instanceId) const;

        private:
            QMap<QString, QMap<int, Object*>> objNameToObjMap; ///< Internal mapping of objects by name and instance ID.
            QMap<QString, QSet<int>> reservedInstanceIds;      ///< IDs handed out by generateInstanceId() but not yet registered by add().
            int objectCount;                                   ///< Number of objects across all names, kept by add/remove.
            mutable QMutex mutex;                              ///< Guards the maps and the counter.

            /**
             * @brief Generates a new instance ID for a given object name.
             *
             * Returns the lowest non-negative id that is neither registered nor
             * reserved by another object being constructed. Deleted ids are
             * therefore reused, so live objects of one name always occupy the
             * dense range 0..count-1. The id stays reserved until @ref add
             * consumes it, which keeps concurrent constructors from colliding.
             *
             * @param objName The name of the object.
             * @return A unique integer ID for this object instance.
             */
            int generateInstanceId(const QString& objName);

            /**
             * @brief Registers a new object into the registry.
             *
             * Called by `Object` when it is constructed.
             * @param obj Pointer to the object to add.
             */
            void add(Object* obj);

            /**
             * @brief Removes an object from the registry.
             *
             * Called by `Object` when it is destructed.
             * @param obj Pointer to the object to remove.
             */
            void remove(Object* obj);
        };

    } // namespace Core
} // namespace Mira
