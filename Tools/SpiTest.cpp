/**
 * @file        SpiTest.cpp
 * @author      CodeDog
 * @brief       SPI connection test.
 * @remarks     Test the SPI link between 2 MCUs.
 *
 * @copyright   (c)2023 CodeDog, All rights reserved.
 */

#include "hal_ex.h"
#include "pins_conf.h"
#include "Async.hpp"
#include "Debug.hpp"
#include "SpiTest.hpp"
#include "Counter.hpp"

//
// API
//

bool SpiTest::isReady()
{
    return HAL_GPIO_ReadPin(SPI_HS_GPIO_Port, SPI_HS_Pin) == GPIO_PIN_SET;
}

AsyncResult *SpiTest::runAsync()
{
    if (m_result) return m_result;
    init();
    m_result = Async::createResult();
    m_state = TS_INIT;
    FSM();
    return m_result;
}

//
// Implementation
//

void SpiTest::init()
{
    HAL_EXTI_RegisterCallbackEx(&PIN_SPI_HS, HS_EXTI);
    HAL_SPI_RegisterCallback(m_hspi, HAL_SPI_TX_RX_COMPLETE_CB_ID, TXRX_DMA_complete);
}

void SpiTest::deinit()
{
    HAL_EXTI_UnregisterCallbackEx(&PIN_SPI_HS);
    HAL_SPI_UnRegisterCallback(m_hspi, HAL_SPI_TX_RX_COMPLETE_CB_ID);
}

void SpiTest::reset()
{
    CS(1);
    clearTimeout();
    deinit();
    m_state = TS_INIT;
}

void SpiTest::FSM()
{
    switch (m_state)
    {
    case TS_INIT:
        T0 = Counter::getTicks();
        TXRX_DMA(A);
        m_state = TS_TRANSACTION_1;
        break;
    case TS_TRANSACTION_1:
        if (RX_check(B))
        {
            m_state = TS_TRANSACTION_2;
            TXRX_DMA(B);
        }
        else fail(1);
        break;
    case TS_TRANSACTION_2:
        if (RX_check(A)) complete(); else fail(2);
        break;
    }
}

void SpiTest::complete()
{
    T = Counter::getTime(T0);
    BR = 1.0 / (32.0 * T);
    reset();
    Debug::log(LogMessage::info, "SPI link operational.");
    Debug::dump("Test completed in %.3f ms, approx. baud rate %.3f Mbit/s.", T * 1000.0, BR);
    Async::complete(&m_result);
}

void SpiTest::fail(uint8_t i)
{
    reset();
    Debug::log(LogMessage::error, "SPI transaction RX%i.", i);
    Async::fail(&m_result);
}

void SpiTest::CS(bool state) { if (state) HAL_EX_PIN_SET_H(m_csPin); else HAL_EX_PIN_SET_L(m_csPin); }

bool SpiTest::HS() { return HAL_EX_PIN_GET_STATE(m_hsPin); }

void SpiTest::TXRX_DMA(uint32_t mask)
{
    auto target32 = reinterpret_cast<uint32_t*>(m_txData);
    auto length = bufferSize >> 2;
    for (size_t i = 0; i < length; i++) target32[i] = i xor mask;
    memset(m_rxData, 0, bufferSize + 1);
    CS(0);
    HAL_SPI_TransmitReceive_DMA(m_hspi, m_txData, m_rxData, bufferSize);
    setTimeout();
}

void SpiTest::TXRX_DMA_complete(SPI_HandleTypeDef*) { CS(1); }

bool SpiTest::RX_check(uint32_t mask)
{
    auto target32 = reinterpret_cast<uint32_t*>(m_rxData);
    auto length = bufferSize >> 2;
    for (size_t i = 0; i < length; i++) if (target32[i] != (i xor mask)) return false;
    return true;
}

void SpiTest::HS_EXTI(bool hs) { if (!HS()) clearTimeout(); else FSM(); }

void SpiTest::setTimeout()
{
    if (m_timeoutHandle) clearTimeout();
    m_timeoutHandle = delay_async(m_timeout_ms, nullptr, timeout);
}

void SpiTest::clearTimeout()
{
    wait_handle_dispose(m_timeoutHandle);
    m_timeoutHandle = nullptr;
}

void SpiTest::timeout(void*)
{
    reset();
    if (m_result) Async::fail(&m_result);
    Debug::log(LogMessage::error, "SPI test timed out.");
}
