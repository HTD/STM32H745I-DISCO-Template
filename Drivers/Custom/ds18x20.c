/**
 * @file      ds18x20.c
 * @author    CodeDog
 * @brief     Digital thermometer driver for DS1820, DS18S20 and DS18B20.
 *
 * @see       https://pdf1.alldatasheet.pl/pdfjsview/web/viewer.html?file=//pdf1.alldatasheet.pl/datasheet-pdf/view/58548/DALLAS/DS1820/+_7WW9UzYIKCTchVBEvN+/datasheet.pdf
 *            https://datasheets.maximintegrated.com/en/ds/DS18S20.pdf
 *            https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf
 *            https://www.maximintegrated.com/en/design/technical-documents/app-notes/4/4377.html
 *
 * @copyright (c)2022 CodeDog, All rights reserved.
 */

#include "ds18x20.h"

#ifdef HAL_UART_MODULE_ENABLED

#if WIRE_USE_IT

#pragma region Forward declarations

static void s1_callback(Wire_HandleTypeDef* wire);
static void s2_callback(Wire_HandleTypeDef* wire);
static void s2_s1_callback(DS18X20_HandleTypeDef* hdev);
static void c1_callback(DS18X20_HandleTypeDef* hdev);
static void c2_callback(DS18X20_HandleTypeDef* hdev);

#pragma endregion

#endif

#pragma region start Private functions

/**
 * @fn void set_cmd(DS18X20_HandleTypeDef*, DS18X20_CommandTypeDef*)
 * @brief Sets the 1-Wire buffer to a command byte.
 * @param hdev Device handle pointer.
 * @param cmd Command byte.
 */
static inline void set_cmd(DS18X20_HandleTypeDef* hdev, DS18X20_CommandTypeDef* cmd)
{
  hdev->wire->buffer.ptr = cmd;
  hdev->wire->buffer.size = 1;
}

#if WIRE_USE_IT

/**
 * @fn HAL_StatusTypeDef s1_command(DS18X20_HandleTypeDef*, DS18X20_CommandTypeDef, DS18X20_CallbackTypeDef)
 * @brief Sends a DS18X20 ROM command.
 * @param hdev Device handle pointer.
 * @param cmd Command byte.
 * @param callback A function to call when the ROM function completes.
 * @return HAL status.
 */
static HAL_StatusTypeDef s1_command(DS18X20_HandleTypeDef* hdev, DS18X20_CommandTypeDef cmd, DS18X20_CallbackTypeDef callback)
{
  hdev->async_state.S1_callback = callback;
  hdev->async_state.S1_cmd = cmd;
  hdev->async_state.S1 = DS18X20_S1_RESET;
  return wire_reset_it(hdev->wire, s1_callback);
}

/**
 * @fn HAL_StatusTypeDef s2_command(DS18X20_HandleTypeDef*, DS18X20_CommandTypeDef, DS18X20_CallbackTypeDef)
 * @brief Sends a DS18X20 function command.
 * @param hdev Device handle pointer.
 * @param cmd Command byte.
 * @param callback A function to call when function command completes.
 * @return HAL status.
 */
static HAL_StatusTypeDef s2_command(DS18X20_HandleTypeDef* hdev, DS18X20_CommandTypeDef cmd, DS18X20_CallbackTypeDef callback)
{
  hdev->async_state.S2_callback = callback;
  hdev->async_state.S2_cmd = cmd;
  hdev->async_state.S2 = DS18X20_S2_COMMAND_TX;
  return s1_command(hdev, hdev->match_rom ? DS18X20_CMD_MATCH_ROM : DS18X20_CMD_SKIP_ROM, s2_s1_callback);
}

/**
 * @fn HAL_StatusTypeDef c1_start(DS18X20_HandleTypeDef*, DS18X20_CallbackTypeDef)
 * @brief Initializes a DS18X20 chip by reading its ROM code, reading the scratchpad and basic settings.
 * @param hdev Device handle pointer.
 * @param callback A function to call when the initialization completes.
 * @return HAL status.
 */
