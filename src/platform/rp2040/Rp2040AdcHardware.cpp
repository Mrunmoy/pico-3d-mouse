#include "platform/rp2040/Rp2040AdcHardware.hpp"

#include "hardware/adc.h"
#include "hardware/gpio.h"

namespace p3d::platform::rp2040
{
    Rp2040AdcHardware::Rp2040AdcHardware(
        const std::array<std::uint32_t, pins::kNumAdcChannels>& channelGpios)
        : m_channelGpios(channelGpios)
    {
    }

    bool Rp2040AdcHardware::init()
    {
        adc_init();
        m_initialized = true;
        return true;
    }

    bool Rp2040AdcHardware::initChannel(std::uint8_t channel)
    {
        if (channel >= pins::kNumAdcChannels)
        {
            return false;
        }
        if (!m_initialized)
        {
            adc_init();
            m_initialized = true;
        }
        const std::uint32_t gpio = m_channelGpios[channel];
        adc_gpio_init(gpio);
        gpio_disable_pulls(gpio);
        return true;
    }

    bool Rp2040AdcHardware::selectChannel(std::uint8_t channel)
    {
        if (channel >= pins::kNumAdcChannels)
        {
            return false;
        }
        adc_select_input(channel);
        return true;
    }

    bool Rp2040AdcHardware::readRaw(std::uint16_t& outCounts)
    {
        outCounts = adc_read();
        return true;
    }
}
