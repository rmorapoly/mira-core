#pragma once

namespace Mira {
    template <typename T>
    class Singleton
    {
    protected:
        explicit Singleton() {}
    public:
        virtual ~Singleton() {
            instance = nullptr;
        }
        static T* getInstance() {
            if (!instance)
                instance = new T;
            return instance;
        }
    protected:
        static T* instance;
    };

    template <typename T>
    T* Singleton<T>::instance = nullptr;
}

