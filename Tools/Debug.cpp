/**
 * @file        Debug.cpp
 * @author      CodeDog
 * @brief       Provides serial port debugging.
 *
 * @copyright   (c)2023 CodeDog, All rights reserved.
 */

#include "Debug.hpp"

/**
 * @brief Initializes the serial port debugging module.
 *
 * @param huart UART handle pointer.
 */
void Debug::init(UART_HandleTypeDef* huart, LogMessage::Severity lvl)
{
    static bool s_isInitializing = false;
    if (s_isInitializing) return;
    s_isInitializing = true;
    m_level = lvl;
    if (!m_messages) m_messages = new LogMessage[capacity];
    if (!m_isInitialized) m_last = -1;
    if (!m_uart && huart)
    {
        m_uart = huart;
        HAL_UART_RegisterCallback(m_uart, HAL_UART_TX_COMPLETE_CB_ID, tx_complete);
        sendNext();
    }
    s_isInitializing = false;
    m_isInitialized = true;
}

/**
 * @brief Frees all resources allocated for the debug module.
 */
void Debug::deinit()
{
    if (m_messages) delete[] m_messages;
    m_messages = 0;
    m_last = m_lastSent = -1;
    HAL_UART_UnRegisterCallback(m_uart, HAL_UART_TX_COMPLETE_CB_ID);
    m_uart = nullptr;
}

/**
 * @brief Gets the current serverity level for the log.
 *
 * @return Severity level.
 */
LogMessage::Severity Debug::level() { return m_level; }

/**
 * @brief Sends a single message to the serial port in DMA mode.
 *
 * @param n Message queue index.
 */
void Debug::send(int n)
{
    static bool isSending = false;
    if (isSending || n > m_last) return;
    isSending = true;
    m_lastSent = n;
    LogMessage* message = &m_messages[n];
    auto [buffer, length] = message->buffer();
    if (!length) return;
    if (m_uart) HAL_UART_Transmit_DMA(m_uart, buffer, length);
    isSending = false;
}

/**
 * @brief Sends the next message in the queue.
 */
void Debug::sendNext()
{
    if (m_lastSent < m_last) send(++m_lastSent);
    else m_last = m_lastSent = -1;
}

/**
 * @brief   Handles message TX complete interrupt.
 * @remarks Deletes the last sent message, removes it from the queue, sends the next message if the message queue is not empty.
 *
 * @param huart UART handle pointer.
 */
void Debug::tx_complete(UART_HandleTypeDef* huart)
{
    LogMessage* message = &m_messages[m_lastSent];
    if (!message->buffer().second) return;
    message->clear();
    sendNext();
}

#include "c_bindings.h"

EXTERN_C_BEGIN

#include <stdarg.h>
#include <string.h>
#include "c_debug.h"

void debug_init(UART_HandleTypeDef* huart) { Debug::init(huart); }

void debug(uint8_t severity, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    const size_t lineBufferSize = 255;
    char lineBuffer[lineBufferSize];
    vsnprintf(lineBuffer, lineBufferSize, format, args);
    va_end(args);
    Debug::log((LogMessage::Severity)severity, lineBuffer);
}

void red_light_on()
{
    HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, GPIO_PIN_RESET);
}

void red_light_off()
{
    HAL_GPIO_WritePin(LD6_GPIO_Port, LD6_Pin, GPIO_PIN_SET);
}

EXTERN_C_END
