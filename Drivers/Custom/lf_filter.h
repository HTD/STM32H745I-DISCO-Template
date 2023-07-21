/**
 * @file      lf_filter.h
 * @author    CodeDog
 * @brief     A simple digital filter that requires a digital pin state to be stable for a certain time to trigger an event, header file.
 * @remarks
 *            Using delay_async timer (with usually 1kHz time source) to sample the input pin state.
 *
 * @copyright (c)2022 CodeDog, All rights reserved.
 */

#pragma once

#include "hal_ex.h"
#include "tim_ex.h"

#define LFF_HCOUNT 6

typedef struct __LFF_HandleTypeDef LFF_HandleTypeDef;

typedef void (*LFF_CallbackTypeDef)(LFF_HandleTypeDef*);

typedef struct __LFF_HandleTypeDef
{
  GPIO_PinTypeDef*    pin;              ///< GPIO pin structure pointer.
  WaitHandleTypeDef*  wait_handle;      ///< Registered wait handle pointer.
  const void*         binding;          ///< Pointer to the optional binding.
  float               sample_interval;  ///< The sampling time interval between readings in milliseconds.
  uint16_t            sample_threshold; ///< The number of 1 or 0 samples to trigger event.
  uint16_t            counter;          ///< Counter for 1 (set) readings.
  uint16_t            current_min;      ///< Local counter minimum.
  uint16_t            current_max;      ///< Local counter maximum.
  uint8_t             current_state;    ///< 1: Set, 0: Reset.
  uint8_t             last_sample;      ///< 1: Set, 0: Reset.
  LFF_CallbackTypeDef state_changed;    ///< A function to call when the pin state changes.
  LFF_CallbackTypeDef stats_changed;    ///< A function to call when the pin stats changes.
} LFF_HandleTypeDef;

#ifdef __cplusplus
extern "C"
{
#endif

HAL_StatusTypeDef LFF_RegisterPin(GPIO_PinTypeDef* pin, float sampleInterval, uint16_t sampleThreshold,
    LFF_CallbackTypeDef stateChanged, const void* binding);
LFF_HandleTypeDef* LFF_GetHandle(GPIO_PinTypeDef* pin);
HAL_StatusTypeDef LFF_ChangeInterval(GPIO_PinTypeDef* pin, float interval);
HAL_StatusTypeDef LFF_ChangeThreshold(GPIO_PinTypeDef* pin, float threshold);
HAL_StatusTypeDef LFF_UnregisterPin(GPIO_PinTypeDef* pin);

#ifdef __cplusplus
}
#endif
