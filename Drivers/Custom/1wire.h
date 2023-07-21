/**
 * @file      wire.h
 * @author    CodeDog
 * @brief     1-Wire protocol header file.
 *
 * @copyright (c)2022 CodeDog, All rights reserved.
 */

#pragma once

#include "hal_ex.h"
#include "tim_ex.h"

#define WIRE_USE_UART       0   ///< 1-Wire through UART configured as single wire.
#define WIRE_USE_TIM        1   ///< 1-Wire using TIM interrupt for timing.

#define WIRE_USE_POLLING    0   ///< Use polling mode driver.
#define WIRE_USE_IT         1   ///< Use interrupt mode driver.

#define WIRE_HCOUNT         1   ///> The number of 1-Wire handles reserved.
#define WIRE_TIMEOUT        100 ///> Timeout value for 1-wire operation.

/**
 * @typedef Wire_PullUpTypeDef
 * @brief 1-Wire pullup mode enumeration.
 */
typedef enum
{
  WIRE_P_WEAK,  ///> Weak pullup provided by external resistor.
  WIRE_P_STRONG ///> Strong pullup from the TX pin push-pull output.
} Wire_PullUpTypeDef;

/**
 * @typedef Wire_BufferTypeDef
 * @brief 1-Wire buffer struct.
 */
typedef struct
{
  void* ptr;            ///> Data pointer.
  uint16_t size;        ///> Data size.
  uint16_t offset;      ///> The index of the current byte.
  uint8_t bit_index;    ///> The index of the current bit in the current byte.
  uint8_t current_byte; ///> The current byte being processed.
} Wire_BufferTypeDef;

typedef struct __Wire_HandleTypeDef Wire_HandleTypeDef;

typedef HAL_StatusTypeDef (*Wire_InitFunctionTypeDef)(Wire_HandleTypeDef*);
typedef HAL_StatusTypeDef (*Wire_FunctionTypeDef)(Wire_HandleTypeDef*);
typedef HAL_StatusTypeDef (*Wire_DataBitFunctionTypeDef)(Wire_HandleTypeDef*, uint8_t*);

/**
 * @typedef Wire_DriverTypeDef
 * @brief 1-Wire transport layer definition.
 * @struct
 */
typedef struct
{
  Wire_InitFunctionTypeDef init;          ///< Initializes the driver.
#if WIRE_USE_POLLING
  Wire_FunctionTypeDef reset;             ///< Sends the RESET signal.
  Wire_DataBitFunctionTypeDef bit_rx;     ///< Receives a bit.
  Wire_DataBitFunctionTypeDef bit_tx;     ///< Sends a bit.
#endif
#if WIRE_USE_IT
  Wire_FunctionTypeDef reset_it;          ///< Sends the RESET signal.
  Wire_DataBitFunctionTypeDef bit_rx_it;  ///< Receives a bit.
  Wire_DataBitFunctionTypeDef bit_tx_it;  ///< Sends a bit.
#endif
} Wire_DriverTypeDef;

#if WIRE_USE_IT

/**
 * @typedef Wire_StateTypeDef
 * @brief 1-Wire state enumeration.
 */
typedef enum
{
  WIRE_S_INIT,      ///> The 1-Wire is in intial state.
  WIRE_S_RESET,     ///> Wire reset pulse returned status.
  WIRE_S_READY,     ///> Wire ready for communication.
  WIRE_S_BIT_TX,    ///> Wire bit is sent.
  WIRE_S_BIT_RX,    ///> Wire bit is received.
  WIRE_S_CPLT,      ///> Wire transaction complete.
  WIRE_S_ERROR      ///> Wire transaction failed.

} Wire_StateTypeDef;

typedef void (*Wire_CallbackTypeDef)(Wire_HandleTypeDef*);

#endif

/**
 * @typedef Wire_HandleTypeDef
 * @brief 1-Wire line handle.
 */
typedef struct __Wire_HandleTypeDef
{
#if WIRE_USE_UART
  UART_HandleTypeDef* UART;         ///> UART configured as Single Wire Half Duplex.
#endif
#if WIRE_USE_TIM
  TIM_HandleTypeDef* TIM;           ///> TIM configured to provide 1MHz timing after prescaller.
#endif
  GPIO_PinTypeDef* GPIO;            ///> UART TX port, if defined it will be used to provide power in parasite mode.
  uint8_t is_powered;               ///> 1 - device is powered / doesn't need strong pullup, 0 - use parasite mode.
  uint8_t txd;                      ///> The byte being transmitted.
  uint8_t rxd;                      ///> The byte being received.
  uint8_t wd;                       ///> Watchdog - if set to 1 and time elapses it should cause timeout error.
  HAL_StatusTypeDef status;         ///> HAL status of the last I/O operation.
  Wire_BufferTypeDef buffer;        ///> Current data buffer.
  void* hdev;                       ///> Optional peripheral handle if available.
  Wire_DriverTypeDef* driver;       ///> Transport layer driver.
#if WIRE_USE_IT
  Wire_StateTypeDef state;          ///> Current 1-Wire communication state.
  Wire_CallbackTypeDef callback;    ///> A callback for a completed operation.
#endif
} Wire_HandleTypeDef;

extern Wire_HandleTypeDef* wire_handles[WIRE_HCOUNT]; ///> 1-Wire handle pointers.

// API calls

HAL_StatusTypeDef wire_init(Wire_HandleTypeDef* wire, void* hdev);
HAL_StatusTypeDef wire_register_handle(Wire_HandleTypeDef* wire);
void wire_state_machine(Wire_HandleTypeDef* wire);

#if WIRE_USE_POLLING
HAL_StatusTypeDef wire_reset(Wire_HandleTypeDef* wire);
HAL_StatusTypeDef wire_rx(Wire_HandleTypeDef* wire);
HAL_StatusTypeDef wire_tx(Wire_HandleTypeDef* wire);
#endif

#if WIRE_USE_IT
HAL_StatusTypeDef wire_reset_it(Wire_HandleTypeDef* wire, Wire_CallbackTypeDef callback);
HAL_StatusTypeDef wire_rx_it(Wire_HandleTypeDef* wire, Wire_CallbackTypeDef callback);
HAL_StatusTypeDef wire_tx_it(Wire_HandleTypeDef* wire, Wire_CallbackTypeDef callback);
#endif

HAL_StatusTypeDef wire_crc(Wire_HandleTypeDef* wire);
void wire_set_pullup(Wire_HandleTypeDef* wire, Wire_PullUpTypeDef pullup);
