#pragma once

#include <QObject>
#include "mira/core/Import.h"
#include "mira/utils/Singleton.h"
#include "ObjectRegistry.h"

namespace Mira {
    namespace Core {

        /**
         * @class Root
         * @brief Central singleton entry point of the Mira Core module.
         *
         * `Root` lives in `Mira::Core` and exposes the services provided by
         * the Mira Core components — currently the `ObjectRegistry`.
         *
         * This class inherits from `Mira::Utils::Singleton<Root>` and should be accessed via:
         * @code
         * Mira::Core::Root* core = Mira::Core::Root::getInstance();
         * @endcode
         *
         * ### Example Usage:
         * @code
         * auto& core = *Mira::Core::Root::getInstance();
         * int count = core.registry().count("MyObject");
         * @endcode
         */
        class MIRA_CORE_EXPORT Root : public QObject, public Mira::Utils::Singleton<Root>
        {
            Q_OBJECT
            friend class Mira::Utils::Singleton<Root>;

            /**
             * @brief Constructs the Root instance.
             * @param parent Optional QObject parent.
             *
             * Note: Only accessible to Mira::Utils::Singleton<Root>.
             */
            explicit Root(QObject* parent = nullptr);

        public:
            /**
             * @brief Returns the global ObjectRegistry.
             * @return The object registry instance.
             */
            ObjectRegistry& registry();

        private:
            ObjectRegistry objectRegistry; ///< Tracks all Mira::Core::Object instances.
        };

    } // namespace Core
} // namespace Mira
