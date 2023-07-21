#pragma once

#include <cstdint>

class TimeSpan
{
public:

    /// @brief Creates zero time span.
    TimeSpan();

    /// @brief Creates a time span from seconds.
    TimeSpan(double value);

    /// @brief Creates a time span from hours, minutes and seconds.
    TimeSpan(uint8_t hours, uint8_t minutes, double seconds);

    /// @brief Creates a time span from days, hours, minutes and seconds.
    TimeSpan(int days, uint8_t hours, uint8_t minutes, double seconds);

    /// @returns Seconds component of the time span.
    double seconds();

    /// @returns Minutes component of the time span.
    uint8_t minutes();

    /// @returns Hours component of the time span.
    uint8_t hours();

    /// @returns Days component of the time span.
    int days();

    /// @returns Sign component of the time span, -1 if negative, 1 if positive, 0 otherwise.
    int sign();

    /// @returns Total number of seconds.
    double totalSeconds();

    /// @returns Total number of minutes.
    double totalMinutes();

    /// @returns Total number of hours.
    double totalHours();

    /// @returns Total number of days.
    double totalDays();

    TimeSpan& operator+=(TimeSpan& other);
    TimeSpan& operator-=(TimeSpan& other);
    TimeSpan& operator+=(double other);
    TimeSpan& operator-=(double other);

    bool operator==(TimeSpan& other);
    bool operator!=(TimeSpan& other);
    bool operator<=(TimeSpan& other);
    bool operator>=(TimeSpan& other);
    bool operator<(TimeSpan& other);
    bool operator>(TimeSpan& other);

private:
    double m_value;
};