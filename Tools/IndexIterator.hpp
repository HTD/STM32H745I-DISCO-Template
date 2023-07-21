/**
 * @file        IndexIterator.hpp
 * @author      CodeDog
 * @brief       Iterator for indexable collections.
 *
 * @copyright   (c)2023 CodeDog, All rights reserved.
 */

#pragma once

#include <cstddef>

/**
 * @brief Iterates over indexable collection.
 *
 * @tparam TIndexable The indexable collection type to iterate over.
 * @tparam TIndex Index type.
 */
template<class TIndexable, typename TIndex = size_t>
class IndexIterator
{
public: // Type aliases:
    using ValueType = typename TIndexable::ValueType;
    using ReferenceType = ValueType&;
    using PointerType = ValueType*;

public: // API:

    /**
     * @brief Creates the iterator for the specified collection and index.
     *
     * @param collection Collection pointer.
     * @param index Element index.
     */
    IndexIterator(TIndexable* collection, TIndex index) : m_collection(collection), m_index(index) { }

    /**
     * @brief Pre-increments the iterator.
     *
     * @returns Iterator.
     */
    IndexIterator& operator++()
    {
        m_index++;
        return *this;
    }

    /**
     * @brief Post-increments the iterator.
     *
     * @returns Iterator before.
     */
    IndexIterator operator++(int)
    {
        IndexIterator iterator = *this;
        ++(*this);
        return iterator;
    }

    /**
     * @brief Pre-decrements the iterator.
     *
     * @returns Iterator.
     */
    IndexIterator& operator--()
    {
        m_index--;
        return *this;
    }

    /**
     * @brief Post-decrements the iterator.
     *
     * @returns Iterator before.
     */
    IndexIterator operator--(int)
    {
        IndexIterator iterator = *this;
        --(*this);
        return iterator;
    }

    /**
     * @fn ReferenceType operator *()
     * @brief Returns the element's reference.
     * @returns Element's reference.
     */
    ReferenceType operator*()
    {
        return (*m_collection)[m_index];
    }

    /**
     * @fn PointerType operator ->()
     * @brief Returns the element's pointer.
     * @returns Element's pointer.
     */
    PointerType operator->()
    {
        return &(*m_collection)[m_index];
    }

    /**
     * @fn bool operator ==(const IndexIterator&)const
     * @brief Equality operator.
     * @param other The other instance.
     * @returns True if the instances are equal.
     */
    bool operator==(const IndexIterator& other) const
    {
        return m_collection == other.m_collection && m_index == other.m_index;
    }

    /**
     * @fn bool operator !=(const IndexIterator&)const
     * @brief Inequality operator.
     * @param other The other instance.
     * @returns True if the instances are not equal.
     */
    bool operator!=(const IndexIterator& other) const
    {
        return m_collection != other.m_collection || m_index != other.m_index;
    }

    /**
     * @fn unsigned int index()
     * @brief Returns the index of the iterator.
     * @returns Index.
     */
    unsigned int index() const
    {
        return m_index;
    }

private:

    TIndexable* m_collection;   // TIndexable m_collection pointer.
    TIndex m_index;             // Current iterator index.

};
