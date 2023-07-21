/**
 * @file        Thread.hpp
 * @author      CodeDog
 * @brief       Simple thread synchronization tool, header file.
 * @remarks     Call Thread::tick() each time the threads can synchronize.
 *
 * @copyright   (c)2023 CodeDog, All rights reserved.
 */

#pragma once

#include <stdint.h>
#include "Action.hpp"

/**
 * @class Thread
 * @brief Thread synchronization tool.
 */
class Thread
{
public:

    using threadId_t = int32_t;               // Thread identifier type.
    static constexpr threadId_t None = -1;    // No thread, empty value.
    static constexpr threadId_t Default = 0;  // Default thread, should tick every 1ms.
    static constexpr threadId_t Frame = 1;    // GUI thread, should tick every frame.

    Thread() = delete;
    Thread(const Thread& other) = delete;
    Thread(Thread&& other) = delete;

    /// @brief Synchronizes an action to be called from the tick thread.
    static void sync(Action action, threadId_t threadId = Default);

    /// If the action is passed from ISR, synchronizes an action to be called from the tick thread.
    /// Just calls the action otherwise.
    static void syncISR(Action action, threadId_t threadId = Default);

    /// @returns 1: Called from ISR. 0: Not called from ISR.
    static bool fromISR();

    /// Displays a debug error message if the function was called from ISR.
    /// Should be compiled conditionally in `debug` builds.
    /// When the error is triggered it's unrecoverable, reset the device, fix the underlying problem.
    static void warnISR();

    /// @brief Calls the actions added with the sync method and returns the thread to the pool afterwards.
    static void tick(threadId_t threadId = Default);

protected:
    /// @brief Represent an action bound to a specified thread.
    struct ThreadAction final
    {
        Action action;          // Action callback.
        threadId_t threadId;    // Thread identifier.
        ThreadAction() : action(), threadId(None) { }
    };

    constexpr static const uint8_t max = 16; ///< Maximal number of actions that can be scheduled within 1 tick time.
    static inline ThreadAction actions[max]{};
    static inline bool e = 0; ///< 1: Enabled, at least one tick called. 0: Disabled, tick not called.
    static inline uint8_t q = 0; ///< 1: Actions present. 0: No actions present.

};
