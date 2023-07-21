/**
 * @file      datetime.c
 * @author    CodeDog
 * @brief     Date / time tools.
 * @remarks
 *            Introduces DateTypeDef, TimeTypeDef, DateTimeTypeDef types.
 *            Supports RTC_DateTypeDef, RTC_TimeTypeDef and DWORD FAT time types and provides conversions between them.
 *
 * @copyright	(c)2022 CodeDog, All rights reserved.
 */

#include "c_datetime.h"

/**
 * @fn uint8_t dayOfWeek(uint16_t, uint8_t, uint8_t)
 * @brief Calculates the day of week for specified date, Michael Keith and Tom Craver implementation.
 * @see https://en.wikipedia.org/wiki/Determination_of_the_day_of_the_week#Keith
 * @param y Year.
 * @param m Month.
 * @param d Day.
 * @return Day of week index, [0..6], 0: Sunday, 1: Monday...
 */
uint8_t dayOfWeek(uint16_t y, uint8_t m, uint8_t d)
{
    return (d += m < 3 ? y-- : y - 2, 23 * m / 9 + d + 4 + y / 4 - y / 100 + y / 400) % 7;
}

/**
 * @fn uint8_t daysInMonth(uint16_t, uint8_t, uint8_t)
 * @brief Gets the number of days in a month.
 * @param y Year.
 * @param m Month.
 * @param d Day.
 * @return Number of days in month.
 */
uint8_t daysInMonth(uint16_t y, uint8_t m)
{
    return m != 2 ? 30 + ((m % 2) ^ (m > 7)) : 28 + (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0));
}

/**
 * @fn uint8_t isDayInMonth(uint16_t, uint8_t, uint8_t)
 * @brief Tests if the day exists in a month.
 * @param y Year.
 * @param m Month.
 * @param d Day.
 * @return 1: Exists. 0: Doesn't exist.
 */
uint8_t isDayInMonth(uint16_t y, uint8_t m, uint8_t d)
{
    return d > 0 && d <= (m != 2 ? 30 + ((m % 2) ^ (m > 7)) : 28 + (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0)));
}

/**
 * @fn uint8_t isValidDate(uint16_t, uint8_t, uint8_t)
 * @brief Tests if the given date is valid.
 * @param y Year. Considering the limitations of RTC_DateTypeDef and FAT time, only years from 2000 to 2107 are valid.
 * @param m Month.
 * @param d Day.
 * @return 1: Is valid. 0: Is invalid.
 */
uint8_t isValidDate(uint16_t y, uint8_t m, uint8_t d)
{
    return y > 1999 && y < 2108 && m > 0 && m < 13 && isDayInMonth(y, m, d);
}

/**
 * @fn uint8_t isValidTime(uint8_t, uint8_t, uint8_t)
 * @brief Tests if the given time is valid.
 * @param h Hours. [0..23] are considered valid.
 * @param m Minutes.
 * @param s Seconds.
 * @return 1: Is valid. 0: Is invalid.
 */
uint8_t isValidTime(uint8_t h, uint8_t m, uint8_t s)
{
    return h < 24 && m < 61 && s < 61;
}

/**
 * @fn uint8_t isValidDateTime(DateTimeTypeDef*)
 * @brief Tests if the given DateTimeTypeDef structure contains valid date and time.
 * @param dt DateTimeTypeDef structure pointer.
 * @return 1: Contains valid date and time. 0: Contains invalid date or time.
 */
uint8_t isValidDateTime(DateTimeTypeDef* dt)
{
    return isValidDate(dt->date.y, dt->date.m, dt->date.d)
            && isValidTime(dt->time.h, dt->time.m, dt->time.s) && dt->time.f < 1.0;
}

/**
 * @fn void RTC2DateTime(RTC_DateTypeDef*, RTC_TimeTypeDef*, DateTimeTypeDef*)
 * @brief Converts the RTC date and time to ISO date/time.
 * @param rtcDate RTC date pointer.
 * @param rtcTime RTC time pointer.
 * @param dateTime ISO date/time pointer.
 */
void RTC2DateTime(RTC_DateTypeDef* rd, RTC_TimeTypeDef* rt, DateTimeTypeDef* dt)
{
    dt->date.y = 2000 + rd->Year;
    dt->date.m = rd->Month;
    dt->date.d = rd->Date;
    dt->time.h = rt->Hours;
    dt->time.m = rt->Minutes;
    dt->time.s = rt->Seconds;
    if (rt->SecondFraction != 0)
        dt->time.f = (rt->SecondFraction - rt->SubSeconds) / (double) (rt->SecondFraction + 1);
}

/**
 * @fn void DateTime2RTC(DateTimeTypeDef*, RTC_DateTypeDef*, RTC_TimeTypeDef)
 * @brief Converts the ISO date/time to RTC date and RTC time.
 * @param dateTime ISO date/time pointer.
 * @param rtcDate RTC date pointer.
 * @param rtcTime RTC time pointer.
 */
