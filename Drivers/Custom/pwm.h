/**
 * @file      pwm.h
 * @author    CodeDog
 * @brief     HAL TIM PWM generator header file.
 *
 * @copyright (c)2022 CodeDog, All rights reserved.
 */

#pragma once

#include "hal_ex.h"
#include "tim_ex.h"

/**
 * @typedef PWM_HandleTypeDef
 * @struct
 * @brief PWM generator handle.
 */
typedef struct
{
    TIM_HandleTypeDef*  htim;       ///< TIM handle pointer.
    uint32_t            channel;    ///< TIM channel.
    uint16_t            timeBase;   ///< Time base in MHz.
    uint32_t            F;          ///< The PWM frequency in Hz.
    float               DC;         ///< The PWM duty cycle in %.
    uint8_t             state;      ///< 1: ON, 0: OFF.
} PWM_HandleTypeDef;

#ifdef __cplusplus
extern "C" {
#endif

HAL_StatusTypeDef PWM_Init(PWM_HandleTypeDef* hpwm, TIM_HandleTypeDef* htim, uint32_t channel, uint16_t timeBase);
HAL_StatusTypeDef PWM_Set(PWM_HandleTypeDef* hpwm, uint32_t f, float dc);
HAL_StatusTypeDef PWM_Set_F(PWM_HandleTypeDef* hpwm, uint32_t f);
HAL_StatusTypeDef PWM_Set_DC(PWM_HandleTypeDef* hpwm, float dc);
HAL_StatusTypeDef PWM_Start(PWM_HandleTypeDef* hpwm);
HAL_StatusTypeDef PWM_Stop(PWM_HandleTypeDef* hpwm);

#ifdef __cplusplus
}
#endif
