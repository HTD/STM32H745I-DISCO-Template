/**
 * @file        PCM16S.cpp
 * @author      CodeDog
 * @brief       A 16-bit stereo PCM sample type.
 *
 * @copyright   (c)2023 CodeDog, All rights reserved.
 */

#include "PCM16S.hpp"
#include <cmath>

PCM16S& PCM16S::operator=(int16_t value)
{
    sample.channels.left = value;
    sample.channels.right = value;
    return *this;
}

PCM16S &PCM16S::operator=(uint32_t value)
{
    sample.value = value;
    return *this;
}

PCM16S& PCM16S::operator=(double normalized)
{
    if (normalized < -1.0) normalized = -1.0;
    else if (normalized > 1.0) normalized = 1.0;
    int16_t value = static_cast<int16_t>(round(0x7fff * normalized));
    sample.channels.left = value;
    sample.channels.right = value;
    return *this;
}

PCM16S &PCM16S::operator=(float normalized)
{
    if (normalized < -1.0f) normalized = -1.0f;
    else if (normalized > 1.0f) normalized = 1.0f;
    int16_t value = static_cast<int16_t>(round(0x7fff * normalized));
    sample.channels.left = value;
    sample.channels.right = value;
    return *this;
}

PCM16S::operator uint32_t()
{
    return sample.value;
}
