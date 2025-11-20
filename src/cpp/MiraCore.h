#pragma once

#include <QObject>
#include <QMap>
#include "MiraSingleton.h"
#include "MiraMessageManager.h"
#include "MiraObjectRegistry.h"

namespace Mira {

    /**
     * @class Core
     * @brief Central singleton class that provides access to Mira's core services.
     *
     * `Core` manages:
     * - The global `MessageManager` for logging and message routing.
     * - The `ObjectRegistry` for object lifetime and instance tracking.
     * - Mapping of object names to instance IDs.
     *
     * This class inherits from `Singleton<Core>` and should be accessed via:
     * @code
     * Mira::Core* core = Mira::Core::getInstance();
     * @endcode
     *
     * ### Example Usage:
     * @code
     * auto& core = *Mira::Core::getInstance();
     * core.getMessageManager().report({"Hello", QtInfoMsg});
     * int count = core.getObjectRegistery().count("MyObject");
     * @endcode
     */
    class MIRA_CORE_EXPORT Core : public QObject, public Singleton<Core>
    {
        Q_OBJECT
        friend class Singleton<Core>;

        /**
         * @brief Constructs the Core instance.
         * @param parent Optional QObject parent.
         *
         * Note: Only accessible to Singleton<Core>.
         */
        explicit Core(QObject *parent = nullptr);

    public:
        /**
         * @brief Returns a constant reference to the global MessageManager.
         * @return The message manager instance.
         */
        const MessageManager& getMessageManager() const;

        /**
         * @brief Returns a reference to the global ObjectRegistry.
         * @return The object registry instance.
         */
        ObjectRegistry& getObjectRegistery();

    private:
        const MessageManager messageManager;    ///< Handles routing and reporting of messages.
        ObjectRegistry objectRegistery;         ///< Tracks all Mira::Object instances.
        QMap<QString, int> objNameToInstanceId; ///< Maps object names to unique instance IDs.
    };

} // namespace Mira
