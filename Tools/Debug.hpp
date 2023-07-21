/**
 * @file        Debug.hpp
 * @author      CodeDog
 * @brief       Provides serial port debugging. Header file.
 *
 * @copyright   (c)2023 CodeDog, All rights reserved.
 */

#pragma once

#ifdef DEBUG
#define LOG_LEVEL debug
#else
#define LOG_LEVEL info
#endif

#define DUMP_MARGIN 24

#include <stdio.h>
#include "main.h"
#include "cmsis_os2.h"
#include "LogMessage.hpp"
#include <new>

class Debug final
{
public:

    /**
     * @brief Prints a formatted debug message.
     *
     * @param format Text format.
     * @param ...args Variadic arguments.
     */
    template<class ...va> static void printf(const char* format, va ...args)
    {
        if (!m_isInitialized) init(0);
        int offset = ++m_last;
        if (offset < capacity)
        {
            LogMessage* msg = new(&m_messages[offset]) LogMessage(LogMessage::debug);
            msg->printf(format, args...);
            if (!offset) send(offset);
        }
    }

    /**
     * @brief Prints a formatted debug message.
     *
     * @param format Text format.
     * @param ... Optional arguments.
     */
    template<class ...va> static void tsprintf(const char* format, va ...args)
    {
        if (!m_isInitialized) init(0);
        int offset = ++m_last;
        if (offset < capacity)
        {
            LogMessage* msg = new(&m_messages[offset]) LogMessage(LogMessage::debug);
            msg->addTimestamp()->add(' ')->printf(format, args...);
            if (!offset) send(offset);
        }
    }

    /**
     * @brief Adds a formatted debug message to the log, for the detail level and above.
     *
     * @param format Text format.
     * @param args... Variadic arguments.
     */
    template<class ...va> static void dump(const char* format, va ...args)
    {
        if (!m_isInitialized) init(0);
#ifndef DEBUG
    return;
#else
        if (m_level < LogMessage::detail) return;
#endif
        int offset = ++m_last;
        if (offset < static_cast<int>(capacity))
        {
#ifdef DUMP_MARGIN
            LogMessage* msg = new(&m_messages[offset]) LogMessage(LogMessage::detail);
            msg->add(' ', DUMP_MARGIN)->printf(format, args...)->add("\r\n");
#else
            msg->printf(format, args...)->add("\r\n");
#endif
            if (!offset) send(offset);
        }
    }

    /**
     * @brief Adds a formatted debug message to the log with a timestamp.
     *
     * @param format Text format.
     * @param args... Variadic arguments.
     */
    template<class ...va> static void log(const char* format, va ...args)
    {
        if (!m_isInitialized) init(0);
        if (m_level < LogMessage::debug) return;
        int offset = ++m_last;
        if (offset < static_cast<int>(capacity))
        {
            LogMessage* msg = new(&m_messages[offset]) LogMessage(LogMessage::debug);
            msg->addTimestamp()->add(' ')->printf(format, args...)->add("\r\n");
            if (!offset) send(offset);
        }
    }

    /**
     * @brief Adds a formatted message to the log with a timestamp.
     *
     * @param severity Message severity.
     * @param format Text format.
     * @param args... Variadic arguments.
     */
    template<class ...va> static void log(LogMessage::Severity severity, const char* format, va ...args)
    {
        if (!m_isInitialized) init(0);
        if (severity > m_level) return;
        int offset = ++m_last;
        if (offset < static_cast<int>(capacity))
        {
            LogMessage* msg = new(&m_messages[offset]) LogMessage(severity);
            msg->addTimestamp()->add(' ');
            switch (severity)
            {
            case LogMessage::error:
                msg->add("ERROR: ");
                break;
            case LogMessage::warning:
                msg->add("WARNING: ");
                break;
            case LogMessage::info:
                msg->add("INFO: ");
                break;
            default:
                break;
            }
            msg->printf(format, args...)->add("\r\n");
            if (!offset) send(offset);
        }
    }

public:

    static void init(UART_HandleTypeDef* huart, LogMessage::Severity lvl = LogMessage::LOG_LEVEL);

    static void deinit();

    static LogMessage::Severity level();

protected:

    static void send(int n);

    static void sendNext();

    static void tx_complete(UART_HandleTypeDef* huart);

public:

    /// @brief Maximum number of messages that can be kept in memory before they are sent.
    static constexpr size_t capacity = 256;

protected:

    static inline UART_HandleTypeDef* m_uart{};                      // Debug port.
    static inline bool m_isInitialized = false;                      // True if the debug module has been initialized.
    static inline LogMessage::Severity m_level = LogMessage::debug;  // Maximum severity to pass a message.
    static inline LogMessage* m_messages = new LogMessage[capacity]; // Message queue.
    static inline int m_last = 0;                                    // The last message index in the queue.
    static inline int m_lastSent = 0;                                // The last sent message index.

private:

    Debug() = delete;
    Debug(const Debug&) = delete;
    Debug& operator=(const Debug&) = delete;

};
