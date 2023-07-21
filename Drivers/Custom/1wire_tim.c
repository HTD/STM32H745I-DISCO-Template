/**
 * @file      1wire_tim.c
 * @author    CodeDog
 * @brief     1-Wire TIM bit banging transport layer driver.
 * @remarks
 *            Uses HAL TIM for precise line bit banging.
 *            See: https://www.maximintegrated.com/en/design/technical-documents/app-notes/1/126.html
 *
 * @copyright	(c)2022 CodeDog, All rights reserved.
 */

#include "1wire.h"

#if WIRE_USE_TIM

#include "1wire_tim.h"

static TIM_HandleTypeDef* CalibrationTIM;     ///< A calibration counter pointer (used to measure wire TIM lags).
static uint16_t WireTIMLag1 = WIRE_TIM_LAG1;  ///< A number of 10ns time slices between starting the timer and the first tick being count.
static uint16_t WireTIMLag2 = WIRE_TIM_LAG2;  ///< A number of 10ns time slices between starting the timer and the TIM IRQ being handled.
static uint8_t WireCalibrationState = 0;      ///< A current state of the wire TIM calibration.

/**
 * @fn void wire_set_calibration(TIM_HandleTypeDef*)
 * @brief Sets the calibration timer for the 1-Wire driver.
 * @param calibrationTIM A pointer to a timer ticking considerably faster than the 1-Wire timer.
 */
void wire_set_calibration(TIM_HandleTypeDef* calibrationTIM)
{
  CalibrationTIM = calibrationTIM;
}

/**
 * @fn void wire_tim_calibrate(Wire_HandleTypeDef*)
 * @brief Calibrates the 1-Wire TIM activation lag times.
 * @param wire 1-Wire handle pointer.
 */
static void wire_tim_calibrate(Wire_HandleTypeDef* wire)
{
  if (!CalibrationTIM || !wire || !wire->TIM || WireCalibrationState) return;
  TIM_TypeDef* instance = wire->TIM->Instance;
  TIM_TypeDef* reference = CalibrationTIM->Instance;
  reference->PSC = 0; // just to make sure, that the further calculations are valid.
  HAL_TIM_Base_Start(CalibrationTIM);
  HAL_Delay(1);
  instance->ARR = 0xfffe;
  instance->CNT = 0;
  reference->CNT = 0;
  WireCalibrationState = 0;
  __HAL_TIM_ENABLE(wire->TIM);
  while (instance->CNT == 0) { }
  WireTIMLag1 = reference->CNT / (instance->PSC + 1);
#if WIRE_USE_IT
  while (WireCalibrationState == 0) { }
  WireTIMLag2 = reference->CNT / (instance->PSC + 1);
#else
  __HAL_TIM_DISABLE(wire->TIM);
#endif
  HAL_TIM_Base_Stop(CalibrationTIM);
}

static inline void set_L(Wire_HandleTypeDef* wire) { HAL_EX_PIN_SET_L(wire->GPIO); }
static inline void set_H(Wire_HandleTypeDef* wire) { HAL_EX_PIN_SET_H(wire->GPIO); }
static inline uint8_t get_state(Wire_HandleTypeDef* wire) { return HAL_EX_PIN_GET_STATE(wire->GPIO); }

#if WIRE_USE_POLLING

/**
 * @fn void delay_us(Wire_HandleTypeDef*, uint16_t)
 * @brief Spin waits the t microseconds.
 * @param wire 1-Wire handle pointer.
 * @param t The number of microseconds to wait.
 */
static void delay_us(Wire_HandleTypeDef* wire, uint16_t t)
{
  uint16_t target = 100 * t;
  wire->TIM->Instance->ARR = 0xfffe;
  wire->TIM->Instance->CNT = WireTIMLag1;
  __HAL_TIM_ENABLE(wire->TIM);
  while (wire->TIM->Instance->CNT < target);
  __HAL_TIM_DISABLE(wire->TIM);
}