static HAL_StatusTypeDef c1_start(DS18X20_HandleTypeDef* hdev, DS18X20_CallbackTypeDef callback)
{
  hdev->async_state.callback = callback;
  hdev->async_state.C1 = DS18X20_C1_READ_ROM;
  return s1_command(hdev, DS18X20_CMD_READ_ROM, c1_callback);
}

/**
 * @fn HAL_StatusTypeDef c2_start(DS18X20_HandleTypeDef*, DS18X20_CallbackTypeDef)
 * @brief Starts the temperature measurement.
 * @param hdev Device handle pointer.
 * @param callback A function to call when the measurement is done.
 * @return HAL status.
 */
static HAL_StatusTypeDef c2_start(DS18X20_HandleTypeDef* hdev, DS18X20_CallbackTypeDef callback)
{
  hdev->async_state.callback = callback;
  hdev->async_state.C2 = DS18X20_C2_CONVERT_T;
  return s2_command(hdev, DS18X20_CMD_CONVERT_T, c2_callback);
}

#endif // WIRE_USE_IT

#if WIRE_USE_POLLING

static HAL_StatusTypeDef send_byte(DS18X20_HandleTypeDef* hdev, uint8_t byte)
{
  hdev->wire->buffer.ptr = &byte;
  hdev->wire->buffer.size = 1;
  return wire_tx(hdev->wire);
}

/**
 * @fn HAL_StatusTypeDef read_rom(DS18X20_handle*)
 * @brief Reads the unique DS18X20 ROM code in polling mode.
 * @param hdev Device handle pointer.
 * @return HAL status.
 */
static HAL_StatusTypeDef read_rom(DS18X20_HandleTypeDef* hdev)
{
//  uint8_t cmd;
  HAL_StatusTypeDef status;
  status = wire_reset(hdev->wire);
  if (status != HAL_OK) return status;
  send_byte(hdev, DS18X20_CMD_READ_ROM);
//  cmd = DS18X20_CMD_READ_ROM;
//  hdev->wire->buffer.ptr = &cmd;
//  hdev->wire->buffer.size = 1;
//  status = wire_tx(hdev->wire);
  if (status != HAL_OK) return status;
  hdev->wire->buffer.ptr = &hdev->rom_code;
  hdev->wire->buffer.size = DS18X20_ROM_CODE_SIZE;
  status = wire_rx(hdev->wire);
  if (status != HAL_OK) return status;
  hdev->is_ds18b20 = hdev->rom_code[0] == DS18B20_ID;
  return wire_crc(hdev->wire);
}

/**
 * @fn HAL_StatusTypeDef send_cmd(DS18X20_handle*, uint8_t)
 * @brief Sends a command in polling mode.
 * @param hdev Device handle pointer.
 * @param cmd Command to send.
 * @return HAL status.
 */
static HAL_StatusTypeDef send_cmd(DS18X20_HandleTypeDef* hdev, DS18X20_CommandTypeDef cmd)
{
  HAL_StatusTypeDef status;
  status = wire_reset(hdev->wire);
  if (status != HAL_OK) return status;
  if (!hdev->match_rom)
  {
    status = send_byte(hdev, DS18X20_CMD_SKIP_ROM);
    if (status != HAL_OK) return status;
  }
  else
  {
    status = send_byte(hdev, DS18X20_CMD_MATCH_ROM);
    if (status != HAL_OK) return status;
    hdev->wire->buffer.ptr = &hdev->rom_code;
    hdev->wire->buffer.size = DS18X20_ROM_CODE_SIZE;
    status = wire_tx(hdev->wire);
    if (status != HAL_OK) return status;
  }
  return send_byte(hdev, cmd);
}

/**
 * @fn HAL_StatusTypeDef read_scratchpad(DS18X20_handle*)
 * @brief Reads the current scratchpad content in polling mode.
 * @param hdev Device handle pointer.
 * @return HAL status.
 */
static HAL_StatusTypeDef read_scratchpad(DS18X20_HandleTypeDef* hdev)
{
  HAL_StatusTypeDef status;
  status = send_cmd(hdev, DS18X20_CMD_READ_SCRATCHPAD);
  if (status != HAL_OK) return status;
  hdev->wire->buffer.ptr = &hdev->scratchpad;
  hdev->wire->buffer.size = DS18X20_SCRATCHPAD_SIZE;
  status = wire_rx(hdev->wire);
  if (status != HAL_OK) return status;
  return wire_crc(hdev->wire);
}

