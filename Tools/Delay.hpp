/**
 * @file        Delay.hpp
 * @author      CodeDog
 * @brief       Asynchronous delay using FreeRTOS threads.
 *
 * @copyright	(c)2023 CodeDog, All rights reserved.
 */

#pragma once

#include "cmsis_os2.h"
#include <unordered_map>
#include <stdlib.h>
#include "Action.hpp"
#include "Mutex.hpp"
#include "Thread.hpp"

/**
 * Uses FreeRTOS threads to delay actions asynchronously.
 * Do not use where a high precision or low latency is required.
 * Do use where ISR run context is not allowed.
 */
class Delay
{
public:

    /**
     * @brief   Sets a delayed action as a new thread. Cheap version.
     * @remarks Doesn't create and store the task parameters.
     *
     * @tparam ticks Delay time in OS ticks, 1ms default.
     * @param action An action to call when the time elapses.
     * @returns OS thread identifier that can be used to cancel the thread.
     */
    template<uint32_t ticks>
    static osThreadId_t set(Action action)
    {
        return osThreadNew([](void* vpAction){
            osDelay(ticks);
            reinterpret_cast<Action>(vpAction)();
            osThreadTerminate(osThreadGetId());
        }, reinterpret_cast<void*>(action), nullptr);
    }

    /**
     * @brief Sets a delayed action as a new thread. Full version.
     *        Runtime ticks value supported.
     *
     * @param ticks Delay time in OS ticks, 1ms default.
     * @param action Action to call when the time elapses.
     * @returns OS thread identifier that can be used to cancel the thread.
     */
    static osThreadId_t set(uint32_t ticks, Action action)
    {
#ifdef DEBUG
        Thread::warnISR();
#endif
        Params* params = new Params { ticks, action };
        osThreadId_t threadId = osThreadNew([](void* vParams){
            Params* iParams = reinterpret_cast<Params*>(vParams);
            osDelay(iParams->ticks);
            iParams->action();
            osThreadId_t iThreadId = osThreadGetId();
            osThreadTerminate(iThreadId);
            removeTask(iThreadId);
        }, reinterpret_cast<void*>(params), nullptr);
        m_mutex.lock();
        m_tasks[threadId] = params;
        m_mutex.unlock();
        return threadId;
    }

    /**
     * @brief Sets a delayed action taking an argument reference as a new thread.
     *
     * @tparam TArg The argument reference type.
     * @param ticks Delay time in OS ticks, 1ms default.
     * @param reference Argument reference.
     * @param action Action to call when the time elapses.
     * @returns OS thread identifier that can be used to cancel the thread.
     */
    template<typename TArg>
    static osThreadId_t set(uint32_t ticks, TArg& reference, Ac1<TArg&> action)
    {
#ifdef DEBUG
        Thread::warnISR();
#endif
        ParamsT<TArg>* params = new ParamsT<TArg> { ticks, action, reference };
        osThreadId_t threadId = osThreadNew([](void* vParams){
            ParamsT<TArg>* iParams = reinterpret_cast<ParamsT<TArg>*>(vParams);
            osDelay(iParams->ticks);
            iParams->action(iParams->reference);
            osThreadId_t iThreadId = osThreadGetId();
            osThreadTerminate(iThreadId);
            removeTask(iThreadId);
        }, reinterpret_cast<void*>(params), nullptr);
        m_mutex.lock();
        m_tasks[threadId] = params;
        m_mutex.unlock();
        return threadId;
    }

    /**
     * @brief Cancels a task started with set() method, if it was started and is currently running.
     *
     * @param threadId OS thread identifier.
     */
    static void cancel(osThreadId_t threadId)
    {
        if (!threadId) return;
        removeTask(threadId);
        if (osThreadGetState(threadId) == osThreadRunning) osThreadTerminate(threadId);
    }

protected:

    /**
     * @brief Removes the task from the m_task map if it's there.
     *
     * @param threadId OS thread identifier.
     */
    static void removeTask(osThreadId_t threadId)
    {
        m_mutex.lock();
        auto it = m_tasks.find(threadId);
        if (it != m_tasks.end())
        {
            free(it->second);
            m_tasks.erase(it);
        }
        m_mutex.unlock();
    }

    /**
     * @brief A structure to store delay action parameters without argument.
     */
    struct Params
    {
        uint32_t ticks;
        Action action;
    };

    /**
     * @brief A structure to store delayed action parameters with argument.
     *
     * @tparam TArg The type of the action argument.
     */
    template<typename TArg>
    struct ParamsT
    {
        uint32_t ticks;
        Ac1<TArg&> action;
        TArg& reference;
    };

    /**
     * @brief A map of allocated `Params<TArg>` structures
     *        used to deallocate them when completing or cancelling the task.
     */
    static inline std::unordered_map<osThreadId_t, void*> m_tasks{};

    /**
     * @brief Mutex for protecting the `m_tasks` collection access.
     */
    static inline std::Mutex m_mutex = std::Mutex();

};
