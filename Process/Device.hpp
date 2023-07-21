/**
 * @file        Device.hpp
 * @author      CodeDog
 * @brief       Controls complex hardware behaviors. Header file.
 *
 * @copyright   (c)2023 CodeDog, All rights reserved.
 */

#pragma once

#include "c_bindings.h"
#include "main.h"
#include "tim_ex.h"
#include "cmsis_os2.h"
#include "DateTimeEx.hpp"

/// @brief Controls complex hardware behaviors.
class Device final
{

public:

    Device() = delete;
    Device(const Device&) = delete;
    Device(const Device&&) = delete;

    /// @brief Initializes the hadrware configuration.
    static void init(void);

    /// @brief Sets the LCD screen backlight.
    static void setBacklight(bool value);

    /// @brief Makes a system BEEP sound.
    static void beep();

    /// @returns A value indicating that the system time is set.
    static bool timeSet();

    /// @brief Sets the system time.
    static void setTime(DateTime& time);


protected:
    static inline bool m_isInitialized = 0;             // The device completed the initialization.
    static inline bool m_isTimeSet = 0;                 // The system time has been set.
    static inline DateTimeEx m_timeInitialized = 0;     // The system time value the device was initialized.

};

EXTERN_C_BEGIN
extern TIM_HandleTypeDef htim7;     // Microtime timer handle defined in main.c.
extern TIM_HandleTypeDef htim17;    // Delay timer handle defined in main.c.
EXTERN_C_END
