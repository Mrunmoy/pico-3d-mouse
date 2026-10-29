#pragma once

#include <cstdint>
#include "PicoSdkStubState.hpp"

inline void reset_usb_boot(std::uint32_t /*usb_activity_gpio_pin_mask*/,
                           std::uint32_t /*disable_interface_mask*/)
{
    p3d::test::sdk_stub::state().bootloaderEntered = true;
}
