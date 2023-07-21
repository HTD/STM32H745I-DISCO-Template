/**
 * @file        Counter.cpp
 * @author      CodeDog
 * @brief       High precision clock counter.
 *
 * @copyright   (c)2023 CodeDog, All rights reserved.
 */

#include "Counter.hpp"
#include "main.h"

#ifndef COUNTER_TIM
#error COUNTER_TIM must be defined.
#endif
#ifndef COUNTER_1S
#error COUNTER_1S must be defined.
#endif

extern TIM_HandleTypeDef COUNTER_TIM;

/**
 * @brief Initializes high precision time counter.
 */
void Counter::init()
{
    HAL_TIM_Base_Start(&COUNTER_TIM);
}

/**
 * @brief Gets the current number of the hardware timer ticks as a reference for the getTime() method.
 *
 * @return The current number of ticks.
 */
uint32_t Counter::getTicks()
{
    return COUNTER_TIM.Instance->CNT;
}

/**
 * @brief Gets the time in seconds that elapsed since the reference tick.
 *
 * @param t0 Reference tick.
 * @return Time in seconds.
 */
double Counter::getTime(uint32_t t0)
{
    return static_cast<uint32_t>(COUNTER_TIM.Instance->CNT - t0) / static_cast<double>(COUNTER_1S);
}

/**
 * @brief Gets the time slices from subsequent calls.
 *
 * @param t0 Reference tick reference. Gets reset on each call.
 * @return Time in seconds.
 */
double Counter::getTimeSlice(uint32_t& t0)
{
    uint32_t tick = COUNTER_TIM.Instance->CNT;
    double time = static_cast<uint32_t>(tick - t0) / static_cast<double>(COUNTER_1S);
    t0 = tick;
    return time;
}

void Counter::spinWait(double t)
{
    uint32_t t1 = COUNTER_TIM.Instance->CNT + static_cast<uint32_t>(COUNTER_1S * t);
    while (COUNTER_TIM.Instance->CNT < t1);
}