/**
 * @fn HAL_StatusTypeDef write_scratchpad(DS18X20_handle*)
 * @brief Writes the current scratchpad conten in polling mode..
 * @param hdev Device handle pointer.
 * @return HAL status.
 */
static HAL_StatusTypeDef write_scratchpad(DS18X20_HandleTypeDef* hdev)
{
  HAL_StatusTypeDef status;
  status = send_cmd(hdev, DS18X20_CMD_WRITE_SCRATCHPAD);
  if (status != HAL_OK) return status;
  hdev->wire->buffer.ptr = &hdev->scratchpad[DS18X20_OFFSET_USER_BYTE1];
  hdev->wire->buffer.size = 3;
  status = wire_tx(hdev->wire);
  if (status != HAL_OK) return status;
  return wire_reset(hdev->wire);
}

#endif // WIRE_USE_POLLING

/**
 * @fn uint8_t get_resolution(DS18X20_handle*)
 * @brief Gets the bit resolution from the LOCAL scratchpad.
 * @param hdev Device handle pointer.
 * @return Resolution.
 */
static uint8_t get_resolution(DS18X20_HandleTypeDef* hdev) // call after scratchpad is read.
{
  return hdev->is_ds18b20
         ? ((hdev->scratchpad[DS18X20_OFFSET_CONFIGURATION]
           & ~DS18X20_RESOLUTION_MASK)
           >> DS18X20_RESOLUTION_R0_SHIFT) + DS18B20_RESOLUTION_MIN
         :
         DS18X20_RESOLUTION;
}

/**
 * @fn void set_resolution(DS18X20_handle*)
 * @brief Writes the new resolution to the LOCAL scratchpad.
 * @param hdev Device handle pointer.
 * @param resolution New resolution to set.
 */
static void set_resolution(DS18X20_HandleTypeDef* hdev, uint8_t resolution)
{
  if (!hdev->is_ds18b20) return;
  hdev->scratchpad[DS18X20_OFFSET_CONFIGURATION]
  &= DS18X20_RESOLUTION_MASK;
  hdev->scratchpad[DS18X20_OFFSET_CONFIGURATION]
  |= (hdev->resolution - DS18B20_RESOLUTION_MIN) << DS18X20_RESOLUTION_R0_SHIFT;
}

/**
 * @fn void calculate_accuracy(DS18X20_handle*)
 * @brief Calculates resolution related fields in the DS18X20 handle.
 * @param hdev Device handle pointer.
 */
static void calculate_accuracy(DS18X20_HandleTypeDef* hdev)
{
  uint8_t resolution = get_resolution(hdev);
  hdev->accuracy = DS18X20_T0_ACCURACY;
  hdev->tconv = hdev->is_ds18b20
                ? DS18B20_TCONV_MAX / (1 << (DS18B20_RESOLUTION_MAX - resolution))
                : DS18X20_TCONV_MAX;
  hdev->increment = hdev->is_ds18b20
                    ? DS18B20_INCREMENT_MIN * (1 << (DS18B20_RESOLUTION_MAX - resolution))
                    : DS18X20_INCREMENT_MIN;
  if (hdev->increment > hdev->accuracy) hdev->accuracy = hdev->increment;
  if (hdev->is_ds18b20 && resolution < DS18B20_RESOLUTION_MAX) hdev->tconv++;
}

/**
 * @fn void update_accuracy(DS18X20_handle*)
 * @brief Updates the DS18X20 accuracy for the current reading.
 * @param hdev Device handle pointer.
 */
