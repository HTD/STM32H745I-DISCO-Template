/**
 * @file        PCM16S.hpp
 * @author      CodeDog
 * @brief       A 16-bit stereo PCM sample type. Header file.
 *
 * @copyright   (c)2023 CodeDog, All rights reserved.
 */

#pragma once

#include <stdint.h>

/**
 * @brief Internal 2 16-bit values, packed.
 *
 */
struct __attribute__((__packed__)) __PCM16S_S
{
    int16_t left;   // Left channel sample value.
    int16_t right;  // Right channel sample value.
};

/**
 * @brief Internal union of 2 packed 16-bit values.
 */
union __PCM16S_U
{
    __PCM16S_S channels;    // Channels separated.
    uint32_t value;         // Channels combined.
};

/**
 * @brief PCM16 stereo sample.
 */
struct PCM16S
{
    /**
     * @brief Assigns a 16-bit sample value to both channels.
     *
     * @param value 16-bit sample value.
     */
    PCM16S& operator=(int16_t value);

    /**
     * @brief Assings a 32-bit other sample value.
     *
     * @param value 32-bit containing data for both channels.
     */
    PCM16S& operator=(uint32_t value);

    /**
     * @brief Assings a normalized value from [-1.0 .. 1.0] range as the sample value for both channels.
     *
     * @param normalized A normalized sample value from [-1.0 .. 1.0] range.
     */
    PCM16S& operator=(double normalized);

    /**
     * @brief Assings a normalized value from [-1.0 .. 1.0] range as the sample value for both channels.
     *
     * @param normalized A normalized sample value from [-1.0 .. 1.0] range.
     */
    PCM16S& operator=(float normalized);

    /**
     * @brief Gets the underlying 32-bit combined sample data.
     */
    operator uint32_t();

    __PCM16S_U sample; // Sample data.

};
