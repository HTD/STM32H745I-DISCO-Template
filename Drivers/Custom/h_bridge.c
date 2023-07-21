/**
 * @file      h_bridge.h
 * @author    CodeDog
 * @brief     H-Bride motor controller driver.
 * @remarks
 *            Compatible with A4950, DRV8870 and TB67H450FNG.
 *            Interrupt safe, thread safe, doesn't block.
 *
 * @copyright (c)2022 CodeDog, All rights reserved.
 */

#include "h_bridge.h"

#define DC_H 1.0f // 100% duty cycle, H state.
#define DC_L 0.0f // 0% duty cycle, L state.

/**
 * @fn void set(HBridge_HandleTypeDef*, float, float)
 * @brief Sets the control PWM duty cycles for both PWM generators.
 * @remarks Sets the device state to transient.
 * @param hdev Device handle pointer.
 * @param dc1 Control PWM1 duty cycle <0..1>.
 * @param dc2 Control PWM2 duty cycle <0..1>.
 */
static void set(HBridge_HandleTypeDef* hdev, float dc1, float dc2)
{
  hdev->state = H_BRIDGE_S_TRS;
  PWM_Set(hdev->PWM1, hdev->F, dc1);
  PWM_Set(hdev->PWM2, hdev->F, dc2);
  PWM_Start(hdev->PWM1);
  PWM_Start(hdev->PWM2);
}

/**
 * @fn void reset(HBridge_HandleTypeDef*)
 * @brief Resets the control PWM to the off / standby state.
 * @param hdev Device handle pointer.
 */
static void reset(HBridge_HandleTypeDef* hdev)
{
  PWM_Set(hdev->PWM1, hdev->F, 0);
  PWM_Set(hdev->PWM2, hdev->F, 0);
  PWM_Stop(hdev->PWM1);
  PWM_Stop(hdev->PWM2);
  uint8_t isBraking = hdev->state == H_BRIDGE_S_BRK;
  hdev->state = H_BRIDGE_S_OFF;
  if (hdev->events.PWMStateChanged)
    hdev->events.PWMStateChanged(hdev);
  if (isBraking && hdev->events.brakeCompleted)
    hdev->events.brakeCompleted(hdev);
  else if (hdev->events.initialized)
  {
    hdev->events.initialized(hdev);
    hdev->events.initialized = NULL;
  }
}

/**
 * @fn HAL_StatusTypeDef HBridge_init(HBridge_HandleTypeDef*, PWM_HandleTypeDef*, PWM_HandleTypeDef*, uint32_t)
 * @brief Initializes the H-bridge.
 * @remarks Calculates the minimum and maximum usable duty cycles and sendins a special BRAKE / STOP signal.
 * @param hdev H-Bridge handle pointer.
 * @param PWM mode, one of HBridge_ModeTypeDef enumeration.
 * @param pwm1 PWM generator 1 pointer.
 * @param pwm2 PWM generator 2 pointer.
 * @param f H-Bridge base frequency in Hz.
 * @return HAL_OK: success, HAL_ERROR: invalid parameters.
 */
HAL_StatusTypeDef HBridge_Init(HBridge_HandleTypeDef* hdev, HBridge_ModeTypeDef mode, PWM_HandleTypeDef* pwm1, PWM_HandleTypeDef* pwm2, uint32_t f)
{
  if (!hdev || !pwm1 || !pwm2 || !f) return HAL_ERROR; // invalid parameters.
  hdev->PWM1 = pwm1;
  hdev->PWM2 = pwm2;
  hdev->PWM_Mode = mode;
  hdev->F = f;
  hdev->state = H_BRIDGE_S_OFF;
  float f_min = 1000.0f / H_BRIDGE_TS;
  if (hdev->F < f_min) return HAL_ERROR; // Frequency too low, stand-by threshold exceeded.
  hdev->DCmin = 0.0000001f * hdev->F * H_BRIDGE_TD;
  hdev->DCmax = DC_H - hdev->DCmin;
  set(hdev, DC_H, DC_H);
  delay_async(H_BRIDGE_TR, hdev, (TIM_EX_CallbackTypeDef) reset);
  return HAL_OK;
}

/**
 * @fn HAL_StatusTypeDef HBridge_FWD(HBridge_HandleTypeDef*, float)
 * @brief Starts the motor in forward direction with a speed setting between 0 and 1.
 * @remarks If the same speed is requested, the call will be ignored. If the driver is in transient state the call will fail.
 * @param hdev Device handle pointer.
 * @param dc Duty cycle <0..1>.
 * @return HAL status.
 */