static void update_accuracy(DS18X20_HandleTypeDef* hdev)
{
  if (hdev->is_ds18b20)
  {
    if (hdev->reading < DS18B20_T0_MIN || hdev->reading > DS18B20_T0_MAX)
      hdev->accuracy = DS18X20_T1_ACCURACY;
    if (hdev->reading < DS18X20_T1_MIN || hdev->reading > DS18X20_T1_MAX)
      hdev->accuracy = DS18X20_T2_ACCURACY;
    if (hdev->increment > hdev->accuracy) hdev->accuracy = hdev->increment;
  }
  else
  {
    if (hdev->reading < DS18X20_T0_MIN || hdev->reading > DS18X20_T0_MAX)
      hdev->accuracy = DS18X20_T2_ACCURACY;
  }
}

#pragma region State machines

#if WIRE_USE_IT

/**
 * @fn void s1_callback(Wire_HandleTypeDef*)
 * @brief DS18X20 ROM state machine.
 * @param wire 1-Wire handle pointer.
 */
static void s1_callback(Wire_HandleTypeDef* wire)
{
  DS18X20_HandleTypeDef* hdev = wire->hdev;
  if (!hdev) return;
  check:
  hdev->async_state.status = wire->status;
  if (hdev->async_state.status != HAL_OK)
  {
    if (hdev->async_state.callback) hdev->async_state.callback(hdev);
    return;
  }
  next_state:
  switch (hdev->async_state.S1)
  {
  case DS18X20_S1_INIT:
    hdev->async_state.S1 = DS18X20_S1_READY;
    goto next_state;
    break;
  case DS18X20_S1_RESET:
    hdev->async_state.S1 = DS18X20_S1_COMMAND_TX;
    set_cmd(hdev, &hdev->async_state.S1_cmd);
    hdev->async_state.status = wire_tx_it(hdev->wire, s1_callback);
    if (hdev->async_state.status != HAL_OK) goto check;
    break;
  case DS18X20_S1_COMMAND_TX:
    switch (hdev->async_state.S1_cmd)
    {
    case DS18X20_CMD_READ_ROM:
      hdev->wire->buffer.ptr = &hdev->rom_code;
      hdev->wire->buffer.size = DS18X20_ROM_CODE_SIZE;
      hdev->async_state.S1 = DS18X20_S1_COMMAND_RX;
      hdev->async_state.status = wire_rx_it(hdev->wire, s1_callback);
      if (hdev->async_state.status != HAL_OK) goto check;
      break;
    case DS18X20_CMD_MATCH_ROM:
      hdev->wire->buffer.ptr = &hdev->rom_code;
      hdev->wire->buffer.size = DS18X20_ROM_CODE_SIZE;
      hdev->async_state.S1 = DS18X20_S1_READY;
      hdev->async_state.status = wire_tx_it(hdev->wire, s1_callback);
      if (hdev->async_state.status != HAL_OK) goto check;
      break;
    case DS18X20_CMD_SKIP_ROM:
      hdev->async_state.S1 = DS18X20_S1_READY;
      goto next_state;
      break;
    }
    break;
  case DS18X20_S1_COMMAND_RX:
    hdev->async_state.status = wire_crc(hdev->wire);
    if (hdev->async_state.S1_cmd == DS18X20_CMD_READ_ROM)
      hdev->is_ds18b20 = hdev->rom_code[0] == 0x28;
  case DS18X20_S1_READY:
    if (hdev->async_state.S1_callback) hdev->async_state.S1_callback(hdev);
    break;
  }
}

/**
 * @fn void s2_callback(Wire_HandleTypeDef*)
 * @brief DS18X20 function state machine.
 * @param wire 1-Wire handle pointer.
 */
