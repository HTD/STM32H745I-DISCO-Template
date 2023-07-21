/**
 * @file      1wire.c
 * @author    CodeDog
 * @brief     1-Wire protocol.
 *
 * @copyright	(c)2022 CodeDog, All rights reserved.
 */

#include "1wire.h"

#if 1 || WIRE_USE_TIM
#include "1wire_tim.h"
#endif
#if WIRE_USE_UART
#include "1wire_uart.h"
#endif

/**
 * 1-Wire reserved handles.
 */
Wire_HandleTypeDef* wire_handles[WIRE_HCOUNT];

/**
 * @fn HAL_StatusTypeDef wire_init(Wire_HandleTypeDef*, void*)
 * @brief Initializes a 1-Wire binding for a device.
 * @param wire 1-Wire handle pointer.
 * @param hdev A pointer to the device handle.
 * @return HAL status.
 */
HAL_StatusTypeDef wire_init(Wire_HandleTypeDef* wire, void* hdev)
{
  wire_register_handle(wire);
#if WIRE_USE_TIM
  if (wire->TIM) wire->driver = &wire_tim_driver;
#endif // WIRE_USE_TIM
#if WIRE_USE_UART
  if (wire->UART && !wire->driver) wire->driver = &wire_uart_driver;
#endif // WIRE_USE_UART
  if (!wire->driver || !wire->driver->init) return HAL_ERROR; // Invalid configuration.
  wire->hdev = hdev;
  return wire->status = wire->driver->init(wire);
}

/**
 * @fn HAL_StatusTypeDef wire_register_handle(Wire_HandleTypeDef*)
 * @brief Registers the handle pointer.
 * @param wire 1-Wire handle pointer.
 * @return HAL_OK: success, HAL_ERROR: already registered or out of pointers.
 */
HAL_StatusTypeDef wire_register_handle(Wire_HandleTypeDef* wire)
{
  for (uint8_t i = 0; i < WIRE_HCOUNT; i++)
  {
    if (wire_handles[i] == wire) return HAL_OK; // Already initialized.
    if (!wire_handles[i])
    {
      wire_handles[i] = wire;
      return HAL_OK;
    }
  }
  return HAL_ERROR; // Out of pointers.
}

#if WIRE_USE_POLLING

/**
 * @fn HAL_StatusTypeDef wire_reset(Wire_HandleTypeDef*)
 * @brief Sends the reset signal in polling mode and returns HAL_OK if a valid response is received.
 * @param wire 1-Wire handle pointer.
 * @return HAL status.
 */
HAL_StatusTypeDef wire_reset(Wire_HandleTypeDef* wire)
{
  if (!wire->driver) return HAL_ERROR;
  return wire->driver->reset(wire);
}

/**
 * @fn HAL_StatusTypeDef wire_rx(Wire_HandleTypeDef*)
 * @brief Receives data to the current buffer over the 1-Wire in polling mode.
 * @param wire 1-Wire handle pointer.
 * @return HAL status.
 */
HAL_StatusTypeDef wire_rx(Wire_HandleTypeDef* wire)
{
  if (!wire->buffer.ptr || !wire->driver) return HAL_ERROR;
  HAL_StatusTypeDef status;
  uint8_t rx, tx = 0xff, target;
  for (uint16_t offset = 0; offset < wire->buffer.size; offset++)
  {
    target = 0;
    for (uint8_t bit_index = 0; bit_index < 8; bit_index++)
    {
      status = wire->driver->bit_rx(wire, &rx);
      if (status != HAL_OK) return status;
      target |= (rx & 1) << bit_index;
    }
    ((uint8_t*)wire->buffer.ptr)[offset] = target;
  }
  return status;
}

/**
 * @fn HAL_StatusTypeDef wire_tx(Wire_HandleTypeDef*)
 * @brief Sends data from the current buffer over the 1-Wire in polling mode.
 * @param wire 1-Wire handle pointer.
 * @return HAL status.
 */
HAL_StatusTypeDef wire_tx(Wire_HandleTypeDef* wire)
{
  if (!wire->buffer.ptr || !wire->driver) return HAL_ERROR;
  HAL_StatusTypeDef status;
  uint8_t tx;
  for (uint16_t offset = 0; offset < wire->buffer.size; offset++)
    for (uint8_t bit_index = 0; bit_index < 8; bit_index++)
    {
      tx = (((uint8_t*) wire->buffer.ptr)[offset] >> bit_index & 1) != 0;
      status = wire->driver->bit_tx(wire, &tx);
      if (status != HAL_OK) return status;
    }
  return status;
}

