/**
 * @file      tim_counter.h
 * @author    CodeDog
 * @brief     TIM based frequency counter header file.
 *
 * @copyright (c)2022 CodeDog, All rights reserved.
 */

#pragma once

#include "hal_ex.h"
#include "tim_ex.h"

#define TC_HCOUNT 2  ///< TIM counter handles count.
#define TC_T_MAX 500 ///< Time in milliseconds that is the maximal period measured.

typedef struct __TC_HandleTypeDef TC_HandleTypeDef;

typedef void (*TC_CallbackTypeDef)(TC_HandleTypeDef*); ///< TIM counter callback.

/**
 * @typedef TC_HandleTypeDef
 * @struct __TC_HandleTypeDef
 * @brief TIM counter handle.
 */
typedef struct __TC_HandleTypeDef
{
    TIM_HandleTypeDef*  htim;               ///< TIM handle pointer.
    uint16_t            channel;            ///< TIM channel.
    uint16_t            timeBase;           ///< Time base in MHz.
    uint16_t            PTR;                ///< Periods to reset.
    uint16_t            T1;                 ///< Ticks count 1.
    uint16_t            T2;                 ///< Ticks count 2.
    uint16_t            TC;                 ///< Ticks count.
    uint16_t            PC;                 ///< Period counter.
    uint32_t            LT;                 ///< Last system tick.
    double              F;                  ///< Measured frequency in Hz.
    double              LF;                 ///< Last measured frequency in Hz.
    uint8_t             state;              ///< Measurement state (0 / 1).
    TC_CallbackTypeDef  frequency_changed;  ///< Frequency changed callback.
} TC_HandleTypeDef;

#ifdef __cplusplus
extern "C" {
#endif

HAL_StatusTypeDef TC_Init_IT(TC_HandleTypeDef* htc, TC_CallbackTypeDef frequency_changed);

#ifdef __cplusplus
}
#endif