static void s2_callback(Wire_HandleTypeDef* wire)
{
  DS18X20_HandleTypeDef* hdev = wire->hdev;
  if (!hdev) return;
  check:
  if (wire->status != HAL_OK) hdev->async_state.status = wire->status;
  if (hdev->async_state.status != HAL_OK)
  {
    if (hdev->async_state.callback) hdev->async_state.callback(hdev);
    return;
  }
  next_state:
  switch (hdev->async_state.S2)
  {
  case DS18X20_S2_INIT:
    hdev->async_state.S2 = DS18X20_S2_READY;
    goto next_state;
    break;
  case DS18X20_S2_COMMAND_TX:
    switch (hdev->async_state.S2_cmd)
    {
    case DS18X20_CMD_READ_POWER_SUPPLY:
      hdev->wire->buffer.ptr = &hdev->wire->is_powered;
      hdev->wire->buffer.size = 1;
      hdev->async_state.S2 = DS18X20_S2_READY;
      hdev->async_state.status = wire_rx_it(hdev->wire, s2_callback);
      if (hdev->async_state.status != HAL_OK) goto check;
      break;
    case DS18X20_CMD_READ_SCRATCHPAD:
      hdev->wire->buffer.ptr = &hdev->scratchpad;
      hdev->wire->buffer.size = DS18X20_SCRATCHPAD_SIZE;
      hdev->async_state.S2 = DS18X20_S2_COMMAND_RX;
      hdev->async_state.status = wire_rx_it(hdev->wire, s2_callback);
      if (hdev->async_state.status != HAL_OK) goto check;
      break;
    case DS18X20_CMD_WRITE_SCRATCHPAD:
      hdev->wire->buffer.ptr = &hdev->scratchpad[DS18X20_OFFSET_USER_BYTE1];
      hdev->wire->buffer.size = DS18B20_CONFIGURATION_SIZE;
      hdev->async_state.S2 = DS18X20_S2_READY;
      hdev->async_state.status = wire_tx_it(hdev->wire, s2_callback);
      if (hdev->async_state.status != HAL_OK) goto check;
      break;
    default:
      hdev->async_state.S2 = DS18X20_S2_READY;
      goto next_state;
    }
    break;
  case DS18X20_S2_COMMAND_RX:
    switch (hdev->async_state.S2_cmd)
    {
    case DS18X20_CMD_READ_SCRATCHPAD:
      hdev->async_state.status = wire_crc(hdev->wire);
      if (hdev->async_state.status == HAL_OK)
        calculate_accuracy(hdev);
      hdev->async_state.S2 = DS18X20_S2_READY;
    }
  case DS18X20_S2_READY:
    if (hdev->async_state.S2_callback) hdev->async_state.S2_callback(hdev);
    break;
  }
}

/**
 * @fn void s2_s1_callback(DS18X20_HandleTypeDef*)
 * @brief Sends the function command after completing a ROM command.
 * @param hdev Device handle pointer.
 */
static void s2_s1_callback(DS18X20_HandleTypeDef* hdev)
{
  set_cmd(hdev, &hdev->async_state.S2_cmd);
  hdev->async_state.status = wire_tx_it(hdev->wire, s2_callback);
}

/**
 * @fn void c1_callback(DS18X20_HandleTypeDef*)
 * @brief DS18X20 initialization state machine.
 * @param hdev Device handle pointer.
 */
static void c1_callback(DS18X20_HandleTypeDef* hdev)
{
  check:
  if (hdev->async_state.status != HAL_OK)
  {
    if (hdev->async_state.callback) hdev->async_state.callback(hdev);
    return;
  }
  uint8_t resolutionSetting = hdev->resolution;
  next_state:
  switch (hdev->async_state.C1)
  {
  case DS18X20_C1_INIT:
    hdev->async_state.C1 = DS18X20_C1_READY;
    goto next_state;
    break;
  case DS18X20_C1_READ_ROM:
    hdev->async_state.C1 = DS18X20_C1_READ_SCRATCHPAD;
    hdev->async_state.status = s2_command(hdev, DS18X20_CMD_READ_SCRATCHPAD, c1_callback);
    if (hdev->async_state.status != HAL_OK) goto check;
    break;
  case DS18X20_C1_READ_SCRATCHPAD:
    hdev->async_state.C1 = DS18X20_C1_READ_POWER_SUPPLY;
    hdev->async_state.status = s2_command(hdev, DS18X20_CMD_READ_POWER_SUPPLY, c1_callback);
    if (hdev->async_state.status != HAL_OK) goto check;
    break;
  case DS18X20_C1_READ_POWER_SUPPLY:
    hdev->resolution = get_resolution(hdev);
    if (hdev->is_ds18b20 &&
        resolutionSetting != hdev->resolution &&
        resolutionSetting >= DS18B20_RESOLUTION_MIN &&
        resolutionSetting <= DS18B20_RESOLUTION_MAX)
    {
      set_resolution(hdev, hdev->resolution = resolutionSetting);
      hdev->async_state.C1 = DS18X20_C1_WRITE_SCRATCHPAD;
      hdev->async_state.status = s2_command(hdev, DS18X20_CMD_WRITE_SCRATCHPAD, c1_callback);
      if (hdev->async_state.status != HAL_OK) goto check;
    }
    else
    {
      hdev->async_state.C1 = DS18X20_C1_READY;
      goto next_state;
    }
    break;
  case DS18X20_C1_WRITE_SCRATCHPAD:
    calculate_accuracy(hdev);
  case DS18X20_C1_READY:
    if (hdev->async_state.callback) hdev->async_state.callback(hdev);
    break;
  }
}

