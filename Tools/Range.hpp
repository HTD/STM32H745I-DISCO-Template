/**
 * @file        Range.hpp
 * @author      CodeDog
 * @brief       Universal floating points numbers iterator.
 *
 * @copyright	(c)2023 CodeDog, All rights reserved.
 */

#pragma once
#include <cmath>
#include <limits>

 /**
  * @brief Iterates between a range of numbers.
  */
class RangeIterator
{
public:

    /**
     * @brief Creates the number iterator instance.
     *
     * @param start A value that starts the sequence.
     * @param end A value that ends the sequence.
     * @param numValues The number of values in the sequence.
     */
    RangeIterator(double start, double end, size_t numValues) :
        m_start(start),
        m_end(end),
        m_numValues(numValues),
        m_step((end - start) / static_cast<double>(numValues - 1ul)),
        m_current(numValues > 1 ? start : end),
        m_currentIndex(0) { }

    /**
     * @brief Creates a zero iterator that doesn't produce anything.
     */
    RangeIterator() :
        m_start(0),
        m_end(0),
        m_numValues(0),
        m_step(0),
        m_current(0),
        m_currentIndex(0) { }

    /**
     * @brief Gets the current index of the iterator.
     *
     * @returns Current index value.
     */
    size_t index() { return m_currentIndex; }

    RangeIterator& operator++()
    {
        m_current = (m_currentIndex < m_numValues - 1)
            ? m_start + m_step * ++m_currentIndex
            : m_end;
        return *this;
    }

    RangeIterator operator++(int)
    {
        RangeIterator temp = *this;
        ++(*this);
        return temp;
    }

    bool operator==(const RangeIterator& other) const
    {
        return
            std::fabs(m_current - other.m_current) <= std::numeric_limits<double>::epsilon() * std::fabs(m_current);
    }

    bool operator!=(const RangeIterator& other) const
    {
        return !(*this == other);
    }

    double operator*() const
    {
        return m_current;
    }

private:
    double m_start;
    double m_end;
    size_t m_numValues;
    double m_step;
    double m_current;
    size_t m_currentIndex;

};

/**
 * @brief Generates a sequence of `numValues` equally distributed values from `start` to `end` inclusive.
 */
class Range
{
public:

    /**
     * @brief Generates a sequence of `numValues` equally distributed values from `start` to `end` inclusive.
     *
     * @param start First generated value.
     * @param end Last generated value.
     * @param numValues Number of generated values.
     */
    Range(double start, double end, size_t numValues) :
        m_start(start),
        m_step((end - start) / static_cast<double>(numValues - 1ul)),
        m_end(end),
        m_numValues(numValues) { }

    /**
     * @brief Returns the first term iterator.
     *
     * @returns The first term iterator.
     */
    RangeIterator begin() const
    {
        return RangeIterator(m_start, m_end + m_step, m_numValues + 1);
    }

    /**
     * @brief Returns the "last + 1" term iterator.
     *
     * @returns  The "last + 1" term iterator.
     */
    RangeIterator end() const
    {
        return (m_numValues > 0)
            ? RangeIterator(m_end + m_step, m_end + m_step, m_numValues + 1)
            : begin();
    }

private:
    double m_start;     // First value.
    double m_step;      // Increment step.
    double m_end;       // Last value.
    size_t m_numValues; // Number of values.

};
