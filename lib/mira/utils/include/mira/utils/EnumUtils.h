#pragma once

#include <QMetaEnum>
#include <QMetaObject>
#include <QMetaType>
#include <QString>

/**
 * @brief Utility functions for working with Qt-registered enumeration types.
 */
namespace Mira::Utils
{
	/**
	 * @brief Gets the name of an enumeration type.
	 *
	 * Returns the name of the Qt-registered enumeration type.
	 *
	 * @tparam T Enumeration type registered with Qt using
	 *         Q_ENUM or Q_ENUM_NS.
	 *
	 * @return The enumeration type name.
	 *
	 * @pre The enumeration must be registered with Qt.
	 */
	template<typename T>
	static inline QString getEnumName()
	{
		static auto meta = QMetaEnum::fromType<T>();
		Q_ASSERT(meta.isValid());
		return meta.name();
	}

	/**
	 * @brief Gets the key corresponding to an enumeration value.
	 *
	 * Converts an enumeration value to its string representation.
	 *
	 * @tparam T Enumeration type registered with Qt using
	 *         Q_ENUM or Q_ENUM_NS.
	 * @param value Enumeration value.
	 *
	 * @return Enumeration key, or an empty string if the value is invalid.
	 *
	 * @pre The enumeration must be registered with Qt.
	 */
	template<typename T>
	static inline QString getEnumKey(T value)
	{
		static auto meta = QMetaEnum::fromType<T>();
		Q_ASSERT(meta.isValid());
		return meta.valueToKey(static_cast<int>(value));
	}

	/**
	 * @brief Gets the enumeration value corresponding to a key.
	 *
	 * Converts a string key to its enumeration value.
	 *
	 * @tparam T Enumeration type registered with Qt using
	 *         Q_ENUM or Q_ENUM_NS.
	 * @param key Enumeration key.
	 * @param ok Optional pointer set to `true` if the conversion succeeds;
	 *           otherwise set to `false`.
	 *
	 * @return Corresponding enumeration value. If the conversion fails,
	 *         the returned value is implementation-defined (typically -1
	 *         cast to the enumeration type).
	 *
	 * @pre The enumeration must be registered with Qt.
	 */
	template<typename T>
	static inline T getEnumValue(const char* key, bool* ok = nullptr)
	{
		static auto meta = QMetaEnum::fromType<T>();
		Q_ASSERT(meta.isValid());
		return static_cast<T>(meta.keyToValue(key, ok));
	}
}