#endif

#if WIRE_USE_IT

/**
 * @fn void complete_transaction(Wire_HandleTypeDef*)
 * @brief Completes 1-Wire transaction and call registered callback if available.
 * @param wire 1-Wire handle pointer.
 */
static void complete_transaction(Wire_HandleTypeDef* wire)
{
  wire->wd = 0;
  if (wire->callback) wire->callback(wire);
}

/**
 * @fn void check_timeout(Wire_HandleTypeDef*)
 * @brief Checks for timeouts.
 * @param wire 1-Wire handle pointer.
 */
static void check_timeout(Wire_HandleTypeDef* wire)
{
  if (!wire->wd) return;
  wire->state = WIRE_S_ERROR;
  wire->status = HAL_TIMEOUT;
  if (wire->callback) wire->callback(wire);
}

/**
 * @fn void wire_state_machine(Wire_HandleTypeDef*)
 * @brief State machine for 1-Wire interrupt mode callbacks.
 * @param wire 1-Wire handle pointer.
 */
void wire_state_machine(Wire_HandleTypeDef* wire)
{
  next_state:
  switch (wire->state)
  {
  case WIRE_S_INIT:
  case WIRE_S_READY:
    break; // Calls ignored.
  case WIRE_S_RESET:
    wire->state = wire->status == HAL_OK ? WIRE_S_CPLT : WIRE_S_ERROR;
    goto next_state;
  case WIRE_S_BIT_TX:
    if (wire->buffer.bit_index < 8) // Current byte:
    {
      send_bit:
      wire->txd = (wire->buffer.current_byte >> wire->buffer.bit_index & 1) != 0;
      wire->buffer.bit_index++;
      wire->status = wire->driver->bit_tx_it(wire, &wire->txd);
      if (wire->status != HAL_OK) { wire->state = WIRE_S_ERROR; goto next_state; }
      return; // TX next bit.
    }
    if (++wire->buffer.offset < wire->buffer.size) // Next byte:
    {
      wire->buffer.current_byte = ((uint8_t*)wire->buffer.ptr)[wire->buffer.offset];
      wire->buffer.bit_index = 0;
      goto send_bit; // Send bit 0.
    }
    // All sent.
    wire->state = WIRE_S_CPLT; goto next_state;
    break;
  case WIRE_S_BIT_RX:
    if (wire->buffer.bit_index < 8) // Current byte:
    {
      if (wire->buffer.bit_index == 0) ((uint8_t*) wire->buffer.ptr)[wire->buffer.offset] = 0; // Reset byte on bit 0.
      ((uint8_t*)wire->buffer.ptr)[wire->buffer.offset] |= ((wire->rxd & 1) << wire->buffer.bit_index);
      if (++wire->buffer.bit_index < 8)
      {
        // Reveive next bit of the current byte.
        if (wire->driver->bit_rx_it(wire, &wire->rxd) != HAL_OK) { wire->state = WIRE_S_ERROR; goto next_state; }
        return;
      }
    }
    if (++wire->buffer.offset < wire->buffer.size) // Next byte:
    {
      wire->buffer.bit_index = 0; // Recieve bit 0 of next byte.
      if (wire->driver->bit_rx_it(wire, &wire->rxd) != HAL_OK) { wire->state = WIRE_S_ERROR; goto next_state; }
      return;
    }
    // All received.
    wire->state = WIRE_S_CPLT;
  case WIRE_S_CPLT:
    wire->state = WIRE_S_READY;
    complete_transaction(wire);
    break;
  case WIRE_S_ERROR:
    complete_transaction(wire);
    break;
  }
}

/**
 * @fn void set_watchdog(Wire_HandleTypeDef*, double)
 * @brief Sets a watchdog that is cleared on complete_transaction().
 * @param wire 1-Wire handle pointer.
 * @param timeout Timeout in milliseconds.
 */
static inline void set_watchdog(Wire_HandleTypeDef* wire, double timeout)
{
  wire->wd = 1;
  delay_async(timeout, wire, (TIM_EX_CallbackTypeDef)check_timeout);
}

/**
 * @fn HAL_StatusTypeDef wire_reset_it(Wire_HandleTypeDef*, Wire_CallbackTypeDef)
 * @brief Sends the 1-Wire reset pulse in interrupt mode.
 * @param wire 1-Wire handle pointer.
 * @param callback A function to be called after the reset handshake completes.
 * @return HAL status.
 */
