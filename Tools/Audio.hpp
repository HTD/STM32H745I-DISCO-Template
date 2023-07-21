/**
 * @file        Audio.hpp
 * @author      CodeDog
 * @brief       WM8994 SAI beeper. Header file.
 *
 * @remarks     The `init()` function must be called at the end of `MX_SAI2_Init()`.
 *
 * @copyright   (c)2023 CodeDog, All rights reserved.
 */

#pragma once

#include <cstddef>
#include <cstdint>
#include "stm32h745i_discovery_audio.h"
#include "PCM16S.hpp"
#include "Tone.hpp"

extern SAI_HandleTypeDef haudio_out_sai;
extern SAI_HandleTypeDef hsai_BlockA2;

/**
 * @brief Simple WM8994 SAI beeper.
 */
class Audio
{
public:

    using TT = Tone<PCM16S>; // Tone type.

    /// @brief The number of predefined tones.
    static constexpr uint8_t tonesCount = 2;

    /// @brief Predefined tones pointers.
    static inline TT* tones[tonesCount]{};

public:
    Audio() = delete;
    Audio(const Audio& other) = delete;
    Audio(Audio&& other) = delete;

public:

    /**
     * @brief Initializes the audio output hardware.
     *
     * @param sampleRate WM8994 supported sample rate.
     * @return 1: Success. 0: Initialization error.
     */
    static bool init(uint32_t sampleRate = WM8994_FREQUENCY_22K);

    /**
     * @brief Plays a beep sound. DO NOT CALL FROM ISR!
     *
     * @param toneIndex 0: Quiet system beep. 1: Loud alert beep. Default 0 - system beep.
     * @param time Beep time in seconds, zero plays sound until stopped with `stop()`. Default `0.1`.
     */
    static void beep(uint8_t toneIndex = 0, double time = 0.1);

    /// @returns 1: Playing sound. 0: Not playing.
    static bool isPlaying();

    /**
     * @brief Stops the audio playback. DO NOT CALL FROM ISR!
     */
    static void stop();

    /**
     * @brief Gets the current sound volume.
     * @return A value in <0..100> range.
     */
    static uint8_t getVolume();

    /**
     * @brief Sets the current sound volume.
     * @param volume A value in <0..100> range.
     */
    static void setVolume(uint8_t volume);

protected:

    static constexpr uint8_t commandRetry = 255;// The number of attempts of talking to audio device with SAI.
    static inline uint32_t m_sampleRate = 0;    // Sample rate in samples per second.
    static inline bool m_isInitialized = 0;     // 1: Initialized. 0: Not initialized.
    static inline bool m_isPlaying = 0;         // 1: Playback is started. 0: Playback is stopped / not started.
    static inline uint8_t m_volume = 100;       // Default sound volume [0..100]

};
