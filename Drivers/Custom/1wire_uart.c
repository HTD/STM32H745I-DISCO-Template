/**
 * @file      wire.c
 * @author    CodeDog
 * @brief     1-Wire UART transport layer driver.
 * @remarks
 *            Uses HAL UART configured as Single Wire Half Duplex.
 *
 * @copyright (c)2022 CodeDog, All rights reserved.
 */

#if WIRE_USE_UART

#include "1wire_uart.h"

/**
 * @fn HAL_StatusTypeDef set_baudrate(UART_HandleTypeDef*, uint32_t)
 * @brief Sets the baud rate for the UART used for 1-Wire communication.
 * @param wire 1-Wire handle pointer.
 * @param baudrate The UART baud rate in bits per second.
 * @return HAL status.
 */
static HAL_StatusTypeDef set_baudrate(Wire_HandleTypeDef* wire, uint32_t baudrate)
{
  wire->UART->Init.BaudRate = baudrate;
  return HAL_HalfDuplex_Init(wire->UART);
}

#if WIRE_USE_POLLING

/**
 * @fn HAL_StatusTypeDef wire_reset(Wire_HandleTypeDef*)
 * @brief Sends and receives the RESET pulse to initialize the communication.
 * @param wire 1-Wire handle pointer.
 * @return HAL status.
 */
static HAL_StatusTypeDef uart_reset(Wire_HandleTypeDef* wire)
{
  uint8_t tx = WIRE_UART_RESET_REQUEST;
  uint8_t rx = 0x00;
  HAL_StatusTypeDef status;
  status = set_baudrate(wire, WIRE_BAUDRATE_RESET);
  if (status != HAL_OK)
    return status;
  status = HAL_UART_Transmit(wire->UART, &tx, 1, WIRE_TIMEOUT);
  if (status != HAL_OK)
    return status;
  status = HAL_UART_Receive(wire->UART, &rx, 1, WIRE_TIMEOUT);
  if (status != HAL_OK)
    return status;
  status = set_baudrate(wire, WIRE_BAUDRATE_DATA);
  if (status != HAL_OK)
    return status;
  return rx >= 0x70 ? status : HAL_ERROR;
}

/**
 * @fn HAL_StatusTypeDef uart_bit_rx(Wire_HandleTypeDef*, uint8_t*)
 * @brief Receives a single bit in polling mode.
 * @param wire 1-Wire handle pointer.
 * @param target A pointer to the 8-bit interger that will receive the value read.
 * @return HAL status.
 */
static HAL_StatusTypeDef uart_bit_rx(Wire_HandleTypeDef* wire, uint8_t* target)
{
  return HAL_UART_Receive(wire->UART, target, 1, WIRE_TIMEOUT);
}

/**
 * @fn HAL_StatusTypeDef uart_bit_tx(Wire_HandleTypeDef*, uint8_t*)
 * @brief Sends a single bit in polling mode.
 * @param wire 1-Wire handle pointer.
 * @param target A pointer to the 8-bit integer that has the value to send.
 * @return HAL status.
 */
static HAL_StatusTypeDef uart_bit_tx(Wire_HandleTypeDef* wire, uint8_t* target)
{
  return HAL_UART_Transmit(wire->UART, target, 1, WIRE_TIMEOUT);
}

#endif // WIRE_USE_POLLING

#if WIRE_USE_IT

/**
 * @fn Wire_HandleTypeDef get_handle_by_uart*(UART_HandleTypeDef*)
 * @brief Gets the 1-Wire handle pointer for UART handle pointer.
 * @param huart UART handle pointer.
 * @return 1-Wire handle pointer if initialized, NULL otherwise.
 */
static Wire_HandleTypeDef* get_handle_by_uart(UART_HandleTypeDef* huart)
{
  for (uint8_t i = 0; i < WIRE_HCOUNT; i++)
    if (wire_handles[i] && wire_handles[i]->UART == huart)
      return wire_handles[i];
  return 0;
}

/**
 * @fn void uart_state_machine(UART_HandleTypeDef*)
 * @brief Handles callbacks from UART interrupt.
 * @param huart UART handle pointer.
 */
static void uart_state_machine(UART_HandleTypeDef* huart)
{
  Wire_HandleTypeDef* wire = get_handle_by_uart(huart);
  if (!wire) return;
  if (wire->state == WIRE_S_RESET_RX)
  {
    if (wire->rxd < 0x70) wire->status = HAL_ERROR;
    wire->state = wire->status == HAL_OK ? WIRE_S_READY : WIRE_S_INIT;
    if (wire->status == HAL_OK) wire->status = set_baudrate(wire, WIRE_BAUDRATE_DATA);
  }
  wire_state_machine(wire);
}