/**
 * @fn void c2_callback(DS18X20_HandleTypeDef*)
 * @brief Temperature conversion state meachine.
 * @param hdev Device handle pointer.
 */
static void c2_callback(DS18X20_HandleTypeDef* hdev)
{
  double reading = 0;
  uint8_t update = 0;
  check:
  if (hdev->async_state.status != HAL_OK)
  {
    hdev->reading = -273.0; // impossible value to reset the change detection in case of recovery
    if (hdev->async_state.callback) hdev->async_state.callback(hdev);
    return;
  }
  next_state:
  switch (hdev->async_state.C2)
  {
  case DS18X20_C2_INIT:
    hdev->async_state.C2 = DS18X20_C2_READY;
    goto next_state;
    break;
  case DS18X20_C2_CONVERT_T:
    if (!hdev->wire->is_powered)
      wire_set_pullup(hdev->wire, WIRE_P_STRONG);
    hdev->async_state.C2 = DS18X20_C2_READ_SCRATCHPAD_TX;
    delay_async(hdev->tconv, hdev, (TIM_EX_CallbackTypeDef) c2_callback);
    break;
  case DS18X20_C2_READ_SCRATCHPAD_TX:
    if (!hdev->wire->is_powered)
      wire_set_pullup(hdev->wire, WIRE_P_WEAK);
    hdev->async_state.C2 = DS18X20_C2_READ_SCRATCHPAD_RX;
    hdev->async_state.status = s2_command(hdev, DS18X20_CMD_READ_SCRATCHPAD, c2_callback);
    if (hdev->async_state.status != HAL_OK) goto check;
    break;
  case DS18X20_C2_READ_SCRATCHPAD_RX:
    reading = ((int16_t*) (void*) hdev->scratchpad)[0] / (hdev->is_ds18b20 ? 16.0 : 2.0);
    update = fabs(hdev->reading - reading) >= hdev->increment_min;
    if (update)
    {
      hdev->reading = reading;
      update_accuracy(hdev);
      if (hdev->async_state.callback) hdev->async_state.callback(hdev);
    }
    hdev->async_state.C2 = DS18X20_C2_READY;
  case DS18X20_C2_READY:
    if (hdev->async_state.enabled)
    {
      hdev->async_state.C2 = DS18X20_C2_CONVERT_T;
      hdev->async_state.status = s2_command(hdev, DS18X20_CMD_CONVERT_T, c2_callback);
      if (hdev->async_state.status != HAL_OK) goto check;
    }
    break;
  }
}

#endif // WIRE_USE_IT

#pragma endregion

#pragma endregion

#pragma region Public API

#if WIRE_USE_POLLING
/**
 * @fn HAL_StatusTypeDef ds18x20_init(DS18X20_handle*)
 * @brief Initializes the device in polling mode.
 * @remarks Reads ROM code and defaults from the scratchpad, sets the resolution and accuracy if applicable.
 * @param hdev Device handle pointer.
 * @return HAL status.
 */
