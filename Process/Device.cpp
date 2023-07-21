/**
 * @file        Device.cpp
 * @author      CodeDog
 * @brief       Controls complex hardware behaviors.
 *
 * @copyright   (c)2023 CodeDog, All rights reserved.
 */

#include "main.h"
#include "Counter.hpp"
#include "Device.hpp"
#include "Thread.hpp"
#include "Audio.hpp"
#include "Debug.hpp"

void Device::init(void)
{
    TIM_EX_init(&DELAY_TIM, APB_CLOCK, DELAY_RESOLUTION);
    Audio::init();
    Audio::setVolume(100);
}

void Device::setBacklight(bool value)
{
    HAL_GPIO_WritePin(LCD_BL_GPIO_Port, LCD_BL_Pin, value ? GPIO_PIN_SET : GPIO_PIN_RESET);
    Debug::log(LogMessage::info, value ? "Lights on!" : "Lights off!");
}

void Device::beep()
{
    Audio::beep();
    Debug::log(LogMessage::info, "BEEP!");
}

bool Device::timeSet() { return m_isTimeSet; }

void Device::setTime(DateTime& time)
{
    DateTimeEx last(1);
    if (reinterpret_cast<DateTimeEx&>(time).setRTC())
    {
        m_isTimeSet = true;
        Debug::log(LogMessage::info, "Time set successfully.");
    }
    else
    {
        Debug::log(LogMessage::error, "Could not set the system RTC!");
    }
}
