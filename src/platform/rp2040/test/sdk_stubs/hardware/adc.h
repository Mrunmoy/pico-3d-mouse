#pragma once

#include <cstdint>
#include "PicoSdkStubState.hpp"

inline void adc_init()
{
    p3d::test::sdk_stub::state().adcInitialized = true;
}

inline void adc_gpio_init(unsigned int gpio)
{
    p3d::test::sdk_stub::state().adcInitializedGpios.push_back(gpio);
}

inline void adc_select_input(unsigned int input)
{
    p3d::test::sdk_stub::state().selectedAdcInput = input;
}

inline std::uint16_t adc_read()
{
    return p3d::test::sdk_stub::state().nextAdcReadValue;
}
