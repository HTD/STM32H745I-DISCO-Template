/**
 * @file        DateTime.hpp
 * @author      CodeDog
 * @brief       A universal and compact date / time storage class. Header file.
 *
 * @copyright	(c)2023 CodeDog, All rights reserved.
 */

#pragma once

#include <cstdint>
#include "TimeSpan.hpp"

#pragma pack(push, 1)
class DateTime
{
public:
    int16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    double fraction;

    /// @brief Creates an empty DateTime instance.
    DateTime();

    /**
     * @brief Creates a DateTime instance for a specific date and time.
     * @param _year Year component.
     * @param _month Month component, 1..12.
     * @param _day Day component, 1..31.
     * @param _hour Hour component, 0..23.
     * @param _minute Minute component, 0..59.
     * @param _second Second component, 0..59.
     * @param _fraction Fraction of second component, 0..1.
     */
    DateTime(
        int16_t _year, uint8_t _month = 1, uint8_t _day = 1,
        uint8_t _hour = 0, uint8_t _minute = 0, uint8_t _second = 0, double _fraction = 0);

    /// @returns 1: The date is set. 0: The date is empty / zero.
    bool isSet();

    /// @returns 1: The data is valid. 0: The data is invalid.
    bool isValid();

    /// @returns 1: The other date has the same day.
    bool isTheSameDay(DateTime& other);

    /// @returns 1: This time is at least 1 second ahead of the other time (the other time lags behind).
    bool atLeastSecondAheadOf(DateTime& other);

    /// @brief Resets time to the empty / zero value.
    void reset();

    /// @returns A time span between this and the other date/time object.
    TimeSpan operator-(DateTime& other);

    bool operator==(DateTime& other);
    bool operator!=(DateTime& other);
    bool operator<=(DateTime& other);
    bool operator>=(DateTime& other);
    bool operator<(DateTime& other);
    bool operator>(DateTime& other);

    /// @brief Calculates the number of days in month.
    static uint8_t daysInMonth(uint16_t year, uint8_t month);

protected:
    bool cmp(DateTime& other, bool lt, bool eq) const;
    bool eq(DateTime& other) const;


};
#pragma pack(pop)
