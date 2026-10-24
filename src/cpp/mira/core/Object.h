#pragma once

#include <QObject>
#include <type_traits>
#include "MiraCoreImport.h"
#include "mira/Meta.h"
#include "mira/core/Log.h"

/**
 * @def MIRA_DECLARE_OBJECT
 * @brief Automatically initializes the base `Mira::Core::Object` with the
 * derived class name.
 *
 * The name is extracted at compile time from the type currently being
 * constructed (`decltype(this)`), without RTTI or the platform demangler, so
 * a class `Child : public Mira::Core::Object` is named exactly `Child`.
 *
 * Example:
 * @code
 * class MyClass : public Mira::Core::Object {
 * public:
 *     MyClass() MIRA_DECLARE_OBJECT {}
 * };
 * @endcode
 *
 * `MIRA_DECLARE_OBJECT` expands to:
 * @code
 * : Mira::Core::Object(Mira::className<std::remove_pointer_t<decltype(this)>>())
 * @endcode
 */
#define MIRA_DECLARE_OBJECT \
    : Mira::Core::Object(Mira::className<std::remove_pointer_t<decltype(this)>>())

namespace Mira {
    namespace Core {

        /**
         * @class Object
         * @brief Base class for all Mira module objects.
         *
         * `Mira::Core::Object` extends `QObject` and provides:
         *  - A stable `name` identifying the object type.
         *  - An optional `instanceName` (user-provided name).
         *  - A unique `instanceId` generated per type family.
         *  - Public fields (`name`, `instanceId`, `instanceName`) consumed by the
         *    Mira log handler to annotate messages emitted through the `miraInfo()`
         *    family of macros.
         *  - An object scope: while the object lives it is the object in scope, so
         *    every `miraInfo()`-family message logged in that time is attributed to
         *    it, wherever it is logged from.
         *
         * All objects that want Mira's logging, introspection, and registry features
         * should inherit from this class.
         */
        class MIRA_CORE_EXPORT Object : public QObject
        {
            Q_OBJECT

        public:
            /**
             * @brief Constructs a `Mira::Core::Object` and registers it.
             *
             * @param name          The base object name (usually auto-generated using MIRA_DECLARE_OBJECT).
             * @param instanceName  Optional instance label to distinguish multiple objects of the same type.
             * @param parent        Standard QObject parent.
             */
            explicit Object(const QString& name = "",
                            const QString& instanceName = "",
                            QObject* parent = nullptr);

            /**
             * @brief Virtual destructor.
             *
             * Unregisters the object from the `ObjectRegistry`.
             */
            virtual ~Object();

            /** @brief The object type name (e.g., "MyClass"). */
            const QString name;

            /** @brief Optional per-instance name for user identification. */
            const QString instanceName;

            /** @brief Unique incremental ID assigned for each type family. */
            const int instanceId;

            /**
             * @brief Checks whether the object has a meaningful custom name.
             *
             * Returns `true` only if @ref name differs from the automatically
             * assigned class name and is not empty.
             *
             * @return `true` if the object has a user-defined name, otherwise `false`.
             */
            bool hasName() const;

            /**
             * @brief Whether the object has a user-defined instanceName set.
             */
            bool hasInstanceName() const;

            /**
             * @brief Equality operator, comparing name + instanceName + instanceId.
             */
            friend bool operator==(const Object& obj1, const Object& obj2);

            /**
             * @brief Inequality operator.
             */
            friend bool operator!=(const Object& obj1, const Object& obj2);
        };

    } // namespace Core
} // namespace Mira
