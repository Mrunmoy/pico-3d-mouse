#pragma once

namespace p3d::hal
{
    // Pure hardware abstraction for software-triggered reboot into USB firmware update mode
    // (eliminates any need to press the physical BOOTSEL button inside the enclosure).
    class IBootControl
    {
    public:
        virtual ~IBootControl() = default;

        // Immediately reboots the MCU into the ROM USB UF2/picotool bootloader over USB-C.
        virtual void enterBootloader() = 0;
    };
}
