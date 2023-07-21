/**
 * @file		  lf_counter.h
 * @author		CodeDog
 * @brief     Low Frequency Counter with a digital filter for high frequency noise rejection, header file.
 * @remarks
 *            Uses a TIM timer to sample the signal.
 *
 * @copyright	(c)2022 CodeDog, All rights reserved.
 */

#pragma once

#include "hal_ex.h"

//
// LF digital filter requirements:
// I: 1111111111111110011111111000000000000110000001111100111111101111100001000001000000000
// O: 1111111111111111111111111000000000000000000001111111111111111111100000000000000000000.......
//    |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
//    1   1   1   1   1   1   1   0   0   0   0   0   1   1   1   1   1   0
//                            T1                                      T0

#define LFC_HCOUNT 2
#define LFC_AVG_COUNT 10

typedef struct __LFC_HandleTypeDef LFC_HandleTypeDef;

typedef void (*LFC_CallbackTypeDef)(LFC_HandleTypeDef*);

/**
 * @typedef LFC_EdgeTypeDef
 * @enum
 * @brief Edge detection type.
 */
typedef enum
{
  LF_FALLING, ///< H to L signal transition.
  LF_RISING   ///< L to H signal transition.
} LFC_EdgeTypeDef;

/**
 * @typedef LFCounter_FilterTypeDef
 * @struct
 * @brief Digital filter data structure for ST number of samples.
 */
typedef struct
{
  uint8_t   IN; ///< 1: Initialized, 0: First run.
  uint8_t   S0; ///< Pin state on first sample.
  uint8_t   S1; ///< Pin state on last sample.
  uint8_t   CC; ///< Pin state changes count.
  uint16_t  T0; ///< First change offset (SC value).
  uint16_t  T1; ///< Last edge detected offset (SC value).
  uint8_t   AE; ///< 1: Result average enabled, 0: Result average disabled.
  uint8_t   AI; ///< Result average index.

  double AVG[LFC_AVG_COUNT]; ///< A buffer for counting the average value.
} LFC_FilterTypeDef;

/**
 * @typedef LFC_HandleTypeDef
 * @struct __LFC_HandleTypeDef
 * @brief Low frequency counter data structure.
 */
typedef struct __LFC_HandleTypeDef
{
  TIM_HandleTypeDef*  htim;                 ///< TIM handle pointer.
  uint16_t            TB;                   ///< Time base frequency in MHz.
  uint16_t            SR;                   ///< Sample rate.
  uint16_t            ST;                   ///< Stable time in samples.
  GPIO_PinTypeDef*    pin;                  ///< GPIO pin structure pointer.
  LFC_EdgeTypeDef     edge;                 ///< 0: Falling, 1: Rising.
  uint32_t            SC;                   ///< Sample counter.
  uint8_t             LS;                   ///< Last GPIO pin state, 1: SET, 0: RESET.
  uint8_t             RTM;                  ///< 1: Ready to measure, 0: Wait for the edge.
  LFC_FilterTypeDef   filter;               ///< Digital filter data.
  double              T;                    ///< Period in seconds.
  double              t;                    ///< Time of the impulse.
  double              F;                    ///< Frequency in Hz.
  LFC_CallbackTypeDef edge_detected;        ///< A function to call when the configured signal edge is detected.
  LFC_CallbackTypeDef second_edge_detected; ///< A function to call when the next signal edge is detected.
  LFC_CallbackTypeDef T_changed;            ///< A function to call when the signal change is detected.
  void*               binding;              ///< Extra binding to pass.
  uint8_t             is_initialized;       ///< 1: The hardware timer for the counter is initialized, 0: Not initialized.
  uint8_t             is_enabled;           ///< 1: The counter is enabled, hardware timer is started, 0: Disabled, timer stopped.
} LFC_HandleTypeDef;

#ifdef __cplusplus
extern "C"
{
#endif

HAL_StatusTypeDef LFC_Init(LFC_HandleTypeDef* h, TIM_HandleTypeDef* htim, uint16_t tb, uint16_t sr, uint16_t st,
                                  GPIO_PinTypeDef* gpio, LFC_EdgeTypeDef edge);

#ifdef __cplusplus
}
#endif
