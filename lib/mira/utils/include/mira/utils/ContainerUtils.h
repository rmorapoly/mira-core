#pragma once

#include <QList>
#include <QMap>
#include <QVariant>
#include <QVector>
#include <QSet>

/**
 * @brief Utility functions for converting Qt containers and values between compatible types.
 */
namespace Mira::Utils
{
	/**
	 * @brief Converts the elements of a QList to another type.
	 *
	 * Each element is converted using `static_cast`.
	 *
	 * @tparam TCastType Destination element type.
	 * @tparam TOriginType Source element type.
	 *
	 * @param list Source list.
	 *
	 * @return A QList containing the converted elements.
	 */
	template<typename TCastType, typename TOriginType>
	static QList<TCastType> castTo(const QList<TOriginType>& list)
	{
		QList<TCastType> _list;
		for (const auto& item : qAsConst(list))
		{
			_list.append(static_cast<TCastType>(item));
		}
		return _list;
	}

	/**
	 * @brief Converts the elements of a QVector to another type.
	 *
	 * Each element is converted using `static_cast`.
	 *
	 * @tparam TCastType Destination element type.
	 * @tparam TOriginType Source element type.
	 *
	 * @param vector Source vector.
	 *
	 * @return A QVector containing the converted elements.
	 */
	template<typename TCastType, typename TOriginType>
	static QVector<TCastType> castTo(const QVector<TOriginType>& vector)
	{
		QVector<TCastType> _vector;
		for (const auto& item : qAsConst(vector))
		{
			_vector.append(static_cast<TCastType>(item));
		}
		return _vector;
	}

	/**
	 * @brief Converts the elements of a QSet to another type.
	 *
	 * Each element is converted using `static_cast`.
	 *
	 * @tparam TCastType Destination element type.
	 * @tparam TOriginType Source element type.
	 *
	 * @param set Source set.
	 *
	 * @return A QVector containing the converted elements.
	 */
	template<typename TCastType, typename TOriginType>
	static QSet<TCastType> castTo(const QSet<TOriginType>& set)
	{
		 QSet<TCastType> _set;
		 for (const auto& item : qAsConst(set))
		 {
			  _set.insert(static_cast<TCastType>(item));
		 }
		 return _set;
	}

	/**
	 * @brief Converts a QList of QVariant to a typed QList.
	 *
	 * Each QVariant is converted using QVariant::value().
	 *
	 * @tparam TCastType Destination element type.
	 *
	 * @param list Source QVariant list.
	 *
	 * @return A QList containing the converted elements.
	 */
	template<typename TCastType>
	static QList<TCastType> castTo(const QList<QVariant>& list)
	{
		QList<TCastType> _list;
		for (const auto& item : qAsConst(list))
		{
			_list.append(item.value<TCastType>());
		}
		return _list;
	}

	/**
	 * @brief Converts a typed QList to a QList of QVariant.
	 *
	 * Each element is wrapped using QVariant::fromValue().
	 *
	 * @tparam TOriginType Source element type.
	 *
	 * @param list Source list.
	 *
	 * @return A QList of QVariant.
	 */
	template<typename TOriginType>
	static QList<QVariant> castTo(const QList<TOriginType>& list)
	{
		QList<QVariant> _list;
		for (const auto& item : qAsConst(list))
		{
			_list.append(QVariant::fromValue(item));
		}
		return _list;
	}

	/**
	 * @brief Converts a typed QVector to a QVector of QVariant.
	 *
	 * Each element is wrapped using QVariant::fromValue().
	 *
	 * @tparam TOriginType Source element type.
	 *
	 * @param vector Source vector.
	 *
	 * @return A QVector of QVariant.
	 */
	template<typename TOriginType>
	static QVector<QVariant> castTo(const QVector<TOriginType>& vector)
	{
		QVector<QVariant> _vector;
		for (const auto& item : qAsConst(vector))
		{
			_vector.append(QVariant::fromValue(item));
		}
		return _vector;
	}

	/**
	 * @brief Converts the keys and values of a QMap to different types.
	 *
	 * Keys and values are converted using `static_cast`.
	 *
	 * @tparam TKey Destination key type.
	 * @tparam TValue Destination value type.
	 * @tparam TOriginKey Source key type.
	 * @tparam TOriginValue Source value type.
	 *
	 * @param map Source map.
	 *
	 * @return A QMap containing the converted keys and values.
	 */
	template<typename TKey, typename TValue, typename TOriginKey, typename TOriginValue>
	static QMap<TKey, TValue> castTo(const QMap<TOriginKey, TOriginValue>& map)
	{
		QMap<TKey, TValue> _map;
		auto qIt = QMapIterator(map);
		while (qIt.hasNext())
		{
			qIt.next();
			_map.insert(static_cast<TKey>(qIt.key()), static_cast<TValue>(qIt.value()));
		}
		return _map;
	}
}