HAL_StatusTypeDef HBridge_FWD(HBridge_HandleTypeDef* hdev, float dc)
{
  if (dc < 0 || dc > 1) return HAL_ERROR;
  if (hdev->state == H_BRIDGE_S_FWD && hdev->DC == dc) return HAL_OK; // Already in state.
  if (hdev->state == H_BRIDGE_S_TRS || hdev->state == H_BRIDGE_S_BRK) return HAL_ERROR; // Invalid state.
  hdev->DC = dc;
  switch (hdev->PWM_Mode)
  {
  case H_BRIDGE_M_DRV_BRK:
    set(hdev, DC_H, dc);
    break;
  case H_BRIDGE_M_DRV_CST:
    set(hdev, dc, DC_L);
    break;
  }
  hdev->state = H_BRIDGE_S_FWD;
  if (hdev->events.PWMStateChanged)
    hdev->events.PWMStateChanged(hdev);
  return HAL_OK;
}

/**
 * @fn HAL_StatusTypeDef HBridge_REV(HBridge_HandleTypeDef*, float)
 * @brief Starts the motor in reverse direction with a speed setting between 0 and 1.
 * @remarks If the same speed is requested, the call will be ignored. If the driver is in transient state the call will fail.
 * @param hdev Device handle pointer.
 * @param dc Duty cycle <0..1>.
 * @return HAL status.
 */
HAL_StatusTypeDef HBridge_REV(HBridge_HandleTypeDef* hdev, float dc)
{
  if (dc < 0 || dc > 1) return HAL_ERROR;
  if (hdev->state == H_BRIDGE_S_REV && hdev->DC == dc) return HAL_OK; // Already in state.
  if (hdev->state == H_BRIDGE_S_TRS || hdev->state == H_BRIDGE_S_BRK) return HAL_ERROR; // Invalid state.
  hdev->DC = dc;
  switch (hdev->PWM_Mode)
  {
  case H_BRIDGE_M_DRV_BRK:
    set(hdev, dc, DC_H);
    break;
  case H_BRIDGE_M_DRV_CST:
    set(hdev, DC_L, dc);
    break;
  }
  hdev->state = H_BRIDGE_S_REV;
  if (hdev->events.PWMStateChanged)
    hdev->events.PWMStateChanged(hdev);
  return HAL_OK;
}

/**
 * @fn HAL_StatusTypeDef HBridge_SetSpeed(HBridge_HandleTypeDef*, float)
 * @brief Adjust the speed of already moving motor with a speed setting between 0 and 1.
 * @param hdev Device handle pointer.
 * @param dc Duty cycle <0..1>.
 * @return HAL status.
 */
HAL_StatusTypeDef HBridge_SetDC(HBridge_HandleTypeDef* hdev, float dc)
{
  if (dc < 0 || dc > 1) return HAL_ERROR;
  switch (hdev->state)
  {
  case H_BRIDGE_S_FWD:
    switch (hdev->PWM_Mode)
    {
    case H_BRIDGE_M_DRV_BRK:
      PWM_Set_DC(hdev->PWM2, dc);
      break;
    case H_BRIDGE_M_DRV_CST:
      PWM_Set_DC(hdev->PWM1, dc);
      break;
    }
    break;
  case H_BRIDGE_S_REV:
    switch (hdev->PWM_Mode)
    {
    case H_BRIDGE_M_DRV_BRK:
      PWM_Set_DC(hdev->PWM1, dc);
      break;
    case H_BRIDGE_M_DRV_CST:
      PWM_Set_DC(hdev->PWM2, dc);
      break;
    }
    break;
  default:
    return HAL_ERROR; // Not moving / invalid state.
  }
  hdev->DC = dc;
  return HAL_OK;
}

/**
 * @fn HAL_StatusTypeDef HBridge_BRK(HBridge_HandleTypeDef*, float)
 * @brief Initializes braking with the given time in milliseconds.
 * @param hdev Device handle pointer.
 * @param time Time in milliseconds to send the brake signal.
 * @return HAL status.
 */
HAL_StatusTypeDef HBridge_BRK(HBridge_HandleTypeDef* hdev, float time)
{
  if (hdev->state == H_BRIDGE_S_BRK || hdev->state == H_BRIDGE_S_TRS) return HAL_ERROR; // Invalid state.
  set(hdev, DC_H, DC_H);
  hdev->state = H_BRIDGE_S_BRK;
  if (hdev->events.brakeStarted)
    hdev->events.brakeStarted(hdev);
  if (time < 0) return HAL_OK;
  if (time < H_BRIDGE_TR) time = H_BRIDGE_TR;
  delay_async(time, hdev, (TIM_EX_CallbackTypeDef) reset);
  return HAL_OK;
}

/**
 * @fn HAL_StatusTypeDef HBridge_OFF(HBridge_HandleTypeDef*)
 * @brief Turns the motor power off without braking.
 * @param hdev Device handle pointer.
 * @return HAL status.
 */
HAL_StatusTypeDef HBridge_OFF(HBridge_HandleTypeDef* hdev)
{
  if (hdev->state == H_BRIDGE_S_OFF) return HAL_OK; // Already off.
  if (hdev->state == H_BRIDGE_S_TRS || hdev->state == H_BRIDGE_S_BRK) return HAL_ERROR; // Invalid state.
  reset(hdev);
  return HAL_OK;
}

#undef DC_H
#undef DC_L