/**
 * @fn HAL_StatusTypeDef tim_reset(Wire_HandleTypeDef*)
 * @brief Sends the RESET signal in polling mode.
 * @param wire 1-Wire handle pointer.
 * @return HAL status: HAL_OK - valid reset response, HAL_ERROR - invalid reset response.
 */
static HAL_StatusTypeDef tim_reset(Wire_HandleTypeDef* wire)
{
  if (!get_state(wire)) return HAL_ERROR;
  set_L(wire);
  delay_us(wire, WIRE_TIM_RESET_TL);
  set_H(wire);
  delay_us(wire, WIRE_TIM_RESET_TR);
  wire->status = get_state(wire) ? HAL_ERROR : HAL_OK;
  delay_us(wire, WIRE_TIM_RESET_TW);
  return wire->status;
}

/**
 * @fn HAL_StatusTypeDef tim_bit_rx(Wire_HandleTypeDef*, uint8_t*)
 * @brief Receives a single bit in polling mode.
 * @param wire 1-Wire handle pointer.
 * @param target A pointer to the 8-bit interger that will receive the value read.
 * @return HAL status.
 */
static HAL_StatusTypeDef tim_bit_rx(Wire_HandleTypeDef* wire, uint8_t* target)
{
  set_L(wire);
  delay_us(wire, WIRE_TIM_RX_TL);
  set_H(wire);
  delay_us(wire, WIRE_TIM_RX_TH);
  *target = get_state(wire);
  delay_us(wire, WIRE_TIM_RX_TW);
  return wire->status;
}

/**
 * @fn HAL_StatusTypeDef tim_bit_tx(Wire_HandleTypeDef*, uint8_t*)
 * @brief Sends a single bit in polling mode.
 * @param wire 1-Wire handle pointer.
 * @param target A pointer to the 8-bit integer that has the value to send.
 * @return HAL status.
 */
static HAL_StatusTypeDef tim_bit_tx(Wire_HandleTypeDef* wire, uint8_t* target)
{
  set_L(wire);
  delay_us(wire, *target ? WIRE_TIM_TX_T1L : WIRE_TIM_TX_T0L);
  set_H(wire);
  delay_us(wire, *target ? WIRE_TIM_TX_T1H : WIRE_TIM_TX_T0H);
  return wire->status;
}

#endif // WIRE_USE_POLLING

#if WIRE_USE_IT

static Wire_TimBindingTypeDef bindings[WIRE_HCOUNT]; ///< Bindings for handles.

/**
 * @fn Wire_HandleTypeDef get_handle_by_tim*(TIM_HandleTypeDef*)
 * @brief Gets the 1-Wire handle pointer for TIM handle pointer.
 * @param htim TIM handle pointer.
 * @return 1-Wire handle pointer if initialized, NULL otherwise.
 */
static Wire_HandleTypeDef* get_handle_by_tim(TIM_HandleTypeDef* htim)
{
  for (uint8_t i = 0; i < WIRE_HCOUNT; i++)
    if (wire_handles[i] && wire_handles[i]->TIM == htim)
      return wire_handles[i];
  return NULL;
}

/**
 * @fn Wire_TimBindingTypeDef get_tim_binding*(Wire_HandleTypeDef*)
 * @brief Gets the binding data for the 1-Wire handle.
 * @param wire 1-Wire handle pointer.
 * @return Binding data pointer.
 */
static Wire_TimBindingTypeDef* get_tim_binding(Wire_HandleTypeDef* wire)
{
  for (uint8_t i = 0; i < WIRE_HCOUNT; i++)
    if (bindings[i].wire == wire) return &bindings[i];
  return NULL;
}

/**
 * @fn void set_tim_binding(Wire_HandleTypeDef*)
 * @brief Sets a new binding for the 1-Wire handle.
 * @param wire 1-Wire handle pointer.
 */
