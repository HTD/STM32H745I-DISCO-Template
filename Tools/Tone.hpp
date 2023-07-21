/**
 * @file        Tone.hpp
 * @author      CodeDog
 * @brief       A simple tone generator for any sample type that can be set from a `double` normalized value.
 *
 * @copyright   (c)2023 CodeDog, All rights reserved.
 */

#pragma once

#include <cstdint>
#include <cmath>
#include "ThreadSafeAllocator.hpp"

/**
 * @brief Represents a single period of an audio tone.
 */
template<typename TSample>
class Tone final : public ThreadSafeAllocator
{

public:

    /**
     * @brief Waveform shape.
     */
    enum Waveform { Sine, Square };

public:

    /**
     * @brief Creates a tone.
     *
     * @param frequency Frequency in Hz.
     * @param level Sound volume level in dB where 0 is full volume. Default 0.
     * @param waveform Waveform type. Default `Sine`.
     */
    Tone(uint32_t frequency, double level = 0, Waveform waveform = Sine)
    {
        m_length = m_sampleRate / frequency;
        m_samples = new TSample[m_length];
        double normalized = 0;
        for (int32_t i = 0; i < m_length; i++)
        {
            switch (waveform)
            {
            case Sine:
                normalized = sin(_D_PI * i / static_cast<double>(m_length));
                break;
            case Square:
                normalized = 1 - (((i << 1) / m_length) << 1);
                break;
            }
            if (level < 0) normalized *= pow(10.0, 0.05 * level);
            m_samples[i] = normalized;
        }
    }

    Tone(const Tone& other) : m_length(other.m_length), m_samples()
    {
        m_samples = new TSample[m_length];
        for (size_t i = 0; i < m_length; i++) m_samples[i] = other.m_samples[i];
    }

    Tone(Tone&& other) : m_length(other.m_length), m_samples(other.m_samples)
    {
        delete[] other.m_samples;
        other.m_samples = nullptr;
        other.m_length = 0;
    }

    Tone& operator=(const Tone& other) noexcept
    {
        if (other == *this) return *this;
        if (m_samples) delete[] m_samples;
        m_samples = new TSample[m_length];
        for (size_t i = 0; i < m_length; i++) m_samples[i] = other.m_samples[i];
        m_length = other.m_length;
    }

    Tone& operator=(Tone&& other) noexcept
    {
        if (other == *this) return *this;
        if (m_samples) delete[] m_samples;
        m_samples = other.m_samples;
        delete[] other.m_samples;
        other.m_samples = nullptr;
        other.m_length = 0;
    }

    /// @brief Frees the memory if the instance goes out of scope.
    ~Tone() { delete[] m_samples; }

    /// @brief Sets the sample rate (in samples per second) used to generate tones.
    static void setSampleRate(uint32_t sampleRate) { m_sampleRate = sampleRate; }

    /// @brief Returns the data buffer pointer.
    template<typename T = uint8_t*>
    T data() { return reinterpret_cast<T>(m_samples); }

    /// @brief Returns the data buffer size in bytes.
    uint32_t size() { return static_cast<uint32_t>(m_length * sizeof(TSample)); }

    /// @brief Returns the number of samples.
    uint32_t length() { return static_cast<uint32_t>(m_length); }

    /// @brief Returns a specific sample reference.
    TSample& operator[](uint32_t i) { return m_samples[i % m_length]; }

private:
    static inline uint32_t m_sampleRate = 22050;           // Sample rate used to generate tones.
    int32_t m_length;                                      // The length of the tone in samples.
    TSample* m_samples;                                    // Samples buffer pointer.

    static constexpr double _PI = 3.14159265358979323846;  // π
    static constexpr double _D_PI = 2.0 * _PI;             // 2π

};