HAL_StatusTypeDef wire_reset_it(Wire_HandleTypeDef* wire, Wire_CallbackTypeDef callback)
{
  wire->state = WIRE_S_RESET;
  wire->callback = callback;
  wire->status = wire->driver->reset_it(wire);
  if (wire->status != HAL_OK) return wire->status;
  set_watchdog(wire, WIRE_TIMEOUT);
  return wire->status;
}

/**
 * @fn HAL_StatusTypeDef wire_rx_it(Wire_HandleTypeDef*, Wire_CallbackTypeDef)
 * @brief Receives data to the current buffer over the 1-Wire in interrupt mode.
 * @param wire 1-Wire handle pointer.
 * @param callback A function to be called when the transmission completes.
 * @return HAL status.
 */
HAL_StatusTypeDef wire_rx_it(Wire_HandleTypeDef* wire, Wire_CallbackTypeDef callback)
{
  wire->status = wire->state == WIRE_S_READY && wire->buffer.ptr ? HAL_OK : HAL_ERROR;
  if (wire->status != HAL_OK) return wire->status;
  wire->callback = callback;
  wire->buffer.bit_index = 0;
  wire->buffer.offset = 0;
  wire->state = WIRE_S_BIT_RX;
  wire->status = wire->driver->bit_rx_it(wire, &wire->rxd);
  if (wire->status != HAL_OK) return wire->status;
  set_watchdog(wire, WIRE_TIMEOUT);
  return wire->status;
}

/**
 * @fn HAL_StatusTypeDef wire_tx_it(Wire_HandleTypeDef*, Wire_CallbackTypeDef)
 * @brief Sends data from the current buffer over the 1-Wire in interrupt mode.
 * @param wire 1-Wire handle pointer.
 * @param callback A function to be called when the transmission completes.
 * @return HAL status.
 */
HAL_StatusTypeDef wire_tx_it(Wire_HandleTypeDef* wire, Wire_CallbackTypeDef callback)
{
  wire->status = wire->state == WIRE_S_READY && wire->buffer.ptr ? HAL_OK : HAL_ERROR;
  if (wire->status != HAL_OK) return wire->status;
  wire->callback = callback;
  wire->buffer.bit_index = 0;
  wire->buffer.offset = 0;
  wire->buffer.current_byte = ((uint8_t*) wire->buffer.ptr)[wire->buffer.offset];
  wire->txd = (wire->buffer.current_byte >> wire->buffer.bit_index & 1) != 0;
  wire->buffer.bit_index++;
  wire->state = WIRE_S_BIT_TX;
  wire->status = wire->driver->bit_tx_it(wire, &wire->txd);
  if (wire->status != HAL_OK) return wire->status;
  set_watchdog(wire, WIRE_TIMEOUT);
  return wire->status;
}

#endif

/**
 * @fn uint8_t wire_crc(const void*, uint16_t)
 * @brief Calculates the byte CRC for the current buffer.
 * @param wire 1-Wire handle pointer.
 * @return HAL_OK if last byte in the buffer contains valid checksum, HAL_ERROR otherwise.
 */
HAL_StatusTypeDef wire_crc(Wire_HandleTypeDef* wire)
{
  if (!wire->buffer.ptr || wire->buffer.size < 1) return HAL_ERROR;
  uint8_t crc = 0;
  for (uint16_t byte_index = 0; byte_index < wire->buffer.size - 1; byte_index++)
  {
    uint8_t byte = ((uint8_t*) wire->buffer.ptr)[byte_index];
    for (uint8_t bit_index = 0; bit_index < 8; bit_index++)
    {
      uint8_t xor = crc ^ byte;
      crc >>= 1;
      if (xor & 0x01)
        crc ^= 0x8c;
      byte >>= 1;
    }
  }
  return ((uint8_t*) wire->buffer.ptr)[wire->buffer.size - 1] == crc ? HAL_OK : HAL_ERROR;
}

/**
 * @fn void wire_strong_pullup(Wire_HandleTypeDef*, uint8_t)
 * @brief Sets the strong pullup for the 1-wire parasite power supply.
 * @param wire 1-Wire handle pointer.
 * @param pullup Requested state: 1 - power, 0 - data.
 */
void wire_set_pullup(Wire_HandleTypeDef* wire, Wire_PullUpTypeDef pullup)
{
  if (!wire->GPIO || !wire->GPIO->Port) return; // parasitic power supply not enabled.
  HAL_EX_GPIO_set_OT(wire->GPIO, pullup == WIRE_P_STRONG ? GPIO_EX_OT_PP : GPIO_EX_OT_OD);
}
