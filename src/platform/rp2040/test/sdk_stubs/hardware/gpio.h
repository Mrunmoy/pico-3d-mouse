#pragma once

#include <cstdint>
#include "PicoSdkStubState.hpp"

inline constexpr bool GPIO_OUT = true;
inline constexpr bool GPIO_IN  = false;

inline void gpio_init(unsigned int gpio)
{
    p3d::test::sdk_stub::state().lastGpioInitPin = gpio;
}

inline void gpio_set_dir(unsigned int /*gpio*/, bool out)
{
    p3d::test::sdk_stub::state().lastGpioDirOut = out;
}

inline void gpio_put(unsigned int gpio, bool value)
{
    p3d::test::sdk_stub::state().lastGpioPutPin   = gpio;
    p3d::test::sdk_stub::state().lastGpioPutLevel = value;
}

inline void gpio_disable_pulls(unsigned int gpio)
{
    p3d::test::sdk_stub::state().pullsDisabledGpios.push_back(gpio);
}
