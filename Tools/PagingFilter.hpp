/**
 * @file        PageingFilter.hpp
 * @author      CodeDog
 * @brief       Pageing filter for indexable collections.
 *
 * @copyright   (c)2022 CodeDog, All rights reserved.
 */

#pragma once

#include "IndexIterator.hpp"

/**
 * @class PagingFilter
 * @brief A filter providing index iterator over indexable collection that is divided into pages.
 * @tparam TIndexable The indexable collection type to iterate over.
 */
template<class TIndexable>
class PagingFilter
{
public: // Aliases:
    using Iterator = IndexIterator<TIndexable>;

public: // Constructors:
    /**
     * @fn  PagingFilter(TIndexable&, unsigned int)
     * @brief Creates a paging filter over an indexable collection.
     * @param m_collection Indexable collection reference.
     * @param pageSize The number of elements per page.
     */
    PagingFilter(TIndexable* collection, unsigned int pageSize)
            : page(0), m_collection(collection), m_pageSize(pageSize) { }

    PagingFilter(PagingFilter&) = delete; ///< Deletes the copy constructor.

public: // Iterator API:

    /**
     * @fn Iterator begin()
     * @brief Gets the iterator for the first enumerated item.
     * @return Iterator.
     */
    Iterator begin()
    {
        unsigned int limit = m_collection->end().index();
        unsigned int pages = limit / m_pageSize + 1;
        unsigned int pageIndex = ((page % pages) * m_pageSize) % limit;
        return Iterator(m_collection, pageIndex);
    }

    /**
     * @fn Iterator end()
     * @brief Gets the iterator for the first enumerated item that is off limit.
     * @return Iterator.
     */
    Iterator end()
    {
        unsigned int limit = m_collection->end().index();
        unsigned int pages = limit / m_pageSize + 1;
        unsigned int itemsLeft = limit - (((page % pages) * m_pageSize) % limit);
        unsigned int offLimitIndex = ((page % pages) * m_pageSize) % limit + (itemsLeft >= m_pageSize ? m_pageSize : itemsLeft);
        return Iterator(m_collection, offLimitIndex);
    }

    /**
     * @fn unsigned int pages()
     * @brief Gets the number of pages depending on the collection length and elements per page.
     * @return Number of pages.
     */
    unsigned int pages() const
    {
        return m_collection->end().index() / m_pageSize + 1;
    }

    unsigned int page; ///< Current page index for iterator begin() and end() function.

private:
    TIndexable* m_collection; ///< The original m_collection.
    unsigned int m_pageSize; ///< The number of elements per page. Zero disables paging iterator behavior.
};
