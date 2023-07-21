/**
 * @file      ds18x20.h
 * @author    CodeDog
 * @brief     Digital thermometer driver for DS1820, DS18S20 and DS18B20, header file.
 *
 * @see       https://pdf1.alldatasheet.pl/pdfjsview/web/viewer.html?file=//pdf1.alldatasheet.pl/datasheet-pdf/view/58548/DALLAS/DS1820/+_7WW9UzYIKCTchVBEvN+/datasheet.pdf
 *            https://datasheets.maximintegrated.com/en/ds/DS18S20.pdf
 *            https://datasheets.maximintegrated.com/en/ds/DS18B20.pdf
 *            https://www.maximintegrated.com/en/design/technical-documents/app-notes/4/4377.html
 *
 * @copyright (c)2022 CodeDog, All rights reserved.
 */

#pragma once

#include "1wire.h"

#define DS18X20_ID 0x10
#define DS18B20_ID 0x28

#define DS18X20_ROM_CODE_SIZE 8
#define DS18X20_SCRATCHPAD_SIZE 9
#define DS18B20_CONFIGURATION_SIZE 3

#define DS18X20_OFFSET_USER_BYTE1 0x02
#define DS18X20_OFFSET_USER_BYTE2 0x03
#define DS18X20_OFFSET_CONFIGURATION 0x04
#define DS18X20_RESOLUTION_MASK 0b10011111
#define DS18X20_RESOLUTION_R0_SHIFT 5

#define DS18X20_RESOLUTION 9
#define DS18B20_RESOLUTION_MIN 9
#define DS18B20_RESOLUTION_MAX 12
#define DS18B20_RESOLUTION_DEFAULT 12
#define DS18B20_INCREMENT_MIN 0.0625f
#define DS18X20_INCREMENT_MIN 0.5f
#define DS18B20_TCONV_MAX 750
#define DS18X20_TCONV_MAX 750 // definitely not 500 for 3V3 PS.
#define DS18B20_T0_MIN -10.0f
#define DS18X20_T0_MIN 0.0f
#define DS18B20_T0_MAX 85.0f
#define DS18X20_T0_MAX 70.0f
#define DS18X20_T0_ACCURACY 0.5f
#define DS18X20_T1_MIN -30.0f
#define DS18X20_T1_MAX 100.0f
#define DS18X20_T1_ACCURACY 1.0f
#define DS18X20_T2_MIN -55.0f
#define DS18X20_T2_MAX 125.0f
#define DS18X20_T2_ACCURACY 2.0f

/**
 * @typedef DS18X20_CommandTypeDef
 * @brief DS18X20 commands.
 */
typedef enum
{
  DS18X20_CMD_READ_ROM = 0x33,
  DS18X20_CMD_CONVERT_T = 0x44,
  DS18X20_CMD_COPY_SCRATCHPAD = 0x48,
  DS18X20_CMD_WRITE_SCRATCHPAD = 0x4e,
  DS18X20_CMD_MATCH_ROM = 0x55,
  DS18X20_CMD_READ_POWER_SUPPLY = 0xb4,
  DS18X20_CMD_RECALL_E2 = 0xb8,
  DS18X20_CMD_READ_SCRATCHPAD = 0xbe,
  DS18X20_CMD_SKIP_ROM = 0xcc,
  DS18X20_CMD_ALARM_SEARCH = 0xec,
  DS18X20_CMD_SEARCH_ROM = 0xf0

} DS18X20_CommandTypeDef;

typedef struct __DS18X20_HandleTypeDef DS18X20_HandleTypeDef;

#if WIRE_USE_IT

/**
 * @typedef DS18X20_StateTypeDef
 * @brief States enumeration for the DS18X20 interrupt mode state machine.
 */
typedef enum
{
  DS18X20_S_INIT,                                 ///> Ready.
  DS18X20_S_READ_ROM_RESET,                       ///> READ_ROM reset callback.
  DS18X20_S_READ_ROM_TX,                          ///> READ_ROM TX callback.
  DS18X20_S_READ_ROM_RX,                          ///> READ_ROM RX callback.
  DS18X20_S_SKIP_ROM,                             ///> SKIP_ROM TX callback, initialization complete.
  DS18X20_S_MATCH_ROM,                            ///> MATCH_ROM TX callback, initialization complete.
  DS18X20_S_START_CMD,                            ///> _ds18x20_start_cmd_it TX request.
  DS18X20_S_COMMAND_TX,                           ///> _ds18x20_start_cmd_it TX callback.
  DS18X20_S_COMMAND_RX,                           ///> _ds18x20_start_cmd_it RX callback.
  DS18X20_S_READ_SCRATCHPAD,                      ///> READ_SCRATCHPAD RX callback.
  DS18X20_S_WRITE_SCRATCHPAD_TX,                  ///> WRITE_SCRATCHPAD TX callback, sequence complete.
  DS18X20_S_WRITE_SCRATCHPAD_WAIT,                ///> Wait with strong pullup before exiting.
  DS18X20_S_CONVERT_T_START,                      ///> CONVERT_T start.
  DS18X20_S_CONVERT_T_TX,                         ///> CONVERT_T TX callback.
  DS18X20_S_WAIT,                                 ///> Conversion delay starts.
  DS18X20_S_READ_RESULT                           ///> Conversion delay callback, result must be read from the scratchpad.
} DS18X20_StateTypeDef;

/**
 * @typedef DS18X20_S1TypeDef
 * @brief S1 states for ROM functions.
 */
