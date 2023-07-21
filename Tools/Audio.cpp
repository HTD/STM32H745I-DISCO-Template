/**
 * @file        Audio.cpp
 * @author      CodeDog
 * @brief       WM8994 SAI beeper.
 *
 * @copyright   (c)2023 CodeDog, All rights reserved.
 */

#include "Audio.hpp"
#include "Thread.hpp"
#include "Delay.hpp"
#include "Debug.hpp"

bool Audio::init(uint32_t sampleRate) {
    if (m_isInitialized) return false;
    m_sampleRate = sampleRate;
    BSP_AUDIO_Init_t audio_init{};
    audio_init.Device = AUDIO_OUT_DEVICE_SPK_HP,
    audio_init.SampleRate = m_sampleRate,
    audio_init.BitsPerSample = AUDIO_RESOLUTION_16B,
    audio_init.ChannelsNbr = 2,
    audio_init.Volume = 100;
    int32_t result = BSP_ERROR_NONE;
    uint8_t attempts = commandRetry;
    do { result = BSP_AUDIO_OUT_Init(0, &audio_init); osDelay(1); }
    while (result != BSP_ERROR_NONE && --attempts > 0);
    m_isInitialized = result == BSP_ERROR_NONE;
    if (m_isInitialized)
    {
        Debug::log("Audio initialization PASSED.");
        TT::setSampleRate(sampleRate);
        tones[0] = new TT(1000, -10.0, TT::Square);
        tones[1] = new TT(1000, 0.0, TT::Sine);
    }
    else Debug::log(LogMessage::error, "Audio initialization FAILED!");
    return m_isInitialized;
}

void Audio::beep(uint8_t toneIndex, double time)
{
    if (!m_isInitialized || !m_volume || m_isPlaying ||
        toneIndex >= tonesCount || !tones[toneIndex] || Thread::fromISR()) return;
    auto tone = tones[toneIndex];
    int32_t result = BSP_ERROR_NONE;
    uint8_t attempts = commandRetry;
    do { result = BSP_AUDIO_OUT_Play(0, tone->data(), tone->size()); osDelay(1); }
    while (result != BSP_ERROR_NONE && --attempts > 0);
    if (result != BSP_ERROR_NONE)
        Debug::log(LogMessage::error, "BSP_AUDIO_OUT_Play() failed with error code 0x%x!", result);
    // Yes, even if it failed, let's give one last chance for the driver to stop the playback.
    m_isPlaying = true;
    if (time > 0) Delay::set(static_cast<uint32_t>(1000.0 * time), stop);
}

bool Audio::isPlaying()
{
    return m_isPlaying;
}

void Audio::stop()
{
    if (!m_isInitialized || !m_isPlaying || Thread::fromISR()) return;
    int32_t result = BSP_ERROR_NONE;
    uint8_t attempts = commandRetry;
    do { result = BSP_AUDIO_OUT_Stop(0); osDelay(1); }
    while (result != BSP_ERROR_NONE && --attempts > 0);
    if (result == BSP_ERROR_NONE)
    {
        m_isPlaying = false;
    }
    else
    {
        Debug::log(LogMessage::error, "BSP_AUDIO_OUT_Stop() failed with error code %i!", result);
    }
}

uint8_t Audio::getVolume()
{
    return m_volume;
}

void Audio::setVolume(uint8_t volume)
{
    if (!m_isInitialized) return;
    int32_t result = BSP_ERROR_NONE;
    uint8_t attempts = commandRetry;
    do { result = BSP_AUDIO_OUT_SetVolume(0, m_volume = volume); }
    while (result != BSP_ERROR_NONE && --attempts > 0);
    if (result != BSP_ERROR_NONE)
    {
        m_isInitialized = false; // ...So it at least shuts up.
        Debug::log(LogMessage::error, "BSP_AUDIO_OUT_SetVolume() failed with error code %i!", result);
    }
}
