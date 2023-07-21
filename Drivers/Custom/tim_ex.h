/**
 * @file        tim_ex.h
 * @author      CodeDog
 * @brief       HAL TIM extensions header file.
 * @remarks
 *              Uses HAL TIM IRQ to schedule events.
 *
 * @copyright   (c)2022 CodeDog, All rights reserved.
 */

#pragma once

#include "hal_ex.h"

#if (!defined(HAL_TIM_MODULE_ENABLED) || USE_HAL_TIM_REGISTER_CALLBACKS < 1)
#warning "PSSSST! Look here:"
/**
 * This driver depends on HAL TIM with global interrupt and REGISTER CALLBACK features enabled.
 * In STM32CubeIde at least one TIM must be enabled with its global interrupt on.
 * IOC Project Manager > Advanced Settings > Register CallBack
 * change TIM settings to ENABLED.
 */
#endif

#if (defined(HAL_TIM_MODULE_ENABLED) && USE_HAL_TIM_REGISTER_CALLBACKS > 0)

#define DELAY_ASYNC_ENABLED

/**
 * @def WAIT_HANDLES_N
 * @brief Number of reserved delay events.
 */
#define WAIT_HANDLES_N 128

/**
 * @typedef TIM_EX_CallbackTypeDef
 * @fn void (*)(void*)
 * @brief Callback for delay fuction with any pointer passed as argument.
 * @param Any pointer.
 */
typedef void(*TIM_EX_CallbackTypeDef)(void*);

/**
 * @typedef StopwatchCallbackTypeDef
 * @fn void (*)(double)
 * @brief A signature of the stopwatch callback.
 * @param The time in seconds.
 */
typedef void(*StopwatchCallbackTypeDef)(double);

/**
 * @typedef DelayAsync_ConfigTypeDef
 * @brief Contains DelayAsync timing configuration.
 */
typedef struct
{
  TIM_HandleTypeDef* htim;   ///< Timer handle pointer.
  uint16_t time_base;           ///< Timer time base (timer clock) in MHz.
  uint8_t tick_time_exp;        ///< t = 1ns * 10^(tick_time_exp); 0: 1ns, 3: 1μs, 6: 1ms, 9: 1s.
} TIM_EX_ConfigTypeDef;

/**
 * Asynchronous delay event.
 */
typedef struct
{
  uint8_t active;                     ///< 0: free, 1: active.
  uint32_t ticks_left;                ///< Ticks left to complete the delay.
  uint32_t reset;                     ///< If set, the ticks_left will be reset to that value when the time elapses.
  void* binding;                   ///< Binding passed to the callback.
  TIM_EX_CallbackTypeDef callback; ///< Callback to call when the time elapses.
} WaitHandleTypeDef;

/**
 * @typedef StopWatchHandleTypeDef
 * @struct
 * @brief A structure for precise time measurement using HAL TIM.
 */
typedef struct
{
  uint8_t active;
  uint32_t ticks;
  uint32_t ticks_per_second;
  StopwatchCallbackTypeDef second_tick;
} StopwatchHandleTypeDef;

#ifdef __cplusplus
extern "C" {
#endif

HAL_StatusTypeDef TIM_EX_init(TIM_HandleTypeDef* htim, uint16_t time_base, uint8_t tick_time_exp);
WaitHandleTypeDef* delay_async(double ms, void* binding, void (*callback)(void*));
WaitHandleTypeDef* repeat_async(double ms, void* binding, void (*callback)(void*));
void wait_handle_dispose(WaitHandleTypeDef* wh);
void wait_handle_reset(WaitHandleTypeDef* wh, double ms);
void stopwatch_start();
void stopwatch_stop();
void stopwatch_reset();
double stopwatch_get_time();
void stopwatch_register_callback(StopwatchCallbackTypeDef callback);
uint8_t get_TIM_PSC_ARR(uint16_t tb, double f, volatile uint32_t* psc, volatile uint32_t* arr, float* e);
HAL_StatusTypeDef set_TIM_PSC_ARR(TIM_HandleTypeDef* htim, double f, uint16_t tb);

#ifdef __cplusplus
}
#endif

#endif
