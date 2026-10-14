#pragma once

namespace Mira {

    /**
     * @brief A simple CRTP-based singleton template.
     *
     * This class provides a minimal implementation of the Singleton pattern,
     * intended to be used through the Curiously Recurring Template Pattern (CRTP).
     * The user-defined class should inherit from `Singleton<T>` where `T` is the
     * derived class itself:
     *
     * @code
     * #include "Singleton.h"
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
     * It is optional but prefered to generate instance in cpp:
     *
     * @code
     * static auto instance = Root::getInstance();
     * @endcode
     *
     * ### Features
     * - Lazily constructs the singleton instance upon first call to getInstance() (If not generated in cpp).
     * - Ensures only one instance of the derived type exists.
     *
     * ### Notes
     * - The constructor is protected to prevent direct instantiation.
     * - This implementation does not handle multi-threading. If used in a
     *   multi-threaded environment, external synchronization is required.
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
        * destroyed. This allows recreation after destruction if needed.
        */
        virtual ~Singleton() {
            instance = nullptr;
        }

        /**
        * @brief Returns the global singleton instance.
        *
        * If the instance does not yet exist, it is created lazily using the
        * default constructor of @p T.
        *
        * @return A pointer to the singleton instance.
        */
        static T* getInstance() {
            if (!instance)
                instance = new T;
            return instance;
        }

    protected:
        /**
        * @brief Holds the singleton instance of type @p T.
        *
        * This is shared among all uses of the same template instantiation.
        */
        static T* instance;
    };

    template <typename T>
    T* Singleton<T>::instance = nullptr;

} // namespace Mira
