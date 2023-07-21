/**
 * @file      h_bridge.h
 * @author		CodeDog
 * @brief     H-Bride motor controller driver header file.
 * @remarks
 * 				    Compatible with A4950, DRV8870 and TB67H450FNG.
 * 				    Interrupt safe, thread safe, doesn't block.
 *
 * @copyright	(c)2022 CodeDog, All rights reserved.
 */

#pragma once

#include "pwm.h"
#include "tim_ex.h"

#define H_BRIDGE_TD 1000.0f ///< Pulse detection time in nanoseconds.
#define H_BRIDGE_TR 10.0f   ///< Reset time in milliseconds.
#define H_BRIDGE_TS 0.7f    ///< Stand-by time in milliseconds.

typedef enum
{
  H_BRIDGE_S_OFF, ///< Off / stand-by.
  H_BRIDGE_S_FWD, ///< Drive forward.
  H_BRIDGE_S_REV, ///< Drive reverse.
  H_BRIDGE_S_BRK, ///< Brake.
  H_BRIDGE_S_TRS  ///< Transient state.
} HBridge_StateTypeDef;

typedef enum
{
  H_BRIDGE_M_DRV_BRK, ///< Drive / brake cycle.
  H_BRIDGE_M_DRV_CST  ///< Drive / coast cycle.
} HBridge_ModeTypeDef;

typedef struct __HBridge_HandleTypeDef HBridge_HandleTypeDef;

/**
 * @fn void (*)(void*)
 * @typedef H_Bridge_CallbackTypeDef
 * @brief A callback accepting an optional binding.
 * @param Optional binding.
 */
typedef void (*HBridge_CallbackTypeDef)(HBridge_HandleTypeDef*);

/**
 * @typedef H_Bridge_EventsTypeDef
 * @struct
 * @brief H-Bridge event bindings structure.
 */
typedef struct
{
  HBridge_CallbackTypeDef initialized;     ///< Called when the initialization completes.
  HBridge_CallbackTypeDef brakeStarted;    ///< Called when braking starts.
  HBridge_CallbackTypeDef brakeCompleted;  ///< Called when braking ends.
  HBridge_CallbackTypeDef PWMStateChanged; ///< Called when PWM signal state changes (ON / OFF).
} HBridge_EventsTypeDef;

/**
 * @typedef H_Bridge_HandleTypeDef
 * @struct __H_Bridge_HandleTypeDef
 * @brief H-Bridge handle structure.
 */
typedef struct __HBridge_HandleTypeDef
{
  PWM_HandleTypeDef*    PWM1;     ///< PWM generator 1.
  PWM_HandleTypeDef*    PWM2;     ///< PWM generator 1.
  uint32_t              F;        ///< Operating PWM frequency (set above 10kHz).
  HBridge_ModeTypeDef   PWM_Mode; ///< PWM setting mode.
  float                 DC;       ///< Duty cycle setting <0..1>.
  float                 DCmin;    ///< Minimum usable duty cycle.
  float                 DCmax;    ///< Maximum usable duty cycle.
  HBridge_StateTypeDef  state;    ///< The current state of the motor.
  void*                 binding;  ///< Optional binding passed to callbacks.
  HBridge_EventsTypeDef events;   ///< Optional events callbacks.
} HBridge_HandleTypeDef;

#ifdef __cplusplus
extern "C" {
#endif

HAL_StatusTypeDef HBridge_Init(HBridge_HandleTypeDef* hdev, HBridge_ModeTypeDef mode, PWM_HandleTypeDef* pwm1, PWM_HandleTypeDef* pwm2, uint32_t f);
HAL_StatusTypeDef HBridge_FWD(HBridge_HandleTypeDef* hdev, float dc);
HAL_StatusTypeDef HBridge_REV(HBridge_HandleTypeDef* hdev, float dc);
HAL_StatusTypeDef HBridge_SetDC(HBridge_HandleTypeDef* hdev, float dc);
HAL_StatusTypeDef HBridge_BRK(HBridge_HandleTypeDef* hdev, float time);
HAL_StatusTypeDef HBridge_OFF(HBridge_HandleTypeDef* hdev);

#ifdef __cplusplus
}
#endif
