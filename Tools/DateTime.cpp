/**
 * @file        DateTime.cpp
 * @author      CodeDog
 * @brief       A universal and compact date / time storage class.
 *
 * @copyright   (c)2023 CodeDog, All rights reserved.
 */

#include "DateTime.hpp"
#include <cmath>
#include <ctime>


DateTime::DateTime()
    : year(0), month(1), day(1), hour(0), minute(0), second(0), fraction(0) { }

DateTime::DateTime(
    int16_t _year, uint8_t _month, uint8_t _day,
    uint8_t _hour, uint8_t _minute, uint8_t _second, double _fraction)
    : year(_year), month(_month), day(_day),
      hour(_hour), minute(_minute), second(_second), fraction(_fraction) { }

bool DateTime::isSet()
{
    return year != 0 || month != 1 || day != 1 || hour != 0 || minute != 0 || second != 0 || fraction != 0.0;
}

/**
 * @fn bool isValid()
 * @brief Tests if the current instance data is valid.
 * @return 1: Valid. 0: Invalid.
 */
bool DateTime::isValid()
{
    return
        month >= 1 && month <= 12 && day >= 1 && day <= daysInMonth(year, month) &&
        hour <= 24 && minute <= 60 && second <= 60 && fraction < 1;
}

/**
 * @brief Tests if this time is the same date as the other time.
 *
 * @param other Other time to test.
 * @return true The date parts are equal.
 * @return false The date parts differ.
 */
bool DateTime::isTheSameDay(DateTime &other)
{
    return year == other.year && month == other.month && day == other.day;
}

/**
 * @brief Tests if this time is at least 1 second ahead of the other time (the other time lags behind).
 *
 * @param other Other time to test.
 * @return true This time is ahead of the other.
 * @return false This time is not ahead of the other, it's either the same or lags behind.
 */
bool DateTime::atLeastSecondAheadOf(DateTime& other)
{
    if (year > other.year) return true;
    if (year < other.year) return false;
    if (month > other.month) return true;
    if (month < other.month) return false;
    if (day > other.day) return true;
    if (day < other.day) return false;
    if (hour > other.hour) return true;
    if (hour < other.hour) return false;
    if (minute > other.minute) return true;
    if (minute < other.minute) return false;
    if (second > other.second) return true;
    if (second < other.second) return false;
    return false;
}

void DateTime::reset()
{
    year = 0; month = 1; day = 1;
    hour = 0; minute = 0; second = 0; fraction = 0.0;
}

uint8_t DateTime::daysInMonth(uint16_t _year, uint8_t _month)
{
    return _month != 2 ? 30 + ((_month % 2) ^ (_month > 7)) : 28 + (_year % 4 == 0 && (_year % 100 != 0 || _year % 400 == 0));
}

TimeSpan DateTime::operator-(DateTime &other)
{
    tm tm1 = {0};
    tm1.tm_year = year;
    tm1.tm_mon = month;
    tm1.tm_mday = day;
    tm1.tm_hour = hour;
    tm1.tm_min = minute;
    tm1.tm_sec = second;
    time_t t1 = mktime(&tm1);
    tm tm0 = {0};
    tm0.tm_year = other.year;
    tm0.tm_mon = other.month;
    tm0.tm_mday = other.day;
    tm0.tm_hour = other.hour;
    tm0.tm_min = other.minute;
    tm0.tm_sec = other.second;
    time_t t0 = mktime(&tm0);
    double diffIntPart = difftime(t1, t0);
    double diffFractPart = fraction - other.fraction;
    return diffIntPart + diffFractPart;
}

bool DateTime::operator==(DateTime &other) { return eq(other); }
bool DateTime::operator !=(DateTime& other) { return !eq(other); }
bool DateTime::operator <=(DateTime& other) { return cmp(other, true, true); }
bool DateTime::operator >=(DateTime& other) { return cmp(other, false, true); }
bool DateTime::operator <(DateTime& other) { return cmp(other, true, false); }
bool DateTime::operator >(DateTime& other) { return cmp(other, false, false); }

bool DateTime::cmp(DateTime& other, bool lt, bool eq) const
{
    if (year < other.year) return lt;
    if (year > other.year) return !lt;
    if (month < other.month) return lt;
    if (month > other.month) return !lt;
    if (day < other.day) return lt;
    if (day > other.day) return !lt;
    if (hour < other.hour) return lt;
    if (hour > other.hour) return !lt;
    if (minute < other.minute) return lt;
    if (minute > other.minute) return !lt;
    if (second < other.second) return lt;
    if (second > other.second) return !lt;
    if (fraction < other.fraction) return lt;
    if (fraction > other.fraction) return !lt;
    return eq;
}

bool DateTime::eq(DateTime& other) const
{
    return
        year == other.year &&
        month == other.month &&
        day == other.day &&
        hour == other.hour &&
        minute == other.minute &&
        second == other.second &&
        fraction == other.fraction;
}