typedef enum
{
  DS18X20_S1_INIT,                                ///> Initial state, S1 required before S2.
  DS18X20_S1_RESET,                               ///> Reset callback.
  DS18X20_S1_COMMAND_TX,                          ///> S1 command sent.
  DS18X20_S1_COMMAND_RX,                          ///> S1 command received data.
  DS18X20_S1_READY                                ///> Ready for S2 command.
} DS18X20_S1TypeDef;

/**
 * @typedef DS18X20_S2TypeDef
 * @brief S2 states for memory / control functions.
 */
typedef enum
{
  DS18X20_S2_INIT,                                ///> Initaial state, no action.
  DS18X20_S2_COMMAND_TX,                          ///> S2 command sent.
  DS18X20_S2_COMMAND_RX,                          ///> S2 command received data.
  DS18X20_S2_READY                                ///> Ready for C1 command.
} DS18X20_S2TypeDef;

/**
 * @typedef DS18X20_C1TypeDef
 * @brief C1 states for driver initialization.
 */
typedef enum
{
  DS18X20_C1_INIT,                                ///> Initial state, no action.
  DS18X20_C1_READ_ROM,                            ///> READ_ROM completed.
  DS18X20_C1_READ_SCRATCHPAD,                     ///> READ_SCRATCHPAD completed.
  DS18X20_C1_READ_POWER_SUPPLY,                   ///> Power supply state read.
  DS18X20_C1_WRITE_SCRATCHPAD,                    ///> WRITE_SCRATCHPAD completed.
  DS18X20_C1_READY                                ///> Ready for C2 command.
} DS18X20_C1TypeDef;

/**
 * @typedef DS18X20_C2TypeDef
 * @brief C2 states for measurements.
 */
typedef enum
{
  DS18X20_C2_INIT,                                ///> Initial state, no action.
  DS18X20_C2_CONVERT_T,                           ///> CONVERT_T sent.
  DS18X20_C2_READ_SCRATCHPAD_TX,                  ///> READ_SCRATCHPAD started.
  DS18X20_C2_READ_SCRATCHPAD_RX,                  ///> READ_SCRATCHPAD completed.
  DS18X20_C2_READY                                ///> Reading done.
} DS18X20_C2TypeDef;

/**
 * @typedef DS18X20_CallbackTypeDef
 * @brief Callback type for DS18X20 interrupt mode functions.
 */
typedef void (*DS18X20_CallbackTypeDef)(DS18X20_HandleTypeDef*);

/**
 * @typedef DS18X20_AsyncStateTypeDef
 * @brief The state of the DS18X20 asynchronous transaction.
 */
typedef struct
{
  DS18X20_S1TypeDef S1;                 ///> S1 state register.
  DS18X20_S2TypeDef S2;                 ///> S2 state register.
  DS18X20_C1TypeDef C1;                 ///> C1 state register.
  DS18X20_C2TypeDef C2;                 ///> C2 state register.
  uint8_t S1_cmd;                       ///> S1 command to send. Zero means no command.
  uint8_t S2_cmd;                       ///> S2 command to send. Zero means no command.
  DS18X20_CallbackTypeDef S1_callback;  ///> The callback from S1 state.
  DS18X20_CallbackTypeDef S2_callback;  ///> The callback from S2 state.
  uint8_t enabled;                      ///> 1: enabled, continuous reading, 0: stopped, no more readings until resumed.
  HAL_StatusTypeDef status;             ///> HAL status of the last operation.
  DS18X20_CallbackTypeDef callback;     ///> Function called on the end of the sequence.
} DS18X20_AsyncStateTypeDef;

#endif // WIRE_USE_IT

/**
 * @brief DS18X20 handle.
 * Contains the current state of the DS18X20 connected.
 */
typedef struct __DS18X20_HandleTypeDef
{
  Wire_HandleTypeDef* wire;                     ///> 1-Wire handle pointer.
  uint8_t match_rom;                            ///> Set 1 to match with the rom code.
  uint8_t rom_code[DS18X20_ROM_CODE_SIZE];      ///> ROM code unique to each DS18X20.
  uint8_t scratchpad[DS18X20_SCRATCHPAD_SIZE];  ///> DS18X20 volatile data storage.
  uint8_t is_ds18b20;                           ///> 1: is DS18B20, 2: is DS1820 or DS18S20.
  uint8_t resolution;                           ///> Reading resolution in bits per sample.
  uint16_t tconv;                               ///> Conversion time in milliseconds.
  double reading;                               ///> Reading value in °C.
  double accuracy;                              ///> Maximum reading deviation from the exact temperature in °C.
  double increment;                             ///> Minimum reading increment in °C for the current resolution.
  double increment_min;                         ///> Minimum reading increment to trigger change reporting.
#if WIRE_USE_IT
  DS18X20_AsyncStateTypeDef async_state;        ///> Asynchronouos transaction state.
#endif
} DS18X20_HandleTypeDef;

#ifdef __cplusplus
extern "C"
{
#endif

#if WIRE_USE_POLLING
HAL_StatusTypeDef ds18x20_init(DS18X20_HandleTypeDef* hdev);
HAL_StatusTypeDef ds18x20_read(DS18X20_HandleTypeDef* hdev);
#endif

#if WIRE_USE_IT
HAL_StatusTypeDef ds18x20_init_it(DS18X20_HandleTypeDef* hdev, DS18X20_CallbackTypeDef callback);
HAL_StatusTypeDef ds18x20_start_it(DS18X20_HandleTypeDef* hdev, DS18X20_CallbackTypeDef callback);
HAL_StatusTypeDef ds18x20_stop_it(DS18X20_HandleTypeDef* hdev);
HAL_StatusTypeDef ds18x20_resume_it(DS18X20_HandleTypeDef* hdev);
#endif

#ifdef __cplusplus
}
#endif
