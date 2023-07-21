#pragma once

#include <cstdlib>
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os2.h"
#include "Thread.hpp"

/**
 * Provides thread safe memory allocation from FreeRTOS heap.
 * When the RTOS is not running, the standard `malloc()` / `free()` will be used.
 *
 * WARNING:
 * Undefined behavior when more than `mallocPoolSize` allocations are made and not freed before the RTOS starts.
 */
class ThreadSafeAllocator
{

public:

    /**
     * @brief Allocates memory from FreeRTOS heap, thread safe.
     *
     * @param size Requested size in bytes.
     * @returns The address of the allocated memory block.
     */
    static void* allocate(size_t size)
    {
        if (osKernelGetState() != osKernelRunning)
        {
            void* obj = malloc(size);
            mallocPoolAdd(obj);
            return obj;
        }
        if (Thread::fromISR()) taskENTER_CRITICAL_FROM_ISR(); else taskENTER_CRITICAL();
        void* obj = pvPortMalloc(size);
        if (Thread::fromISR()) taskEXIT_CRITICAL_FROM_ISR(5); else taskEXIT_CRITICAL();
        return obj;
    }

    /**
     * @brief Frees the memory previously allocated from FreeRTOS heap, thread safe.
     *
     * @param obj The address of the allocated memory block.
     */
    static void deallocate(void* obj)
    {
        if (osKernelGetState() != osKernelRunning || mallocPoolGet(obj)) return free(obj);
        if (Thread::fromISR()) taskENTER_CRITICAL_FROM_ISR(); else taskENTER_CRITICAL();
        vPortFree(obj);
        if (Thread::fromISR()) taskEXIT_CRITICAL_FROM_ISR(5); else taskEXIT_CRITICAL();
    }

    /**
     * @brief Allocates memory from FreeRTOS heap, thread safe.
     *
     * @tparam T The type of the object to allocate the memory for.
     * @returns The pointer to the allocated object.
     */
    template<typename T>
    static T* allocate()
    {
        return reinterpret_cast<T*>(allocate(sizeof(T)));
    }

    /**
     * @brief Frees the memory previously allocated from FreeRTOS heap, thread safe.
     *
     * @tparam T The type of the object to free the memory of.
     * @param obj The pointer to the allocated object.
     */
    template<typename T>
    static void deallocate(T* obj)
    {
        deallocate((void*)obj);
    }

    static void* operator new(size_t size)
    {
        return allocate(size);
    }

    static void operator delete(void* ptr)
    {
        deallocate(ptr);
    }

    virtual ~ThreadSafeAllocator() { }

private:


    static constexpr size_t mallocPoolSize = 128;           // Number of locations that can be tracked as not under RTOS control.
    static inline size_t m_mallocPoolIndex = 0;             // Current pool index for the new tracked allocation.
    static inline void* m_mallocPool[mallocPoolSize] = {};  // An address pool for the tracked allocations not under RTOS control.

    /**
     * @brief Adds an address to the malloc pool.
     *
     * @remarks It tries to find the first free location.
     *          If all locations are used, it falls back to overwrite the oldest one.
     *
     * @param address Memory address.
     * @returns 1: Added. 0: Out of space.
     */
    static void mallocPoolAdd(void* address)
    {
        if (!address) return;
        size_t newIndex = m_mallocPoolIndex;
        bool spaceFound = 0;
        for (size_t i = 0; i < mallocPoolSize; i++)
        {
            if (m_mallocPool[newIndex])
                newIndex = (newIndex + 1) % mallocPoolSize;
            else
            {
                spaceFound = 1;
                break;
            }
        }
        if (!spaceFound) m_mallocPoolIndex = newIndex;
        m_mallocPool[m_mallocPoolIndex] = address;
        m_mallocPoolIndex = (m_mallocPoolIndex + 1) % mallocPoolSize;
    }

    /**
     * @brief Checks if the address exists in the pool and removes it if it does.
     *
     * @param address Memory address.
     * @returns 1: Existed. 0: Didn't exist.
     */
    static bool mallocPoolGet(void* address)
    {
        if (!address) return false;
        for (size_t i = 0; i < mallocPoolSize; i++)
        {
            size_t currentOffset = (m_mallocPoolIndex + i) % mallocPoolSize;
            if (m_mallocPool[currentOffset] == address)
            {
                m_mallocPool[currentOffset] = 0;
                return true;
            }
        }
        return false;
    }

};
