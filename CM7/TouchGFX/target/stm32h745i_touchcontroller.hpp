#ifndef STM32H745I_TOUCHCONTROLLER_HPP
#define STM32H745I_TOUCHCONTROLLER_HPP

#include <cstdint>
#include <stdint.h>


namespace STM32H745ITouchController
{

//! initialize touch controller
void init();

//! sample a single touch event
bool sampleTouch(int32_t& x, int32_t& y);

}

#endif