static void set_tim_binding(Wire_HandleTypeDef* wire)
{
  for (uint8_t i = 0; i < WIRE_HCOUNT; i++)
  {
    if (!bindings[i].wire || bindings[i].wire == wire)
    {
      bindings[i].wire = wire;
      bindings[i].state = WIRE_TS_READY;
      bindings[i].target = NULL;
      break;
    }
  }
}

/**
 * @fn void start_delay_us(Wire_HandleTypeDef*, uint16_t)
 * @brief Sets the timer interrupt to be called in t microseconds.
 * @param wire 1-Wire handle pointer.
 * @param t Time to call interrupt handler in microseconds.
 */
static inline void start_delay_us(Wire_HandleTypeDef* wire, uint16_t t)
{
  wire->TIM->Instance->ARR = t * 100 - 1;
  wire->TIM->Instance->CNT = WireTIMLag2;
  __HAL_TIM_ENABLE(wire->TIM);
}

/**
 * @fn void tim_state_machine(TIM_HandleTypeDef*)
 * @brief Handles TIM period complete events.
 * @param htim TIM handle pointer.
 */
void tim_state_machine(TIM_HandleTypeDef* htim)
{
  __HAL_TIM_DISABLE(htim);
  Wire_HandleTypeDef* wire = get_handle_by_tim(htim);
  if (!wire) return;
  Wire_TimBindingTypeDef* binding = get_tim_binding(wire);
  if (!binding) return;
  switch (binding->state)
  {
  case WIRE_TS_READY:
    WireCalibrationState = 1;
    break;
  case WIRE_TS_RESET_L:
    set_L(wire);
    binding->state = WIRE_TS_RESET_H;
    start_delay_us(wire, WIRE_TIM_RESET_TL);
    break;
  case WIRE_TS_RESET_H:
    set_H(wire);
    binding->state = WIRE_TS_RESET_READ;
    start_delay_us(wire, WIRE_TIM_RESET_TR);
    break;
  case WIRE_TS_RESET_READ:
    wire->status = get_state(wire) ? HAL_ERROR : HAL_OK;
    binding->state = WIRE_TS_CPLT;
    start_delay_us(wire, WIRE_TIM_RESET_TW);
    break;
  case WIRE_TS_TX0_L:
    set_L(wire);
    binding->state = WIRE_TS_TX0_H;
    start_delay_us(wire, WIRE_TIM_TX_T0L);
    break;
  case WIRE_TS_TX0_H:
    set_H(wire);
    binding->state = WIRE_TS_CPLT;
    start_delay_us(wire, WIRE_TIM_TX_T0H);
    break;
  case WIRE_TS_TX1_L:
    set_L(wire);
    binding->state = WIRE_TS_TX1_H;
    start_delay_us(wire, WIRE_TIM_TX_T1L);
    break;
  case WIRE_TS_TX1_H:
    set_H(wire);
    binding->state = WIRE_TS_CPLT;
    start_delay_us(wire, WIRE_TIM_TX_T1H);
    break;
  case WIRE_TS_RX_L:
    set_L(wire);
    binding->state = WIRE_TS_RX_H;
    start_delay_us(wire, WIRE_TIM_RX_TL);
    break;
  case WIRE_TS_RX_H:
    set_H(wire);
    binding->state = WIRE_TS_RX_READ;
    start_delay_us(wire, WIRE_TIM_RX_TH);
    break;
  case WIRE_TS_RX_READ:
    *(binding->target) = get_state(wire);
    binding->state = WIRE_TS_CPLT;
    start_delay_us(wire, WIRE_TIM_RX_TW);
    break;
  case WIRE_TS_CPLT:
    binding->state = WIRE_TS_READY;
    wire_state_machine(wire);
    break;
  }
}

/**
 * @fn HAL_StatusTypeDef tim_reset_it(Wire_HandleTypeDef*)
 * @brief Sends the reset signal in interrupt mode.
 * @param wire 1-Wire handle pointer.
 * @return HAL status.
 */
