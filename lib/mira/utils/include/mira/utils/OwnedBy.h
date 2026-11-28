#pragma once

#include <QObject>
#include <QDebug>

namespace Mira::Utils {

    /**
     * @brief Provides access to the owning object.
     *
     * This template is intended to be used as a base class for objects that
     * belong to another object in a hierarchy. The owner is resolved through
     * one of two strategies, in the following order:
     *
     * 1. An explicit owner pointer, either passed to the constructor or
     *    assigned later through setOwner().
     * 2. If no explicit owner has been assigned, the owner is obtained from
     *    the Qt parent of the object. This requires the derived class to also
     *    inherit from QObject (with the parent set to the owner) and the
     *    template parameter TOwner to be a QObject derived type.
     *
     * The ownership relationship is logical rather than memory ownership.
     * Lifetime management is the responsibility of the owning object.
     *
     * Typical usage:
     * @code
     * class Root;
     *
     * class IpCore : public OwnedBy<Root>
     * {
     * public:
     *     using OwnedBy<Root>::OwnedBy;
     * };
     * @endcode
     *
     * Alternative usage where the owner is derived from the Qt parent
     * hierarchy instead of being stored explicitly:
     * @code
     * class Root : public QObject
     * {
     *     Q_OBJECT
     * };
     *
     * class IpCore : public QObject, public OwnedBy<Root>
     * {
     *     Q_OBJECT
     * public:
     *     explicit IpCore(Root* owner) : QObject(owner) {}
     * };
     * @endcode
     *
     * @tparam TOwner Type of the owning object. Must derive from QObject if
     * the owner is expected to be resolved through the Qt parent hierarchy.
     */
    template<typename TOwner>
    class OwnedBy
    {

    public:
        virtual ~OwnedBy() = default;
        /**
         * @brief Constructs an OwnedBy object.
         *
         * @param owner Pointer to the owning object. May be nullptr if the
         * owner will be assigned later or resolved through the Qt parent
         * hierarchy.
         */
        OwnedBy(TOwner* owner = nullptr) :
            m_owner(owner)
        {}

        /**
         * @brief Returns the owning object.
         *
         * The owner is resolved from the explicit owner pointer if one has
         * been assigned. Otherwise it is obtained from the Qt parent of the
         * object. The owner must be valid, otherwise an assertion is
         * triggered in debug builds.
         *
         * @return Reference to the owning object.
         */
        TOwner& owner()
        {
            if (m_owner) {
                Q_ASSERT(m_owner);
                return *m_owner;
            }
            else {
                QObject* obj = dynamic_cast<QObject*>(this);
                Q_ASSERT(obj && obj->parent());
                return *dynamic_cast<TOwner*>(obj->parent());
            }
        }

        /**
         * @brief Returns the owning object.
         *
         * The owner is resolved from the explicit owner pointer if one has
         * been assigned. Otherwise it is obtained from the Qt parent of the
         * object. The owner must be valid, otherwise an assertion is
         * triggered in debug builds.
         *
         * @return Constant reference to the owning object.
         */
        const TOwner& owner() const
        {
            if (m_owner) {
                Q_ASSERT(m_owner);
                return *m_owner;
            }
            else {
                const QObject* obj = dynamic_cast<const QObject*>(this);
                Q_ASSERT(obj && obj->parent());
                return *dynamic_cast<const TOwner*>(obj->parent());
            }
        }

    protected:
        /**
         * @brief Sets the owning object.
         *
         * Intended to be used by the owner or helper classes responsible for
         * constructing and linking the ownership hierarchy. This is not
         * needed if the owner is provided through the Qt parent hierarchy.
         *
         * @param owner Pointer to the owning object.
         */
        void setOwner(TOwner* owner)
        {
            m_owner = owner;
        }

    private:
        TOwner* m_owner;
    };

} // namespace Mira::Utils
