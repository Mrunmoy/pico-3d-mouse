#include "platform/rp2040/Rp2040BootControl.hpp"

#include "pico/bootrom.h"

namespace p3d::platform::rp2040
{
    void Rp2040BootControl::enterBootloader()
    {
        reset_usb_boot(0U, 0U);
    }
}
