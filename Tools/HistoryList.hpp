/**
 * @file        HistoryList.hpp
 * @author      CodeDog
 * @brief       A collection of a fixed amount of history elements
 *              that can be added, removed and iterated from the most recent one.
 *              Setting the page size allows the standard iterator to iterate over a subset of elements.
 *
 * @copyright   (c)2022 CodeDog, All rights reserved.
 */

#pragma once

#include <stddef.h>
#include "IndexIterator.hpp"

/**
 * @class HistoryList
 * @brief A history (stack-like) list providing IndexIterator.
 * @tparam T Element type.
 * @tparam N Capacity.
 */
template<typename T, int N>
class HistoryList
{

public: // Type aliases:
    using ValueType = T;
    using Iterator = IndexIterator<HistoryList<T, N>>;

public: // Static:
    static const unsigned int capacity = N; ///< Fixed list capacity.

public: // API:
    /**
     * @fn HistoryList()
     * @brief Creates a new history list.
     */
    HistoryList() : m_length(0), m_offset(-1), m_elements() { }

    /**
     * @fn HistoryList(const HistoryList*)
     * @brief Deletes the copy constructor.
     * @param other
     */
    HistoryList(const HistoryList&) = delete;

    /**
     * @fn unsigned int length()const
     * @brief Gets the current length of the list.
     * @return
     */
    unsigned int length() const
    {
        return m_length;
    }

    /**
     * @fn void reset()
     * @brief Resets the history list.
     */
    void reset()
    {
        m_length = 0;
        m_offset = -1;
        trim();
    }

    /**
     * @fn      T& get()
     * @brief   Gets the new history element's reference.
     * @return  A reference to the new element.
     */
    T& add()
    {
        ++m_length;
        if (m_length > capacity) m_length = capacity;
        m_offset = (m_offset + 1) % capacity;
        return m_elements[m_offset];
    }

    /**
     * @fn      bool any()
     * @brief   Returns true if there are any elements on the list.
     * @return  True if any elements on the list.
     */
    bool any() const
    {
        return m_length > 0;
    }

    /**
     * @fn      T& back()
     * @brief   Takes the last added element from the list and returns the reference to it.
     *          If the list was empty, returns the first empty element.
     * @return  The previously added element or empty (first) element's reference.
     */
    T& back()
    {
        if (m_length > 1)
        {
            new (m_elements + m_offset) T();
            --m_length;
            m_offset = m_offset > 0 ? m_offset - 1 : capacity - 1;
        }
        return current();
    }

    /**
     * @fn      T& current()
     * @brief   Gets the last added element's reference.
     *          If the list was empty, returns the first empty element.
     * @return  Last added element or empty element's reference.
     */
    T& current() const
    {
        return m_offset >= 0 ? m_elements[m_offset] : m_elements[0];
    }

    /**
     * @fn       T& at(int)
     * @brief    Gets the reference to the element at the specified history level.
     * @param    level A history level, 0 is the most recent (current).
     * @return   A reference to the element at specified level.
     */
    T& operator[](int level)
    {
        return m_elements[(m_offset - level) % capacity];
    }

public: // Iterator API:

    /**
     * @fn      iterator begin()
     * @brief   Gets the iterator for the first (most recently added) item.
     * @return  iterator.
     */
    Iterator begin()
    {
        return Iterator(this, 0);
    }

    /**
     * @fn      iterator end()
     * @brief   Gets the iterator for the one over the last (non existing) item.
     * @return  iterator.
     */
    Iterator end()
    {
        return Iterator(this, m_length);
    }

protected: // Internal API (for storage implementation):

    /**
     * @fn      void trim()
     * @brief   Trims the unset elements by calling the default constructor on them.
     */
    void trim()
    {
        for (unsigned int i = m_length; i < capacity; i++) m_elements[i] = { };
    }

    /**
     * @fn      int getElementsOffset()
     * @brief   Gets the byte offset of the elements data.
     * @returns Elements' byte offset.
     */
    int getElementsOffset() const
    {
        return (int) ((char*) &m_elements - (char*) this);
    }

    /**
     * @fn      int getElementsSize()
     * @brief   Gets the size of the non-empty elements in bytes.
     * @returns Size.
     */
    size_t getElementsSize() const
    {
        return m_length * sizeof(T);
    }

    /**
     * @fn      size_t getDataSize()
     * @brief   Gets the current data size.
     * @returns Size.
     */
    size_t getDataSize() const
    {
        return getElementsOffset() + getElementsSize();
    }

    /**
     * @fn      size_t getMaxDataSize()
     * @brief   Gets the full data size.
     * @returns Size.
     */
    size_t getMaxDataSize() const
    {
        return getElementsOffset() + capacity * sizeof(T);
    }

private: // Data:
    unsigned int m_length; ///< List length.
    int m_offset; ///< Current element offset.
    T m_elements[N]; ///< Elements array.

};