/**
 * @fn HAL_StatusTypeDef uart_reset_it(Wire_HandleTypeDef*)
 * @brief Sends the reset signal in interrupt mode.
 * @param wire UART handle pointer.
 * @return HAL status.
 */
static HAL_StatusTypeDef uart_reset_it(Wire_HandleTypeDef* wire)
{
  wire->status = set_baudrate(wire, WIRE_BAUDRATE_RESET);
  if (wire->status != HAL_OK) return wire->status;
  wire->txd = WIRE_UART_RESET_REQUEST;
  wire->status = HAL_UART_Transmit_IT(wire->UART, &wire->txd, 1);
  return wire->status;
}

/**
 * @fn HAL_StatusTypeDef uart_bit_rx_it(Wire_HandleTypeDef*, uint8_t*)
 * @brief Receives a single bit in interrupt mode.
 * @param wire 1-Wire handle pointer.
 * @param target A pointer to the 8-bit interger that will receive the value read.
 * @return HAL status.
 */
static HAL_StatusTypeDef uart_bit_rx_it(Wire_HandleTypeDef* wire, uint8_t* target)
{
  return HAL_UART_Receive_IT(wire->UART, target, 1);
}

/**
 * @fn HAL_StatusTypeDef uart_bit_tx_it(Wire_HandleTypeDef*, uint8_t*)
 * @brief Sends a single bit in interrupt mode.
 * @param wire 1-Wire handle pointer.
 * @param target A pointer to the 8-bit integer that has the value to send.
 * @return HAL status.
 */
static HAL_StatusTypeDef uart_bit_tx_it(Wire_HandleTypeDef* wire, uint8_t* target)
{
  return HAL_UART_Transmit_IT(wire->UART, target, 1);
}

#endif // WIRE_USE_IT

/**
 * @fn HAL_StatusTypeDef wire_init(Wire_HandleTypeDef*, void*)
 * @brief Initializes a 1-Wire binding for a device.
 * @param wire 1-Wire handle pointer.
 * @param hdev A pointer to the device handle.
 * @return HAL status, error if already initialized, not enough handles reserved or UART initialization failed.
 */
static HAL_StatusTypeDef uart_init(Wire_HandleTypeDef* wire)
{
  if (!wire || !wire->UART) return HAL_ERROR;
  wire->status = wire_register_handle(wire);
  if (wire->status != HAL_OK) return wire->status;
  wire->UART->Init.BaudRate = WIRE_BAUDRATE_RESET;
  wire->UART->Init.WordLength = UART_WORDLENGTH_8B;
  wire->UART->Init.StopBits = UART_STOPBITS_1;
  wire->UART->Init.Parity = UART_PARITY_NONE;
  wire->UART->Init.Mode = UART_MODE_TX_RX;
  wire->UART->Init.HwFlowCtl = UART_HWCONTROL_NONE;
  wire->UART->Init.OverSampling = UART_OVERSAMPLING_16;
  wire->UART->Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  wire->UART->AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_RXOVERRUNDISABLE_INIT;
  wire->UART->AdvancedInit.OverrunDisable = UART_ADVFEATURE_OVERRUN_DISABLE;
  wire->status = HAL_HalfDuplex_Init(wire->UART);
  if (wire->status != HAL_OK) return wire->status;
#if WIRE_USE_IT
  wire->status =
      HAL_UART_RegisterCallback(wire->UART, HAL_UART_TX_COMPLETE_CB_ID, uart_state_machine);
  if (wire->status != HAL_OK) return wire->state;
  wire->status =
      HAL_UART_RegisterCallback(wire->UART, HAL_UART_RX_COMPLETE_CB_ID, uart_state_machine);
  if (wire->status != HAL_OK) return wire->state;
#endif // WIRE_USE_IT
  return wire->status;
}

/**
 * 1-Wire UART transport layer driver.
 */
Wire_DriverTypeDef wire_uart_driver =
{
    .init = uart_init,
#if WIRE_USE_POLLING
    .reset = uart_reset,
    .bit_rx = uart_bit_rx,
    .bit_tx = uart_bit_tx,
#endif // WIRE_USE_POLLING
#if WIRE_USE_IT
    .reset_it = uart_reset_it,
    .bit_rx_it = uart_bit_rx_it,
    .bit_tx_it = uart_bit_tx_it
#endif // WIRE_USE_IT
};

#endif // WIRE_USE_UART
