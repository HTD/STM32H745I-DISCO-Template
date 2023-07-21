/**
 * @file        StaticTextBuffer.hpp
 * @author      CodeDog
 * @brief       Provides statically allocated fixed size text buffer.
 * @remarks     Use if required buffer size is known at compile time / for stack allocations.
 *
 * @copyright   (c)2022 CodeDog, All rights reserved.
 */

#pragma once

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <array>

/**
 * @brief Statically allocated buffer.
 * @tparam TSize Fixed size in bytes.
 *
 * @remarks Copyable, not moveable.
 */
template<size_t TSize>
class StaticTextBuffer final
{
public:

    /**
     * @brief Creates new fixed size text buffer.
     */
    StaticTextBuffer() : m_data(), m_length(0) { };

    StaticTextBuffer(const StaticTextBuffer& other) : m_data(), m_length(other.m_length)
    {
        for (size_t i = 0; i < other.m_length; i++)
            m_data[i] = other.m_data[i];
    }

    StaticTextBuffer(const StaticTextBuffer&&) = delete;

    StaticTextBuffer& operator=(const StaticTextBuffer& other)
    {
        if (other == *this) return *this;
        clear();
        m_length = other.m_length;
        for (size_t i = 0; i < other.m_length; i++)
            m_data[i] = other.m_data[i];
    }

    /// @returns Character at the specified index reference.
    inline char& at(int index)
    {
        return m_data[index];
    }

    /// @returns The pointer to a null-terminated character array.
    inline const char* c_str() const
    {
        return const_cast<char*>(m_data.data());
    }

    /// @brief @returns The pointer to a null-terminated `T` array.
    template<typename T> inline T* as() const
    {
        return static_cast<T*>((void*) m_data.data());
    }

    /// @brief Clears the buffer by zeroing all of its memory.
    inline void clear()
    {
        m_length = 0;
        memset(&m_data, 0, TSize);
    }

    /// @returns The size of the buffer in bytes.
    inline size_t size() const
    {
        return TSize;
    }

    /// @returns The C-string length without the null terminator.
    inline size_t length() const
    {
        return m_length;
    }

    /// @returns Character at the specified index reference.
    inline char& operator[](int index)
    {
        return m_data[index];
    }

    /**
     * @brief Appends formatted string. Overflowing text will be discarded.
     *
     * @tparam va Variadic argumets.
     * @param format String format.
     * @param args Optional arguments.
     */
    template<class ...va> void printf(const char* format, va ...args)
    {
        if (m_length < TSize) m_length+= snprintf(&(m_data[m_length]), TSize - m_length, format, args...);
    }

    /// @brief Appends a byte character.
    inline void poke(char value)
    {
        if (m_length < TSize) m_data[m_length++] = value;
    }

    /// @brief Appends 2 byte characters.
    inline void poke(char v1, char v2)
    {
        if (m_length < TSize) m_data[m_length++] = v1;
        if (m_length < TSize) m_data[m_length++] = v2;
    }

    /// @brief Appends 3 byte characters.
    inline void poke(char v1, char v2, char v3)
    {
        if (m_length < TSize) m_data[m_length++] = v1;
        if (m_length < TSize) m_data[m_length++] = v2;
        if (m_length < TSize) m_data[m_length++] = v3;
    }

    /// @brief Appends 4 byte characters.
    inline void poke(char v1, char v2, char v3, char v4)
    {
        if (m_length < TSize) m_data[m_length++] = v1;
        if (m_length < TSize) m_data[m_length++] = v2;
        if (m_length < TSize) m_data[m_length++] = v3;
        if (m_length < TSize) m_data[m_length++] = v4;
    }

    /// @brief Appends 5 byte characters.
    inline void poke(char v1, char v2, char v3, char v4, char v5)
    {
        if (m_length < m_size) m_data[m_length++] = v1;
        if (m_length < m_size) m_data[m_length++] = v2;
        if (m_length < m_size) m_data[m_length++] = v3;
        if (m_length < m_size) m_data[m_length++] = v4;
        if (m_length < m_size) m_data[m_length++] = v5;
    }

private:
    std::array<char, TSize> m_data; // Data array.
    size_t m_length;
};