static HAL_StatusTypeDef tim_reset_it(Wire_HandleTypeDef* wire)
{
  Wire_TimBindingTypeDef* binding = get_tim_binding(wire);
  if (!binding || binding->state != WIRE_TS_READY || !get_state(wire)) return wire->status = HAL_ERROR;
  binding->state = WIRE_TS_RESET_L;
  tim_state_machine(wire->TIM);
  return wire->status;
}

/**
 * @fn HAL_StatusTypeDef tim_bit_rx_it(Wire_HandleTypeDef*, uint8_t*)
 * @brief Receives a single bit in interrupt mode.
 * @param wire 1-Wire handle pointer.
 * @param target A pointer to the 8-bit interger that will receive the value read.
 * @return HAL status.
 */
static HAL_StatusTypeDef tim_bit_rx_it(Wire_HandleTypeDef* wire, uint8_t* target)
{
  Wire_TimBindingTypeDef* binding = get_tim_binding(wire);
  if (!binding || binding->state != WIRE_TS_READY)
    return HAL_ERROR;
  binding->state = WIRE_TS_RX_L;
  binding->target = target;
  tim_state_machine(wire->TIM);
  return wire->status;
}

/**
 * @fn HAL_StatusTypeDef tim_bit_tx_it(Wire_HandleTypeDef*, uint8_t*)
 * @brief Sends a single bit in interrupt mode.
 * @param wire 1-Wire handle pointer.
 * @param target A pointer to the 8-bit integer that has the value to send.
 * @return HAL status.
 */
static HAL_StatusTypeDef tim_bit_tx_it(Wire_HandleTypeDef* wire, uint8_t* target)
{
  Wire_TimBindingTypeDef* binding = get_tim_binding(wire);
  if (!binding || binding->state != WIRE_TS_READY)
    return HAL_ERROR;
  binding->state = *target ? WIRE_TS_TX1_L : WIRE_TS_TX0_L;
  tim_state_machine(wire->TIM);
  return wire->status;
}

#endif // WIRE_USE_IT

/**
 * @fn HAL_StatusTypeDef tim_init(Wire_HandleTypeDef*, void*)
 * @brief Initializes the 1-Wire TIM driver.
 * @param wire 1-Wire handle pointer.
 * @return HAL status.
 */
static HAL_StatusTypeDef tim_init(Wire_HandleTypeDef* wire)
{
  if (!wire || !wire->TIM) return HAL_ERROR;
  wire->status = HAL_OK;
  if (WIRE_TIM_APB_CLK < 200) return wire->status = HAL_ERROR; // Time source frequency too low.
  __HAL_TIM_DISABLE(wire->TIM);
  wire->TIM->Instance->PSC = WIRE_TIM_APB_CLK / 100 - 1;
#if WIRE_USE_IT
  __HAL_TIM_DISABLE_IT(wire->TIM, TIM_IT_UPDATE);
  set_tim_binding(wire);
  HAL_TIM_RegisterCallback(wire->TIM, HAL_TIM_PERIOD_ELAPSED_CB_ID, tim_state_machine);
  __HAL_TIM_ENABLE_IT(wire->TIM, TIM_IT_UPDATE);
#endif
  wire_tim_calibrate(wire);
  return wire->status;
}

/**
 * 1-Wire TIM transport layer driver.
 */
Wire_DriverTypeDef wire_tim_driver =
{
    .init = tim_init,
#if WIRE_USE_POLLING
    .reset = tim_reset,
    .bit_rx = tim_bit_rx,
    .bit_tx = tim_bit_tx,
#endif // WIRE_USE_POLLING
#if WIRE_USE_IT
    .reset_it = tim_reset_it,
    .bit_rx_it = tim_bit_rx_it,
    .bit_tx_it = tim_bit_tx_it
#endif // WIRE_USE_IT
};

#endif // WIRE_USE_TIM
