/**
 * @file        DataSetT.hpp
 * @author      CodeDog
 * @brief       An indexable and iterable, fixed size data set class template.
 *
 * @copyright	(c)2023 CodeDog, All rights reserved.
 */

#pragma once

#include "IndexIterator.hpp"

/**
 * @brief An indexable and iterable, fixed size data set class template.
 */
template<typename TPoint, int Capacity>
class DataSetT
{

private:
    int m_lastIndex = -1;
    TPoint m_points[Capacity]{};

public:
    using ValueType = TPoint;
    using Iterator = IndexIterator<DataSetT, int>;

    /**
     * @brief Tests if both data sets point to the same data array and have the same last index.
     *
     * @param other Other data set reference.
     * @returns 1: The same data set. 0: Different data sets.
     */
    bool operator==(DataSetT& other)
    {
        return m_points == other.m_points && m_lastIndex == other.m_lastIndex;
    }

    /**
     * @brief Tests if the data sets point to different data arrays or have different last indexes.
     *
     * @param other Other data set reference.
     * @returns 1: The data sources are different. 0: The data sources are the same.
     */
    bool operator!=(DataSetT& other)
    {
        return m_points != other.m_points || m_lastIndex != other.m_lastIndex;
    }

    /**
     * @brief Gets the point reference at specified index.
     *
     * @param index Point index.
     * @returns Point reference.
     */
    TPoint& operator[](int index)
    {
        return m_points[index];
    }

    static int capacity()
    {
        return Capacity;
    }

    /**
     * @brief Returns the length of the calibration point array.
     *
     * @returns Length of the calibration point array.
     */
    int length() const
    {
        return m_lastIndex + 1;
    }

    /**
     * @brief Returns the first point reference.
     *
     * @returns First point reference.
     */
    TPoint& first()
    {
        return m_points[0];
    }

    /**
     * @brief Returns the last point reference.
     *
     * @returns Last point reference.
     */
    TPoint& last()
    {
        return m_points[m_lastIndex];
    }

    /**
     * @brief Provides iterator for the first point.
     *
     * @returns First point.
     */
    Iterator begin()
    {
        return Iterator(this, 0);
    }

    /**
     * @brief Provides iterator for the one over last point.
     *
     * @returns One over last point.
     */
    Iterator end()
    {
        return Iterator(this, m_lastIndex + 1);
    }

    /**
     * @returns A value indicating that the collection is empty.
     */
    bool empty() const
    {
        return m_lastIndex < 0;
    }

    /**
     * @brief Adds a data point to the collection.
     *
     * @param point A data point.
     */
    virtual void add(TPoint point)
    {
        if (m_lastIndex >= Capacity - 1) return;
        ++m_lastIndex;
        m_points[m_lastIndex] = point;
    }

    /**
     * @brief Copies all the data points from another data set of the same type, zeroes the unused points.
     *
     * @param other Other data set reference.
     */
    virtual void copyFrom(DataSetT& other)
    {
        for (int i = 0; i <= other.m_lastIndex; i++) m_points[i] = other.m_points[i];
        m_lastIndex = other.m_lastIndex;
        for (int i = m_lastIndex + 1; i < Capacity; i++) m_points[i] = 0;
    }

    /**
     * @brief Copies all the data points to the other data set of the same type, zeroes the unused points of the other set.
     *
     * @param other Other data set reference.
     */
    virtual void copyTo(DataSetT& other)
    {
        for (int i = 0; i <= m_lastIndex; i++) other.m_points[i] = m_points[i];
        other.m_lastIndex = m_lastIndex;
        for (int i = other.m_lastIndex + 1; i < Capacity; i++) other.m_points[i] = 0;
    }

    /**
     * @brief Resets the collection to the empty state.
     */
    virtual void zero()
    {
        m_lastIndex = -1;
        for (int i = 0; i < Capacity; i++) m_points[i] = 0;
    }

    /**
     * @brief Tests if the other data set contains the same data.
     *
     * @param other Other data source reference.
     * @returns 1: Both data sources contain the same data. 0: The other data source contains different data.
     */
    bool contentEqual(DataSetT& other)
    {
        if (*this == other) return true;
        if (m_lastIndex != other.m_lastIndex) return false;
        for (int i = 0; i <= m_lastIndex; i++) if (m_points[i] != other.m_points[i]) return false;
        return true;
    }

};
