/**
 * @file        InstancePool.hpp
 * @author      CodeDog
 * @brief       A pool of deletable pointers for asynchronous operations.
 *
 * @copyright   (c)2023 CodeDog, All rights reserved.
 */

#pragma once

#include <vector>
#include <stdlib.h>
#include "Mutex.hpp"

/**
 * @brief   A pool of deletable pointers for asynchronous operations.
 * @remarks Instances MUST be created with a `new` keyword.
 *          Using other kind of pointers or dangling pointers will lead to undefined behavior!
 *          Instance management is thread safe using std::mutex.
 */
class InstancePool
{

public:

    /**
     * @brief Creates a pool without a defined capacity.
     */
    InstancePool() : m_pointers(), m_mutex() { }

    /**
     * @brief Creates a pool with pre-allocated capacity.
     * @param capacity Number of instances that the pool can contain without resizing.
     */
    InstancePool(size_t capacity) : m_pointers(capacity), m_mutex() { m_pointers.clear(); }

    /**
     * @brief Adds an instance pointer created with the `new` keyword to the pool.
     * @param pointer A pointer to the instance created with the `new` keyword.
     */
    void* add(void* pointer)
    {
        if (!pointer) return nullptr;
        if (!contains(pointer))
        {
            m_mutex.lock();
            if (m_pointers.size() == m_pointers.capacity())
                m_pointers.reserve(m_pointers.capacity() + 16);
            m_pointers.push_back(pointer);
            m_mutex.unlock();
        }
        return pointer;
    }

    /**
     * @brief Tests if the pool contains the pointer.
     * @param pointer A pointer to the instance created with the `new` keyword.
     * @return 1: Pool contains the pointer. 0: Pool does not contain the pointer.
     */
    bool contains(void* pointer)
    {
        if (!pointer) return false;
        m_mutex.lock();
        for (size_t i = 0, n = m_pointers.size(); i < n; i++)
        {
            auto element = m_pointers.at(i);
            if (element == pointer)
            {
                m_mutex.unlock();
                return true;
            }
        }
        m_mutex.unlock();
        return false;
    }

    /**
     * @brief If the pointer belongs to the pool, removes the pointer from the pool and releases its allocated memory.
     * @param pointer A pointer to the instance created with the `new` keyword.
     */
    void remove(void* pointer)
    {
        if (!pointer) return;
        m_mutex.lock();
        for (size_t i = 0, n = m_pointers.size(); i < n; i++)
        {
            auto element = m_pointers.at(i);
            if (element == pointer)
            {
                free(pointer);
                m_pointers.erase(m_pointers.begin() + i);
                break;
            }
        }
        m_mutex.unlock();
    }

private:

    std::vector<void*> m_pointers;  // Pointers list.
    std::Mutex m_mutex;              // Mutex.

};
