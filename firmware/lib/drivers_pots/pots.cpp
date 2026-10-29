#include "drivers/pots.hpp"

#include <algorithm>
#include <cstdint>

#include "hardware/adc.h"
#include "hardware/gpio.h"

namespace drivers::pots
{
    namespace
    {
        constexpr std::uint32_t kAdcVrefMv   = 3300;
        constexpr std::uint32_t kAdcMaxCount = 4095;
        constexpr float         kHalfScale   = 2047.5f;
    }

    Reader::Reader(const Config& cfg)
        : m_cfg(cfg)
    {
    }

    void Reader::init()
    {
        adc_init();

        for (const auto& ch : m_cfg.channels)
        {
            adc_gpio_init(ch.gpio);
            gpio_disable_pulls(ch.gpio);
        }
    }

    void Reader::calibrateCenter(std::uint16_t numSamples)
    {
        const std::uint16_t n = (numSamples == 0) ? 1 : numSamples;
        std::array<std::uint32_t, kNumChannels> accum{ 0, 0, 0 };

        for (std::uint16_t s = 0; s < n; ++s)
        {
            for (std::size_t ch = 0; ch < kNumChannels; ++ch)
            {
                accum[ch] += readChannelRaw(m_cfg.channels[ch].adcInput, m_cfg.oversampleCount);
            }
        }

        for (std::size_t ch = 0; ch < kNumChannels; ++ch)
        {
            m_center[ch] = static_cast<std::uint16_t>(accum[ch] / n);
        }
    }

    Sample Reader::read() const
    {
        Sample out{};

        for (std::size_t ch = 0; ch < kNumChannels; ++ch)
        {
            const std::uint16_t raw =
                readChannelRaw(m_cfg.channels[ch].adcInput, m_cfg.oversampleCount);

            out.raw[ch] = raw;
            out.millivolts[ch] = static_cast<std::uint16_t>(
                (static_cast<std::uint32_t>(raw) * kAdcVrefMv) / kAdcMaxCount);

            const std::int32_t diff =
                static_cast<std::int32_t>(raw) - static_cast<std::int32_t>(m_center[ch]);
            const std::int32_t dz = static_cast<std::int32_t>(m_cfg.deadzoneCounts);

            std::int32_t adjusted = 0;
            if (diff > dz)
            {
                adjusted = diff - dz;
            }
            else if (diff < -dz)
            {
                adjusted = diff + dz;
            }

            out.delta[ch] = static_cast<std::int16_t>(adjusted);

            const float norm = static_cast<float>(adjusted) / kHalfScale;
            out.normalized[ch] = std::clamp(norm, -1.0f, 1.0f);
        }

        return out;
    }

    std::uint16_t Reader::readChannelRaw(std::uint32_t adcInput, std::uint8_t samples) const
    {
        adc_select_input(adcInput);

        // Discard one conversion after switching the ADC mux so the internal
        // sample-and-hold capacitor settles to the new potentiometer wiper voltage.
        (void)adc_read();

        const std::uint8_t n = (samples == 0) ? 1 : samples;
        std::uint32_t sum = 0;
        for (std::uint8_t i = 0; i < n; ++i)
        {
            sum += adc_read();
        }

        return static_cast<std::uint16_t>(sum / n);
    }
}