void DateTime2RTC(DateTimeTypeDef* dt, RTC_DateTypeDef* rd, RTC_TimeTypeDef* rt)
{
    rd->Year = dt->date.y - 2000;
    rd->Month = dt->date.m;
    rd->Date = dt->date.d;
    rd->WeekDay = dayOfWeek(dt->date.y, dt->date.m, dt->date.d);
    rt->TimeFormat = 0x00u; // the RTC hour format must be set to 24H!
    rt->Hours = dt->time.h;
    rt->Minutes = dt->time.m;
    rt->Seconds = dt->time.s;
    rt->DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
    rt->StoreOperation = RTC_STOREOPERATION_RESET;
    if (rt->SecondFraction != 0)
        rt->SubSeconds = (1.0 - dt->time.f) * rt->SecondFraction;
}

/**
 * @fn HAL_StatusTypeDef RTC_GetDateTime(DateTimeTypeDef*)
 * @brief Gets the current RTC date and time as ISO time.
 * @param dateTime ISO date/time pointer.
 */
HAL_StatusTypeDef RTC_GetDateTime(DateTimeTypeDef* dt)
{
    if (!hrtc.Instance) return HAL_ERROR;
    RTC_DateTypeDef rd;
    RTC_TimeTypeDef rt;
    HAL_StatusTypeDef status;
    status = HAL_RTC_GetTime(&hrtc, &rt, RTC_FORMAT_BIN);
    if (status != HAL_OK) return status;
    HAL_RTC_GetDate(&hrtc, &rd, RTC_FORMAT_BIN);
    if (status != HAL_OK) return status;
    RTC2DateTime(&rd, &rt, dt);
    return HAL_OK;
}

/**
 * @fn HAL_StatusTypeDef RTC_SetDateTime(DateTimeTypeDef*)
 * @brief Sets the current RTC date and time from ISO time.
 * @param dateTime ISO date/time pointer.
 */
HAL_StatusTypeDef RTC_SetDateTime(DateTimeTypeDef* dt)
{
    if (!hrtc.Instance) return HAL_ERROR;
    if (!isValidDateTime(dt)) return HAL_ERROR;
    RTC_DateTypeDef rd = { };
    RTC_TimeTypeDef rt = { };
    HAL_StatusTypeDef status;
    DateTime2RTC(dt, &rd, &rt);
    status = HAL_RTC_SetDate(&hrtc, &rd, RTC_FORMAT_BIN);
    if (status != HAL_OK) return status;
    rt.StoreOperation = RTC_STOREOPERATION_SET;
    status = HAL_RTC_Init(&hrtc);
    if (status != HAL_OK) return status;
    status = HAL_RTC_SetTime(&hrtc, &rt, RTC_FORMAT_BIN);
    return status;
}

/**
 * @fn DWORD DateTime2FAT(DateTimeTypeDef*)
 * @brief Gets the FAT (MS-DOS type) date and time from ISO date/time.
 * @param dateTime ISO date/time pointer.
 * @return FAT time.
 */
DWORD DateTime2FAT(DateTimeTypeDef* dt)
{
    return FAT_TIME(dt->date.y, dt->date.m, dt->date.d, dt->time.h, dt->time.m, dt->time.s);
}

/**
 * @fn DWORD RTC2FAT(RTC_DateTypeDef*, RTC_TimeTypeDef*)
 * @brief Gets the FAT (MS-DOS type) date and time from RTC date and time.
 * @param rtcDate RTC date pointer.
 * @param rtcTime RTC time pointer.
 * @return FAT time.
 */
DWORD RTC2FAT(RTC_DateTypeDef* rd, RTC_TimeTypeDef* rt)
{
    return FAT_TIME(2000 + rd->Year, rd->Month, rd->Date, rt->Hours, rt->Minutes, rt->Seconds);
}

/**
 * @fn void FAT2DateTime(DWORD, DateTimeTypeDef*)
 * @brief Gets the ISO date/time from FAT (MS-DOS type) date/time format.
 * @param fatTime FAT time.
 * @param dateTime ISO date/time pointer.
 */
void FAT2DateTime(DWORD ft, DateTimeTypeDef* dt)
{
    FAT_TIME_SET(ft, dt->date.y, dt->date.m, dt->date.d, dt->time.h, dt->time.m, dt->time.s);
}

/**
 * @fn DWORD RTC_GetFATTime()
 * @brief Gets the current RTC date and time as FAT time.
 * @return FAT time.
 */
DWORD RTC_GetFATTime()
{
    RTC_DateTypeDef rd;
    RTC_TimeTypeDef rt;
    if (HAL_RTC_GetTime(&hrtc, &rt, RTC_FORMAT_BIN) != HAL_OK) return 0;
    if (HAL_RTC_GetDate(&hrtc, &rd, RTC_FORMAT_BIN) != HAL_OK) return 0;
    return RTC2FAT(&rd, &rt);
}
