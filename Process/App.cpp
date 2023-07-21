/**
 * @file        App.cpp
 * @author      CodeDog
 * @brief       Main application process code.
 *
 * @remarks     Contains initialization code for the main application process.
 *
 * @copyright   (c)2023 CodeDog, All rights reserved.
 */

#include "App.hpp"
#include "Device.hpp"
#include "FS.hpp"

void App::init(void)
{
    static bool s_isInitialized = false;
    if (s_isInitialized) return;
    s_isInitialized = true;
    Device::init();
    FS::availableChanged = fsAvailableChanged;
    FS::init();
    Device::setBacklight(1);
    Device::beep();
}

void App::fsAvailableChanged()
{

}

EXTERN_C_BEGIN
#include "c_debug.h"

/**
 * @brief  Function implementing the SWDTask thread.
 */
void App_Task(void*)
{
    App::init();
    while (1)
    {
        Thread::tick(Thread::Default);  // Default main thread tick.
        osDelay(1);                     // Required to return control to the FreeRTOS scheduler.
    }
}

EXTERN_C_END
