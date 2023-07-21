/**
 * @file        LogMessage.cpp
 * @author      CodeDog
 * @brief       Log message class.
 *
 * @copyright	(c)2022 CodeDog, All rights reserved.
 */

#include "LogMessage.hpp"
#include "DateTimeEx.hpp"

LogMessage::LogMessage()
    : _severity(debug), _length(0), _offset(0), _buffer() { }

LogMessage::LogMessage(Severity s)
    : _severity(s), _length(0), _offset(0), _buffer() { }

LogMessage::LogMessage(const LogMessage &other)
    : _severity(other._severity), _length(other._length), _offset(other._offset), _buffer()
{
    for (size_t i = 0; i < _length; i++) _buffer[i] = other._buffer[i];
}

LogMessage::LogMessage(LogMessage &&other)
    : _severity(other._severity), _length(other._length), _offset(other._offset), _buffer()
{
    for (size_t i = 0; i < _length; i++) _buffer[i] = other._buffer[i];
}

void LogMessage::clear()
{
    _offset = 0;
    _length = 0;
    memset(_buffer, 0, size);
}

LogMessage* LogMessage::add(char c, int count)
{
    if (_length + count > size) return this;
    for (int i = 0; i < count; i++)
    {
        _buffer[_offset] = c;
        _offset++;
        _length++;
    }
    return this;
}

LogMessage* LogMessage::add(const char* s)
{
    size_t l = strlen(s);
    if (_length + l > size) return this;
    memcpy(&_buffer[_offset], s, l);
    _offset += l;
    _length += l;
    return this;
}

LogMessage* LogMessage::addTimestamp()
{
    DateTimeEx timestamp;
    if (timestamp.getRTC())
        return printf(
            dateTimeFormat,
            timestamp.year, timestamp.month, timestamp.day,
            timestamp.hour, timestamp.minute, timestamp.second + timestamp.fraction
        );
    else return this->add('*');
}

std::pair<const uint8_t*, int> LogMessage::buffer()
{
    return { (const uint8_t*)&_buffer, _length };
}


