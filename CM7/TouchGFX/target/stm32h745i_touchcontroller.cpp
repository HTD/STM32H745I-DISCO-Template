#include "stm32h745i_touchcontroller.hpp"
#include "stm32h745i_discovery_ts.h"


static bool _bsp_ts_initialized = false;


/**
 * @brief initializes the touch controller
 */
void STM32H745ITouchController::init()
{
    static const uint32_t width = 480;
    static const uint32_t height = 272;

    TS_Init_t config;
    config.Width = width;
    config.Height = height;
    config.Orientation = TS_SWAP_XY;
    config.Accuracy = 0;

    if (BSP_TS_Init(0, &config) != BSP_ERROR_NONE)
    {
        return;
    }

    _bsp_ts_initialized = true;
}


/**
 * @brief retrieve touch information if a touch event is detected
 * @param x touch x coordinate
 * @param y touch y coordinate
 * @return \c true if a touch event was detected, else \c false
 */
bool STM32H745ITouchController::sampleTouch(int32_t& x, int32_t& y)
{
    static TS_State_t state;

    if (!_bsp_ts_initialized)
    {
        return false;
    }

    BSP_TS_GetState(0, &state);

    if (!state.TouchDetected)
    {
        return false;
    }

    x = state.TouchX;
    y = state.TouchY;
    return true;
}
