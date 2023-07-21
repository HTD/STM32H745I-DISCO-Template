/**
 * @file        Thread.cpp
 * @author      CodeDog
 * @brief       Simple thread synchronization tool.
 * @remarks     Call Thread::tick() each time the threads can synchronize.
 *
 * @copyright   (c)2023 CodeDog, All rights reserved.
 */

#include "Thread.hpp"
#include "Debug.hpp"
#include "stm32h745xx.h"

/**
 * @remarks When called again with the same action before the next tick occurs - it's ignored.
 *          Also does nothing if the thread pool is full (scheduled actions limit exceeded).
 */
void Thread::sync(Action action, threadId_t threadId)
{
    if (!action) return;
    if (!e) { action(); return; }
    for (uint8_t i = 0; i < max; i++)
    {
        if (actions[i].action == action) return;
        if (!actions[i].action)
        {
            actions[i].action = action;
            actions[i].threadId = threadId;
            ++q;
            return;
        }
    }
}

void Thread::syncISR(Action action, threadId_t threadId)
{
    if (e && fromISR()) sync(action, threadId); else action();
}

bool Thread::fromISR()
{
    return (SCB->ICSR & SCB_ICSR_VECTACTIVE_Msk) != 0;
}

void Thread::warnISR()
{
    if (fromISR()) Debug::log(LogMessage::error, "Invalid call from ISR, system compromised!");
}

void Thread::tick(threadId_t threadId)
{
    e = true;
    if (!q) return;
    for (uint8_t i = 0; i < max; i++)
    {
        ThreadAction threadAction = actions[i];
        if (!threadAction.action || threadAction.threadId != threadId) continue;
        if (q) --q;
        threadAction.action();
        actions[i].action = nullptr;
        actions[i].threadId = None;
    }
}
