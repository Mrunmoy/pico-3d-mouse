#include "platform/rp2040/Rp2040GpioOutput.hpp"

#include "hardware/gpio.h"

namespace p3d::platform::rp2040
{
    Rp2040GpioOutput::Rp2040GpioOutput(std::uint32_t gpioPin)
        : m_gpioPin(gpioPin)
    {
    }

    bool Rp2040GpioOutput::init()
    {
        gpio_init(m_gpioPin);
        gpio_set_dir(m_gpioPin, GPIO_OUT);
        return true;
    }

    bool Rp2040GpioOutput::write(bool level)
    {
        gpio_put(m_gpioPin, level ? 1 : 0);
        return true;
    }
}
