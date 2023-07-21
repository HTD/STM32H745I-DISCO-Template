/**
 * @file      1wire_tim.h
 * @author    CodeDog
 * @brief     1-Wire TIM bit banging transport layer driver header file.
 * @remarks
 *            Uses HAL TIM for precise line bit banging.
 *            See: https://www.maximintegrated.com/en/design/technical-documents/app-notes/1/126.html
 *
 * @copyright	(c)2022 CodeDog, All rights reserved.
 */

#pragma once

#include "1wire.h"

#if WIRE_USE_TIM

// Values below should be tested and adjusted for the specific MCU / TIMER:

#define WIRE_TIM_APB_CLK      200   ///< TIM time source frequency in MHz.
#define WIRE_TIM_LAG1         130   ///< TIM start (ENABLE) lag in 10 nanosecond ticks.
#define WIRE_TIM_LAG2         245   ///< TIM start (ENABLE) IRQ delay in 10 nanosecond ticks.

// Following values are taken directly from Maxim documentation:

#define WIRE_TIM_RESET_TL     480u  ///< Maxim docs Table 2 H: Reset signal low state time.
#define WIRE_TIM_RESET_TR     70u   ///< Maxim docs Table 2 I: Reset signal time to wait for the response.
#define WIRE_TIM_RESET_TW     410u  ///< Maxim docs Table 2 J: Reset signal time to wait to complete sequence.
#define WIRE_TIM_TX_T0L       60u   ///< Maxim docs Table 2 C: TX0 signal low state time.
#define WIRE_TIM_TX_T0H       10u   ///< Maxim docs Table 2 D: TX0 signal high state time.
#define WIRE_TIM_TX_T1L       6u    ///< Maxim docs Table 2 A: TX1 signal low state time.
#define WIRE_TIM_TX_T1H       64u   ///< Maxim docs Table 2 B: TX1 signal high state time.
#define WIRE_TIM_RX_TL        6u    ///< Maxim docs Table 2 A: RX signal low state time.
#define WIRE_TIM_RX_TH        9u    ///< Maxim docs Table 2 E: RX signal high state time.
#define WIRE_TIM_RX_TW        55u   ///< Maxim docs Table 2 F: RX signal wait after read time.

/**
 * @typedef Wire_TimStateTypeDef
 * @enum
 * @brief 1-Wire TIM state machine states.
 */
typedef enum
{
  WIRE_TS_READY,      ///< Ready for the next signal.
  WIRE_TS_RESET_L,    ///< Reset signal start with 0 being sent.
  WIRE_TS_RESET_H,    ///< Reset singal step with 1 being sent.
  WIRE_TS_RESET_READ, ///< Reset signal read step.
  WIRE_TS_TX0_L,      ///< Send bit 0 start with 0 being sent.
  WIRE_TS_TX0_H,      ///< Send bit 0 step with 1 being sent.
  WIRE_TS_TX1_L,      ///< Send bit 1 start with 0 being sent.
  WIRE_TS_TX1_H,      ///< Send bit 1 step with 1 being sent.
  WIRE_TS_RX_L,       ///< Receive bit start with 0 being sent.
  WIRE_TS_RX_H,       ///< Receive bit step with 1 being sent.
  WIRE_TS_RX_READ,    ///< Receive bit read step.
  WIRE_TS_CPLT        ///< Sequence complete.
} Wire_TimStateTypeDef;

#if WIRE_USE_IT

typedef struct
{
  Wire_HandleTypeDef* wire;           ///< 1-Wire instance pointer.
  Wire_TimStateTypeDef state;         ///< Current bit-banging operation state.
  uint8_t* target;                    ///< Current bit target.
} Wire_TimBindingTypeDef;

#endif

extern Wire_DriverTypeDef wire_tim_driver;
void wire_set_calibration(TIM_HandleTypeDef* calibrationTIM);

#endif // WIRE_USE_TIM
