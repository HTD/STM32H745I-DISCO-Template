/**
 * @file      datetime.h
 * @author    CodeDog
 * @brief     Date / time tools, header file.
 * @remarks
 *            Introduces DateTypeDef, TimeTypeDef, DateTimeTypeDef types.
 *            Supports RTC_DateTypeDef, RTC_TimeTypeDef and DWORD FAT time types and provides conversions between them.
 *
 * @copyright	(c)2022 CodeDog, All rights reserved.
 */

#pragma once

#include <integer.h>
#include "stm32h7xx_hal.h"

#define ISO_DATE_F "%04u-%02u-%02u" ///< ISO8601 date format.
#define ISO_DATE_L 11 ///< Date string length (trailing zero included).
#define ISO_TIME_F "%02u-%02u-%02u" ///< ISO8601 time format.
#define ISO_TIME_L 9 ///< Time string length (trailing zero included).
#define ISO_DATE_TIME_F "%04u-%02u-%02u %02u-%02u-%02u" ///< ISO8601 date/time format.
#define ISO_DATE_TIME_L 20 ///< ISO8601 date/time length (trailing zero included).
#define ISO_DATE_TIME_MS_F "%04u-%02u-%02u %02u:%02u:%06.3f" ///< ISO8601 date/time + milliseconds format.
#define ISO_DATE_TIME_MS_L 24 ///< ISO8601 date/time + milliseconds length (trailing zero included).
#define ISO_DATE_TIME_US_F "%04u-%02u-%02u %02u:%02u:%09.6f" ///< ISO8601 date/time + microseconds format.
#define ISO_DATE_TIME_US_L 27 ///< ISO8601 date/time + microseconds length (trailing zero included).
#define ISO_DATE_TIME_NS_F "%04u-%02u-%02u %02u:%02u:%012.9f" ///< ISO8601 date/time + nanoseconds format.
#define ISO_DATE_TIME_NS_L 30 ///< ISO8601 date/time + nanoseconds length (trailing zero included).

/**
 * @def FAT_TIME
 * @brief Returns FAT time calculated from date components.
 */
#define FAT_TIME(y, M, d, h, m, s) ((DWORD)(y-1980)<<25|(DWORD)M<<21|(DWORD)d<<16|(DWORD)h<<11|(DWORD)m<<5|(DWORD)s>>1)

/**
 * @def FAT_TIME_SET
 * @brief Sets the date components from FAT time.
 */
#define FAT_TIME_SET(f, y, M, d, h, m, s) {y=1980+(f>>25&127);M=f>>21&15;d=f>>16&31;h=f>>11&31;m=f>>5&63;s=(f&15)<<1;}

extern RTC_HandleTypeDef hrtc;

/**
 * @typedef DateTypeDef
 * @struct
 * @brief A simple unpacked universal date structure.
 */
typedef struct __attribute__((__packed__))
{
    int16_t y; ///< Year number. -32768..32767. Because we can afford 16 bits in XXI century.
    uint8_t m; ///< Month number. 1..12.
    uint8_t d; ///< Day number. 1..31.
} DateTypeDef;

/**
 * @typedef TimeTypeDef
 * @struct
 * @brief A precise unpacked local time structure.
 */
typedef struct __attribute__((__packed__))
{
    uint8_t h; ///< Hour number. 0..23.
    uint8_t m; ///< Minute number. 0..59.
    uint8_t s; ///< Second number. 0..59.
    double f; ///< Fraction of the next second. 0..1.
} TimeTypeDef;

/**
 * @typedef DateTimeTypeDef
 * @struct
 * @brief A simple 10 byte date/time structure containing full date and local time with 1ns precision.
 */
typedef struct __attribute__((__packed__))
{
    DateTypeDef date;
    TimeTypeDef time;
} DateTimeTypeDef;

uint8_t dayOfWeek(uint16_t y, uint8_t m, uint8_t d);
uint8_t daysInMonth(uint16_t y, uint8_t m);
uint8_t isDayInMonth(uint16_t y, uint8_t m, uint8_t d);
uint8_t isValidDate(uint16_t y, uint8_t m, uint8_t d);
uint8_t isValidTime(uint8_t h, uint8_t m, uint8_t s);
uint8_t isValidDateTime(DateTimeTypeDef* dt);
void RTC2DateTime(RTC_DateTypeDef* rd, RTC_TimeTypeDef* rt, DateTimeTypeDef* dt);
void DateTime2RTC(DateTimeTypeDef* dt, RTC_DateTypeDef* rd, RTC_TimeTypeDef* rt);
HAL_StatusTypeDef RTC_GetDateTime(DateTimeTypeDef* dt);
HAL_StatusTypeDef RTC_SetDateTime(DateTimeTypeDef* dt);
DWORD DateTime2FAT(DateTimeTypeDef* dt);
DWORD RTC2FAT(RTC_DateTypeDef* rd, RTC_TimeTypeDef* rt);
void FAT2DateTime(DWORD ft, DateTimeTypeDef* dt);
DWORD RTC_GetFATTime();
