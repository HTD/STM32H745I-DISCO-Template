/**
 * @file        Counter.hpp
 * @author      CodeDog
 * @brief       High precision clock counter, header file.
 *
 * @copyright   (c)2023 CodeDog, All rights reserved.
 */

#pragma once

#include <stdint.h>

/**
 * @brief High precision clock counter.
 */
class Counter
{
public:

    Counter() = delete;
    Counter(const Counter& other) = delete;
    Counter(Counter&& other) = delete;

    // Starts the hardware timer, must be run at least a second before precise measurement can be done.
    static void init();

    // Gets the current number of the hardware timer ticks as a reference for the getTime() method.
    static uint32_t getTicks();

    // Gets the time in seconds that elapsed since the reference tick.
    static double getTime(uint32_t t0);

    // Gets the time slices from subsequent calls.
    static double getTimeSlice(uint32_t& t0);

    /**
     * @brief Spin-waits a specified amount of time.
     * @param t Time to wait. The minmal amount depends on 1 / COUNTER_1S (configured in IOC).
     */
    static void spinWait(double t);

};