HAL_StatusTypeDef ds18x20_init(DS18X20_HandleTypeDef* hdev)
{
  HAL_StatusTypeDef status = wire_init(hdev->wire, hdev);
  if (status != HAL_OK) return status;
  status = read_rom(hdev);
  if (status != HAL_OK) return status;
  status = read_scratchpad(hdev);
  if (status != HAL_OK) return status;
  uint8_t resolutionSetting = hdev->resolution;
  hdev->resolution = get_resolution(hdev);
  calculate_accuracy(hdev);
  if (hdev->is_ds18b20 &&
      resolutionSetting != hdev->resolution &&
      resolutionSetting >= DS18B20_RESOLUTION_MIN &&
      resolutionSetting <= DS18B20_RESOLUTION_MAX)
  {
    hdev->resolution = resolutionSetting;
    calculate_accuracy(hdev);
    set_resolution(hdev, hdev->resolution = resolutionSetting);
    return write_scratchpad(hdev);
  }
  return HAL_OK;
}
#endif

#if WIRE_USE_IT
/**
 * @fn HAL_StatusTypeDef ds18x20_init_it(DS18X20_HandleTypeDef*)
 * @brief Initializes the device in interrupt mode.
 * @remarks Reads ROM code and defaults from the scratchpad, sets the resolution and accuracy if applicable.
 * @param hdev Device handle pointer.
 * @param callback A function to call on initialization completed.
 * @return HAL status.
 */
HAL_StatusTypeDef ds18x20_init_it(DS18X20_HandleTypeDef* hdev, DS18X20_CallbackTypeDef callback)
{
  HAL_StatusTypeDef status = hdev->async_state.status = wire_init(hdev->wire, hdev);
  if (status != HAL_OK) return status;
  return c1_start(hdev, callback);
}
#endif

#if WIRE_USE_POLLING
/**
 * @fn HAL_StatusTypeDef ds18x20_read(DS18X20_handle*)
 * @brief Reads the current temperature in polling mode.
 * @param hdev Device handle pointer.
 * @return HAL status.
 */
HAL_StatusTypeDef ds18x20_read(DS18X20_HandleTypeDef* hdev)
{
  if (!hdev->resolution) return HAL_ERROR; // not initialized.
  HAL_StatusTypeDef status = send_cmd(hdev, DS18X20_CMD_CONVERT_T);
  if (status != HAL_OK) return status;
  wire_set_pullup(hdev->wire, WIRE_P_STRONG);
  HAL_Delay(hdev->tconv);
  wire_set_pullup(hdev->wire, WIRE_P_WEAK);
  status = read_scratchpad(hdev);
  if (status != HAL_OK) return status;
  hdev->reading = ((int16_t*) (void*) hdev->scratchpad)[0] / 16.0f;
  update_accuracy(hdev);
  return status;
}
#endif

#if WIRE_USE_IT
/**
 * @fn HAL_StatusTypeDef ds18x20_start_it(DS18X20_HandleTypeDef*, DS18X20_CallbackTypeDef)
 * @brief Starts the continuous temperature reading from the DS18X20 in the interrupt mode.
 * @param hdev Device handle pointer.
 * @param callback A function to be call on each reading.
 * @return HAL status.
 */
HAL_StatusTypeDef ds18x20_start_it(DS18X20_HandleTypeDef* hdev, DS18X20_CallbackTypeDef callback)
{
  hdev->async_state.enabled = 1;
  return c2_start(hdev, callback);
}

/**
 * @fn HAL_StatusTypeDef ds18x20_stop_it(DS18X20_HandleTypeDef*)
 * @brief Stops the continuous temperature reading from the DS18X20 if it was started.
 * @param hdev Device handle pointer.
 * @return HAL status.
 */
HAL_StatusTypeDef ds18x20_stop_it(DS18X20_HandleTypeDef* hdev)
{
  hdev->async_state.enabled = 0;
  return HAL_OK;
}

/**
 * @fn HAL_StatusTypeDef ds18x20_resume_it(DS18X20_HandleTypeDef*)
 * @brief Resumes the continuous temperature reading from the DS18X20 if it was started, then stopped.
 * @param hdev Device handle pointer.
 * @return HAL status.
 */
HAL_StatusTypeDef ds18x20_resume_it(DS18X20_HandleTypeDef* hdev)
{
  hdev->async_state.enabled = 1;
  return c2_start(hdev, hdev->async_state.callback);
}
#endif // WIRE_USE_IT

#pragma endregion

#endif
