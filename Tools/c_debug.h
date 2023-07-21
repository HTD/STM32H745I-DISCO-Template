/**
 * @file        c_debug.h
 * @author      CodeDog
 * @brief       Debug module C bindings.
 * @remarks     -
 *
 * @copyright   (c)2023 CodeDog, All rights reserved.
 */

#pragma once

#include "stm32h7xx_hal.h"

/// @brief Initializes the UART debug module by providing the configured UART handle pointer.
void debug_init(UART_HandleTypeDef* huart);

/// @brief Sends a debug message.
/// @param severity 0: error, 1: warning, 2: info, 3: debug, 4: detail, 5: spam.
/// @param format Format string.
/// @param ... arguments passed with the format string.
void debug(uint8_t severity, const char* format, ...);

/// @brief Lights the red LED on the MCU board.
void red_light_on();

/// @brief Turns the red LED on the MCU board off.
void red_light_off();
