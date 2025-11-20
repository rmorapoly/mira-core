#pragma once

#include "MiraMessageManager.h"

/**
 * @def MIRA_DECLARE_OBJECT
 * @brief Automatically initializes the base Mira::Object with the demangled type name.
 *
 * This macro is intended to be used when defining a derived class:
 *
 * Example:
 * @code
 * class MyClass : public Mira::Object {
 * public:
 *     MyClass() MIRA_DECLARE_OBJECT {}
 * };
 * @endcode
 *
 * `MIRA_DECLARE_OBJECT` expands to:
 * @code
 * : Mira::Object("MyClass")
 * @endcode
 *
 * Meaning the object will automatically use its own C++ class name.
 */
#define MIRA_DECLARE_OBJECT \
    : Mira::Object(Mira::demangleClassName(typeid (this).name()))

namespace Mira {

    /**
     * @class Object
     * @brief Base class for all Mira mdoule objects.
     *
     * Mira::Object extends QObject and provides:
     *  - A stable `name` identifying the object type.
     *  - An optional `instanceName` (user-provided name).
     *  - A unique `instanceId` generated per type family.
     *  - Easy message logging via `makeMessageHelper()` which is used as macros in MiraMessageManager.
     *
     * All objects that want Mira's logging, introspection, and registry features
     * should inherit from this class.
     */
    class MIRA_CORE_EXPORT Object : public QObject
    {
        Q_OBJECT

    public:
        /**
         * @brief Constructs a Mira::Object and emits a "constructed" message.
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
         * Emits a "destructed" message and unregisters the object from ObjectRegistry.
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
         * This function returns `true` only if the object's @var name is:
         *  - not equal to the automatically assigned class name (`__CLASS_NAME__`), and
         *  - not an empty string.
         *
         * In other words, it indicates whether the user explicitly provided a name
         * different from the default class name.
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

    protected:
        /**
         * @brief Creates a message helper bound to this object.
         *
         * Called internally by miraInfo(), miraDebug(), etc.
         */
        MessageHelper makeMessageHelper(QtMsgType type) const;
    };

} // namespace Mira
