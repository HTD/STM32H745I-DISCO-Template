/**
 * @file      1wire_uart.h
 * @author    CodeDog
 * @brief     1-Wire UART transport layer driver, header file.
 * @remarks
 *            Uses HAL UART configured as Single Wire Half Duplex.
 *
 * @copyright	(c)2022 CodeDog, All rights reserved.
 */

#pragma once

#include "1wire.h"

#if WIRE_USE_UART

/**
 * @def WIRE_USE_EXACT_TIMING
 * @brief Use exact UART timing for 1-wire for slightly faster and more reliable communication.
 * @remarks Works only with certain types of USART ports.
 */
#define WIRE_UART_USE_EXACT_TIMING 0

#if WIRE_UART_USE_EXACT_TIMING

/**
 * @def WIRE_BAUDRATE_RESET
 * @brief Baud rate for the RESET signal. Time slot 480μs => br = 10M / (2 * 480).
 */
#define WIRE_BAUDRATE_RESET 10416

/**
 * @def WIRE_BAUDRATE_DATA
 * @brief Baud rate for the data bits. Time slot 70μs => br = 10M / 70.
 */
#define WIRE_BAUDRATE_DATA 142857

#else

/**
 * @def WIRE_BAUDRATE_RESET
 * @brief Baud rate for the RESET signal. Value compliant with Maxim 1-Wire Master.
 */
#define WIRE_BAUDRATE_RESET 9600

/**
 * @def WIRE_BAUDRATE_DATA
 * @brief Baud rate for the data bits. Value compliant with Maxim 1-Wire Master.
 */
#define WIRE_BAUDRATE_DATA 134400 // 115200 - if 134400 fails.

#endif // WIRE_UART_USE_EXACT_TIMING

#define WIRE_UART_RESET_REQUEST 0xf0     ///< reset signal request content.
#define WIRE_UART_BIT_REQUEST 0xff       ///< bit request signal.

extern Wire_DriverTypeDef wire_uart_driver;

#endif // WIRE_USE_UART
