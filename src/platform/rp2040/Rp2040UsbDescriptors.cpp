#include <cstddef>
#include <cstdint>

#include "pico/stdio_usb.h"
#include "pico/stdio_usb/reset_interface.h"
#include "pico/unique_id.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include "tusb.h"
#pragma GCC diagnostic pop

namespace
{
    // 3Dconnexion SpaceNavigator USB VID/PID so Autodesk Fusion 360 / 3DxWare
    // (as well as SolidWorks, OnShape, Blender, FreeCAD, and spacenavd)
    // automatically recognizes the device as a native 6-DOF 3D mouse.
    constexpr std::uint16_t kUsbVid = 0x046DU;  // Logitech / 3Dconnexion
    constexpr std::uint16_t kUsbPid = 0xC626U;  // SpaceNavigator 6-DOF 3D Mouse

    constexpr std::uint8_t kItfCdc      = 0U;   // occupies interfaces 0 and 1
    constexpr std::uint8_t kItfRpiReset = 2U;   // interface 2 (expected by reset_interface.c)
    constexpr std::uint8_t kItfHid      = 3U;   // interface 3 (6-DOF 3D Mouse HID)
    constexpr std::uint8_t kItfTotal    = 4U;

    constexpr std::uint8_t kEpCdcCmd = 0x81U;
    constexpr std::uint8_t kEpCdcOut = 0x02U;
    constexpr std::uint8_t kEpCdcIn  = 0x82U;
    constexpr std::uint8_t kEpHidIn  = 0x83U;

    constexpr std::uint8_t kStrLang    = 0U;
    constexpr std::uint8_t kStrManuf   = 1U;
    constexpr std::uint8_t kStrProduct = 2U;
    constexpr std::uint8_t kStrSerial  = 3U;
    constexpr std::uint8_t kStrCdc     = 4U;
    constexpr std::uint8_t kStrReset   = 5U;
    constexpr std::uint8_t kStrHid     = 6U;

