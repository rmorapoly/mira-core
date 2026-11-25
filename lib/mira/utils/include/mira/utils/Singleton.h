#pragma once

#include <atomic>
#include <mutex>

namespace Mira::Utils {

    /**
     * @brief A CRTP-based singleton template.
     *
     * The user-defined class should inherit from `Singleton<T>` where `T` is
     * the derived class itself:
     *
     * @code
     * #include "mira/utils/Singleton.h"
     *
     * class Root : public Singleton<Root>
     * {
     *     friend class Singleton<Root>;      // Required!
     * private:
     *     Root() {}                          // Must be private or protected
     *     ...
     * };
     * @endcode
     *
     * ### Features
     * - Lazily constructs the singleton on the first call to `getInstance()`.
     * - Creation is thread-safe: concurrent first calls produce one instance.
     * - Ensures only one instance of the derived type exists.
     *
     * ### Notes
     * - The constructor is protected to prevent direct instantiation.
     * - Destroying the singleton concurrently with `getInstance()` is not
     *   supported; synchronize explicit deletion with all readers.
     * - The instance is intentionally not deleted at exit. Objects with static
     *   storage duration may still use a singleton during their destruction.
     *
     * @tparam T The derived class that should behave as a singleton.
     */
    template <typename T>
    class Singleton
    {
    protected:
        /**
         * @brief Protected constructor to prevent direct instantiation.
         *
         * The derived class should invoke this constructor implicitly.
         */
        explicit Singleton() {}

    public:
        /**
         * @brief Virtual destructor.
         *
         * Clears the internal instance pointer when the singleton object is
         * destroyed, so a later call can create a new one.
         */
        virtual ~Singleton()
        {
            instance.store(nullptr, std::memory_order_release);
        }

        /**
         * @brief Returns the global singleton instance.
         *
         * If no instance exists yet, it is created under @ref creationMutex.
         * Double-checked locking keeps the common path free of locking.
         *
         * @return A pointer to the singleton instance.
         */
        static T* getInstance()
        {
            T* current = instance.load(std::memory_order_acquire);
            if (!current) {
                std::lock_guard<std::mutex> lock(creationMutex);
                current = instance.load(std::memory_order_relaxed);
                if (!current) {
                    current = new T;
                    instance.store(current, std::memory_order_release);
                }
            }
            return current;
        }

    protected:
        static std::atomic<T*> instance;   ///< The single instance, or nullptr.
        static std::mutex creationMutex;   ///< Guards first-time creation.
    };

    template <typename T>
    std::atomic<T*> Singleton<T>::instance{nullptr};

    template <typename T>
    std::mutex Singleton<T>::creationMutex;

} // namespace Mira::Utils
