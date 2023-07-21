/**
 * @file        SpiTest.hpp
 * @author      CodeDog
 * @brief       SPI connection test. Header file.
 * @remarks     Test the SPI link between 2 MCUs.
 *
 * @copyright   (c)2023 CodeDog, All rights reserved.
 */

#pragma once

#include "c_bindings.h"
EXTERN_C_BEGIN
#include <stdio.h>
#include "main.h"
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal_spi.h"
#include "stm32h7xx_hal_spi_ex.h"
#include "tim_ex.h"
EXTERN_C_END
#include "Async.hpp"

EXTERN_C SPI_HandleTypeDef hspi2;

/**
 * @brief Tests the SPI connection to another MCU capable of a compatible response.
 */
class SpiTest
{
public:

    SpiTest() = delete;
    SpiTest(const SpiTest& other) = delete;
    SpiTest(SpiTest&& other) = delete;

    /**
     * @fn bool isReady()
     * @brief Tests if the other device is ready, that is the HS pin is 1.
     * @return 1: Ready and waiting. 0: Not ready / not connected.
     */
    static bool isReady();

    /**
     * @brief Performs a non-blocking, DMA based test of the SPI link with ESP32 slave device.
     * @remarks
     *              TEST SEQUENCE:
     *              1. Master TX(A), slave TX(B).
     *              2. Test master RX(B), slave RX(A).
     *              3. If slave test completed successfully, X = A otherwise B.
     *              4. Master TX(B), slave TX(X).
     *              5. RESULT: master RX(A)
     *
     *              The result of this test will be logged.
     *
     * @returns Asynchronous result with `then` and `failed` continuations.
     */
    static AsyncResult* runAsync();

protected:

    /**
     * @brief Initializes the SPI test by assigning the appropriate system callbacks.
     */
    static void init();

    /**
     * @brief Deinitializes the SPI test by unregistering the registered system callbacks.
     */
    static void deinit();

    /**
     * @brief Resets the full state to initial.
     */
    static void reset();

    /**
     * @brief A finite state machine for the test, called each time the transaction state changes.
     */
    static void FSM();

    /**
     * @brief Completes the test successfully.
     */
    static void complete();

    /**
     * @brief Fails the test with an error.
     *
     * @param i Transaction index.
     */
    static void fail(uint8_t i);

    /**
     * @brief Sets the CS line state.
     *
     * @param state 1: H. 0: L.
     */
    static void CS(bool state);

    /**
     * @brief Gets the HS line state.
     *
     * @returns 1: H. 0: L.
     */
    static bool HS();

    /**
     * @brief Starts sending the sequence defined with XOR mask using DMA.
     *
     * @param mask A XOR mask to be applied to each 32-bit offset of the buffer.
     */
    static void TXRX_DMA(uint32_t mask);

    /**
     * @brief Called when the SPI DMA transaction completes.
     *
     * @param hspi SPI handle pointer.
     */
    static void TXRX_DMA_complete(SPI_HandleTypeDef* hspi);

    /**
     * @brief Tests if the receive buffer contains 32-bit offsets with the specified XOR mask applied.
     *
     * @param mask A XOR mask to be applied to each 32-bit offset of the buffer.
     * @return true The receive buffer contains expected data.
     * @return false The receive buffer contains something else.
     */
    static bool RX_check(uint32_t mask);

    /**
     * @brief Called when the SPI handshake line changes state.
     *
     * @param hs SPI handshake line state.
     */
    static void HS_EXTI(bool hs);

    /**
     * @brief Sets the timeout timer.
     */
    static void setTimeout();

    /**
     * @brief Clears the timeout timer.
     */
    static void clearTimeout();

    /**
     * @brief Called when a timeout occurs.
     */
    static void timeout(void*);

private:

    /**
     * @brief Test state.
     */
    enum TestState
    {
        TS_INIT,           // Ready.
        TS_TRANSACTION_1,  // Transaction 1 completed.
        TS_TRANSACTION_2,  // Transaction 2 completed.
    };

    static inline SPI_HandleTypeDef* m_hspi = &hspi2;      // Default SPI handle pointer.
    static inline GPIO_PinTypeDef* m_csPin = &PIN_SPI_CS;  // SPI CS PIN (Chip Select).
    static inline GPIO_PinTypeDef* m_hsPin = &PIN_SPI_HS;  // SPI HS PIN (Handshake).
    static constexpr size_t bufferSize = 2048;             // RX and TX buffer lengths.
    static constexpr uint32_t A = 0x00000000;              // XOR mask value for A packet.
    static constexpr uint32_t B = 0xffffffff;              // XOR mask value for B packet.
    static inline uint32_t T0 = 0;                         // Timer tick count for the start of the test.
    static inline double T = 0;                            // Test time in seconds.
    static inline double BR = 0;                           // Estimated baud rate in MBit/s.
    static constexpr double m_timeout_ms = 16.0;           // Operation timeout value in milliseconds.
    static inline AsyncResult* m_result{};                 // Asynchronous result of the `runAsync()` operation.
    static inline WaitHandleTypeDef* m_timeoutHandle{};    // Timeout handle pointer.
    static inline TestState m_state{};                     // Current test state.

    __attribute__((aligned(4))) static inline uint8_t m_txData[bufferSize]{};      // TX data buffer.
    __attribute__((aligned(4))) static inline uint8_t m_rxData[bufferSize + 1]{};  // RX data buffer. (+CRC)

};
