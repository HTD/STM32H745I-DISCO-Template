/**
 * @file        MutexImpl.hpp
 * @author      CodeDog
 * @brief       Mutex C++ RAII implementation using CMSIS_OS mutex.
 *
 * @copyright   (c)2023 CodeDog, All rights reserved.
 */

#pragma once

#include "cmsis_os2.h"

/**
 * @brief A base class for std::mutex replacement implementations.
 */
class MutexBase
{
public:

    MutexBase() = default;
    virtual ~MutexBase() = default;

    MutexBase(const MutexBase&) = delete;
    MutexBase(MutexBase&&) = delete;
    MutexBase& operator=(const MutexBase&) = delete;
    MutexBase& operator=(MutexBase&&) = delete;

    virtual void lock() = 0;    // Locks the mutex, blocks if the mutex is not available.
    virtual void unlock() = 0;  // Unlocks the mutex.

};

namespace std
{

/**
 * @brief Mutex C++ RAII implementation using CMSIS_OS mutex.
 */
class Mutex : MutexBase
{
public:

    Mutex(uint32_t timeout = 32) : m_mutex(), m_timeout(timeout) { }

    Mutex(const Mutex&) = delete;
    Mutex(Mutex&&) = delete;

    ~Mutex() { if (m_mutex) osMutexDelete(m_mutex); }

    /**
     * @brief Locks the mutex, blocks if the mutex is not available.
     */
    virtual void lock()
    {
        if (!m_mutex) m_mutex = osMutexNew(nullptr); // Created here to avoid crash when the type is constructed before kernel starts.
        osMutexAcquire(m_mutex, m_timeout);
    }

    /**
     * @brief Unlocks the mutex.
     */
    virtual void unlock()
    {
        if (!m_mutex) return;
        osMutexRelease(m_mutex);
    }

private:

    osMutexId_t m_mutex;    // OS mutex.
    uint32_t m_timeout;     // Default timeout.

};

}