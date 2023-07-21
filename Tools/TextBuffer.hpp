/**
 * @file        TextBuffer.hpp
 * @author      CodeDog
 * @brief       Provides dynamically heap allocated text buffer.
 * @remarks     Use if required buffer size is calculated at runtime.
 *
 * @copyright   (c)2023 CodeDog, All rights reserved.
 */

#pragma once

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <array>
#include <memory>
#include "ThreadSafeAllocator.hpp"

/**
 * @brief Heap allocated text buffer.
 *
 * @remarks Moveable, not copyable.
 */
class TextBuffer final : public ThreadSafeAllocator
{
public:

    /**
     * @brief Dynamically allocates buffer on the heap.
     *
     * @param size Buffer size in bytes.
     */
    TextBuffer(size_t size) : m_data(std::make_unique<char[]>(size)), m_size(size), m_length(0) { };

    TextBuffer(const TextBuffer&) = delete;

    TextBuffer(TextBuffer&& other) noexcept : m_data(std::move(other.m_data)), m_size(other.m_size), m_length(other.m_length)
    {
        other.m_size = 0;
        other.m_length = 0;
    }

    TextBuffer& operator=(const TextBuffer& other) = delete;

    TextBuffer& operator=(TextBuffer&& other) noexcept
    {
        if (this != &other)
        {
            m_data = std::move(other.m_data);
            m_size = other.m_size;
            m_length = other.m_length;
            other.m_size = 0;
            other.m_length = 0;
        }
        return *this;
    }

    /// @returns Character at the specified index reference.
    inline char& at(int index)
    {
        return m_data[index];
    }

    /// @returns The pointer to a null-terminated character array.
    inline const char* c_str() const
    {
        return const_cast<char*>(m_data.get());
    }

    /// @returns The pointer to a null-terminated `T` array.
    template<typename T> inline T* as() const
    {
        return static_cast<T*>((void*) m_data.get());
    }

    /// @brief Clears the buffer by zeroing all of its memory.
    inline void clear()
    {
        m_length = 0;
        memset(m_data.get(), 0, m_size);
    }

    /// @returns The size of the buffer in bytes.
    inline size_t size() const
    {
        return m_size;
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
        if (m_length < m_size) m_length+= snprintf(&(m_data[m_length]), m_size - m_length, format, args...);
    }

    /// @brief Appends a byte character.
    inline void poke(char value)
    {
        if (m_length < m_size) m_data[m_length++] = value;
    }

    /// @brief Appends 2 byte characters.
    inline void poke(char v1, char v2)
    {
        if (m_length < m_size) m_data[m_length++] = v1;
        if (m_length < m_size) m_data[m_length++] = v2;
    }

    /// @brief Appends 3 byte characters.
    inline void poke(char v1, char v2, char v3)
    {
        if (m_length < m_size) m_data[m_length++] = v1;
        if (m_length < m_size) m_data[m_length++] = v2;
        if (m_length < m_size) m_data[m_length++] = v3;
    }

    /// @brief Appends 4 byte characters.
    inline void poke(char v1, char v2, char v3, char v4)
    {
        if (m_length < m_size) m_data[m_length++] = v1;
        if (m_length < m_size) m_data[m_length++] = v2;
        if (m_length < m_size) m_data[m_length++] = v3;
        if (m_length < m_size) m_data[m_length++] = v4;
    }

    inline void poke(char v1, char v2, char v3, char v4, char v5)
    {
        if (m_length < m_size) m_data[m_length++] = v1;
        if (m_length < m_size) m_data[m_length++] = v2;
        if (m_length < m_size) m_data[m_length++] = v3;
        if (m_length < m_size) m_data[m_length++] = v4;
        if (m_length < m_size) m_data[m_length++] = v5;
    }

private:
    std::unique_ptr<char[]> m_data;  // Unique pointer to the heap allocated buffer.
    size_t m_size;                   // Buffer capacity in bytes.
    size_t m_length;                 // Current buffer length.
};