    // Complete 3Dconnexion SpaceNavigator (0x046D:0xC626) HID Report Descriptor
    // required by 3DxWare (Fusion 360) and standard Multi-Axis HID consumers:
    //   Report ID 1: Translation X, Y, Z (3 x int16, range [-500, +500])
    //   Report ID 2: Rotation Rx, Ry, Rz (3 x int16, range [-500, +500])
    //   Report ID 3: Buttons 1..2 (2 bits + 14 bits padding)
    //   Report ID 4: LED Output (1 bit + 7 bits padding)
    const std::uint8_t kHidReportDescriptor[] = {
        0x05U, 0x01U,              // Usage Page (Generic Desktop Ctrls)
        0x09U, 0x08U,              // Usage (Multi-axis Controller)
        0xA1U, 0x01U,              // Collection (Application)

        // Report ID 1: Translation vector (X, Y, Z)
        0xA1U, 0x00U,              //   Collection (Physical)
        0x85U, 0x01U,              //     Report ID (1)
        0x16U, 0x0CU, 0xFEU,       //     Logical Minimum (-500)
        0x26U, 0xF4U, 0x01U,       //     Logical Maximum (+500)
        0x36U, 0x88U, 0xFAU,       //     Physical Minimum (-1400)
        0x46U, 0x78U, 0x05U,       //     Physical Maximum (+1400)
        0x55U, 0x0CU,              //     Unit Exponent (-4)
        0x65U, 0x11U,              //     Unit (SI Linear: cm)
        0x09U, 0x30U,              //     Usage (X)
        0x09U, 0x31U,              //     Usage (Y)
        0x09U, 0x32U,              //     Usage (Z)
        0x75U, 0x10U,              //     Report Size (16)
        0x95U, 0x03U,              //     Report Count (3)
        0x81U, 0x06U,              //     Input (Data,Var,Rel)
        0xC0U,                     //   End Collection

        // Report ID 2: Rotation vector (Rx, Ry, Rz)
        0xA1U, 0x00U,              //   Collection (Physical)
        0x85U, 0x02U,              //     Report ID (2)
        0x16U, 0x0CU, 0xFEU,       //     Logical Minimum (-500)
        0x26U, 0xF4U, 0x01U,       //     Logical Maximum (+500)
        0x36U, 0x88U, 0xFAU,       //     Physical Minimum (-1400)
        0x46U, 0x78U, 0x05U,       //     Physical Maximum (+1400)
        0x55U, 0x0CU,              //     Unit Exponent (-4)
        0x65U, 0x14U,              //     Unit (English Rotation: deg)
        0x09U, 0x33U,              //     Usage (Rx)
        0x09U, 0x34U,              //     Usage (Ry)
        0x09U, 0x35U,              //     Usage (Rz)
        0x75U, 0x10U,              //     Report Size (16)
        0x95U, 0x03U,              //     Report Count (3)
        0x81U, 0x06U,              //     Input (Data,Var,Rel)
        0xC0U,                     //   End Collection

        // Report ID 3: Buttons (2 buttons)
        0xA1U, 0x02U,              //   Collection (Logical)
        0x85U, 0x03U,              //     Report ID (3)
        0x05U, 0x01U,              //     Usage Page (Generic Desktop Ctrls)
        0x05U, 0x09U,              //     Usage Page (Button)
        0x19U, 0x01U,              //     Usage Minimum (Button 1)
        0x29U, 0x02U,              //     Usage Maximum (Button 2)
        0x15U, 0x00U,              //     Logical Minimum (0)
        0x25U, 0x01U,              //     Logical Maximum (1)
        0x35U, 0x00U,              //     Physical Minimum (0)
        0x45U, 0x01U,              //     Physical Maximum (1)
        0x75U, 0x01U,              //     Report Size (1)
        0x95U, 0x02U,              //     Report Count (2)
        0x81U, 0x02U,              //     Input (Data,Var,Abs)
        0x95U, 0x0EU,              //     Report Count (14)
        0x81U, 0x03U,              //     Input (Const,Var,Abs)
        0xC0U,                     //   End Collection

        // Report ID 4: LED Indicator Output
        0xA1U, 0x02U,              //   Collection (Logical)
        0x85U, 0x04U,              //     Report ID (4)
        0x05U, 0x08U,              //     Usage Page (LEDs)
        0x09U, 0x4BU,              //     Usage (Generic Indicator)
        0x15U, 0x00U,              //     Logical Minimum (0)
        0x25U, 0x01U,              //     Logical Maximum (1)
        0x35U, 0x00U,              //     Physical Minimum (0)
        0x45U, 0x01U,              //     Physical Maximum (1)
        0x75U, 0x01U,              //     Report Size (1)
        0x95U, 0x01U,              //     Report Count (1)
        0x91U, 0x02U,              //     Output (Data,Var,Abs)
        0x95U, 0x07U,              //     Report Count (7)
        0x91U, 0x03U,              //     Output (Const,Var,Abs)
        0xC0U,                     //   End Collection

        0xC0U                      // End Collection
    };

    const tusb_desc_device_t kDeviceDescriptor = {
        sizeof(tusb_desc_device_t),
        TUSB_DESC_DEVICE,
#if PICO_STDIO_USB_ENABLE_RESET_VIA_VENDOR_INTERFACE && PICO_STDIO_USB_RESET_INTERFACE_SUPPORT_MS_OS_20_DESCRIPTOR
        0x0210U,
#else
        0x0200U,
#endif
        TUSB_CLASS_MISC,
        MISC_SUBCLASS_COMMON,
        MISC_PROTOCOL_IAD,
        CFG_TUD_ENDPOINT0_SIZE,
        kUsbVid,
        kUsbPid,
        0x0435U,
        kStrManuf,
        kStrProduct,
        kStrSerial,
        1U
    };

    constexpr std::size_t kRpiResetDescLen = 9U;
    constexpr std::size_t kConfigTotalLen =
        TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + kRpiResetDescLen + TUD_HID_DESC_LEN;

