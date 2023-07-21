/**
 * @file      pwm.c
 * @author    CodeDog
 * @brief     HAL TIM PWM generator.
 *
 * @copyright (c)2022 CodeDog, All rights reserved.
 */

#include "pwm.h"

/**
 * @fn HAL_StatusTypeDef PWM_Init(PWM_HandleTypeDef*, TIM_HandleTypeDef*, uint32_t, uint16_t)
 * @brief Initializes the PWM generator handle by binding a timer to it.
 * @param hpwm PWM generator handle pointer.
 * @param htim TIM handle pointer.
 * @param channel TIM channel.
 * @param timeBase Time base in MHz.
 * @return
 */
HAL_StatusTypeDef PWM_Init(PWM_HandleTypeDef* hpwm, TIM_HandleTypeDef* htim, uint32_t channel, uint16_t timeBase)
{
  if (!hpwm || !htim || !timeBase) return HAL_ERROR;
  hpwm->htim = htim;
  hpwm->channel = channel;
  hpwm->timeBase = timeBase;
  return HAL_OK;
}

/**
 * @fn volatile uint32_t get_CCR*(PWM_HandleTypeDef*)
 * @brief Gets the CCR register for the PWM generator.
 * @param htc PWM generator handle pointer.
 * @return CCR register pointer.
 */
static volatile uint32_t* get_CCR(PWM_HandleTypeDef* hpwm)
{
  if (!hpwm) return NULL;
  switch (hpwm->channel)
  {
  case TIM_CHANNEL_1: return &hpwm->htim->Instance->CCR1;
  case TIM_CHANNEL_2: return &hpwm->htim->Instance->CCR2;
  case TIM_CHANNEL_3: return &hpwm->htim->Instance->CCR3;
  case TIM_CHANNEL_4: return &hpwm->htim->Instance->CCR4;
  case TIM_CHANNEL_5: return &hpwm->htim->Instance->CCR5;
  case TIM_CHANNEL_6: return &hpwm->htim->Instance->CCR6;
  default: return NULL;
  }
}

/**
 * @fn HAL_StatusTypeDef assert_params(PWM_HandleTypeDef*)
 * @brief Checks if the handle parameters are sane.
 * @param hpwm PWM generator handle pointer.
 * @return HAL status.
 */
static HAL_StatusTypeDef assert_params(PWM_HandleTypeDef* hpwm)
{
  return (!hpwm ||
    hpwm->timeBase < 1 ||
    hpwm->F < 1 || hpwm->F > hpwm->timeBase * 1000000 ||
    hpwm->DC < 0 || hpwm->DC > 1) ? HAL_ERROR : HAL_OK;
}

/**
 * @fn HAL_StatusTypeDef update(PWM_HandleTypeDef*)
 * @brief Updates the TIM registers with the handle parameters.
 * @param hpwm PWM generator handle pointer.
 * @return HAL status.
 */
static HAL_StatusTypeDef update(PWM_HandleTypeDef* hpwm)
{
  HAL_StatusTypeDef status;
  status = assert_params(hpwm);
  if (status != HAL_OK) return status;
  status = set_TIM_PSC_ARR(hpwm->htim, hpwm->F, hpwm->timeBase);
  if (status != HAL_OK) return status;
  *get_CCR(hpwm) = (uint32_t) (hpwm->DC * (float)hpwm->htim->Instance->ARR);
  return status;
}

/**
 * @fn HAL_StatusTypeDef PWM_Set(PWM_HandleTypeDef*, uint32_t, float)
 * @brief Initializes the PWM generator with the given frequency and duty cycle.
 * @param hpwm PWM generator handle pointer.
 * @param f Frequency in Hz.
 * @param dc Duty cycle <0..1>.
 * @return HAL status.
 */
HAL_StatusTypeDef PWM_Set(PWM_HandleTypeDef* hpwm, uint32_t f, float dc)
{
  if (f > 0 && f < hpwm->timeBase * 1000000) hpwm->F = f;
  else return HAL_ERROR;
  if (dc >= 0 && dc <= 1) hpwm->DC = dc;
  else return HAL_ERROR;
  return update(hpwm);
}

/**
 * @fn HAL_StatusTypeDef PWM_Set_F(PWM_HandleTypeDef*, uint32_t)
 * @brief Sets the PWM generator frequency.
 * @param hpwm PWM generator handle pointer.
 * @param f Frequency in Hz.
 * @return HAL status.
 */
HAL_StatusTypeDef PWM_Set_F(PWM_HandleTypeDef* hpwm, uint32_t f)
{
  if (f > 0 && f < hpwm->timeBase * 1000000) hpwm->F = f;
  else return HAL_ERROR;
  return update(hpwm);
}

/**
 * @fn HAL_StatusTypeDef PWM_Set_DC(PWM_HandleTypeDef*, float)
 * @brief Sets the PWM generator duty cycle.
 * @param hpwm PWM generator handle pointer.
 * @param dc Duty cycle <0..1>.
 * @return HAL status.
 */
HAL_StatusTypeDef PWM_Set_DC(PWM_HandleTypeDef* hpwm, float dc)
{
  if (dc >= 0 && dc <= 1) hpwm->DC = dc;
  else return HAL_ERROR;
  return update(hpwm);
}

/**
 * @fn HAL_StatusTypeDef PWM_Start(PWM_HandleTypeDef*)
 * @brief Starts the PWM generator if not already started.
 * @param hpwm PWM generator handle pointer.
 * @return HAL status.
 */
HAL_StatusTypeDef PWM_Start(PWM_HandleTypeDef* hpwm)
{
  HAL_StatusTypeDef status;
  if (!hpwm->state)
  {
    status = HAL_TIM_PWM_Start_IT(hpwm->htim, hpwm->channel);
    if (status != HAL_OK) return status;
    hpwm->state = 1;
    return status;
  }
  return HAL_OK; // it's OK when already started.
}

/**
 * @fn HAL_StatusTypeDef PWM_Stop(PWM_HandleTypeDef*)
 * @brief Stops the PWM generator if not already stopped.
 * @param hpwm PWM generator handle pointer.
 * @return HAL status.
 */
HAL_StatusTypeDef PWM_Stop(PWM_HandleTypeDef* hpwm)
{
  HAL_StatusTypeDef status;
  if (hpwm->state)
  {
    status = HAL_TIM_PWM_Stop_IT(hpwm->htim, hpwm->channel);
    if (status != HAL_OK) return status;
    hpwm->state = 0;
    return status;
  }
  return HAL_OK; // it's OK when already stopped.
}