    const std::uint8_t kConfigurationDescriptor[kConfigTotalLen] = {
        TUD_CONFIG_DESCRIPTOR(1, kItfTotal, kStrLang, kConfigTotalLen, 0x00, 250),

        TUD_CDC_DESCRIPTOR(kItfCdc, kStrCdc, kEpCdcCmd, 8, kEpCdcOut, kEpCdcIn, 64),

        // Raspberry Pi Vendor Reset interface (interface 2) for buttonless picotool flashing
        9, TUSB_DESC_INTERFACE, kItfRpiReset, 0, 0, TUSB_CLASS_VENDOR_SPECIFIC,
        RESET_INTERFACE_SUBCLASS, RESET_INTERFACE_PROTOCOL, kStrReset,

        // 6-DOF 3D Mouse HID interface (interface 3)
        TUD_HID_DESCRIPTOR(kItfHid, kStrHid, HID_ITF_PROTOCOL_NONE,
                           sizeof(kHidReportDescriptor), kEpHidIn, 16, 5)
    };

    char g_serialStr[PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2 + 1]{};

    const char* const kStringDescriptors[] = {
        "",
        "3Dconnexion",
        "SpaceNavigator",
        g_serialStr,
        "pico-3d-mouse Telemetry & Calibration CDC",
        "RP2040 Reset",
        "SpaceNavigator 6-DOF HID"
    };
}

extern "C"
{
    const std::uint8_t* tud_descriptor_device_cb()
    {
        return reinterpret_cast<const std::uint8_t*>(&kDeviceDescriptor);
    }

    const std::uint8_t* tud_descriptor_configuration_cb(std::uint8_t /*index*/)
    {
        return kConfigurationDescriptor;
    }

    const std::uint8_t* tud_hid_descriptor_report_cb(std::uint8_t /*instance*/)
    {
        return kHidReportDescriptor;
    }

    std::uint16_t tud_hid_get_report_cb(std::uint8_t /*instance*/,
                                        std::uint8_t report_id,
                                        hid_report_type_t /*report_type*/,
                                        std::uint8_t* buffer,
                                        std::uint16_t reqlen)
    {
        if (buffer == nullptr || reqlen == 0U)
        {
            return 0U;
        }
        for (std::uint16_t i = 0U; i < reqlen; ++i)
        {
            buffer[i] = 0U;
        }
        if ((report_id == 1U || report_id == 2U) && reqlen >= 6U)
        {
            return 6U;
        }
        if (report_id == 3U && reqlen >= 2U)
        {
            return 2U;
        }
        return reqlen;
    }

    void tud_hid_set_report_cb(std::uint8_t /*instance*/,
                               std::uint8_t /*report_id*/,
                               hid_report_type_t /*report_type*/,
                               const std::uint8_t* /*buffer*/,
                               std::uint16_t /*bufsize*/)
    {
    }

    const std::uint16_t* tud_descriptor_string_cb(std::uint8_t index, std::uint16_t /*langid*/)
    {
        constexpr std::size_t kMaxChars = 48U;
        static std::uint16_t descStr[kMaxChars]{};

        if (g_serialStr[0] == '\0')
        {
            pico_get_unique_board_id_string(g_serialStr, sizeof(g_serialStr));
        }

        std::size_t len = 0U;
        if (index == kStrLang)
        {
            descStr[1] = 0x0409U;
            len        = 1U;
        }
        else
        {
            constexpr std::size_t kCount = sizeof(kStringDescriptors) / sizeof(kStringDescriptors[0]);
            if (index >= kCount)
            {
                return nullptr;
            }
            const char* str = kStringDescriptors[index];
            while (len < (kMaxChars - 1U) && str[len] != '\0')
            {
                descStr[1U + len] = static_cast<std::uint16_t>(static_cast<unsigned char>(str[len]));
                ++len;
            }
        }

        descStr[0] = static_cast<std::uint16_t>((static_cast<std::uint16_t>(TUSB_DESC_STRING) << 8U) |
                                                static_cast<std::uint16_t>((2U * len) + 2U));
        return descStr;
    }
}
