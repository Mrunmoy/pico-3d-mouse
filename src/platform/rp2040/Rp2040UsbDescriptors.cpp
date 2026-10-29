#include <cstddef>
#include <cstdint>

#include "pico/stdio_usb.h"
#include "pico/unique_id.h"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
#include "tusb.h"
#pragma GCC diagnostic pop

namespace
{
    // 3Dconnexion SpaceMouse Pro Wireless (cabled) USB VID/PID:
    // Supported natively by current Windows 10/11 & macOS 3DxWare 10 drivers
    // (Autodesk Fusion 360, SolidWorks, Rhino, OnShape) as well as Linux spacenavd/FreeCAD/Blender.
    constexpr std::uint16_t kUsbVid = 0x256FU;  // 3Dconnexion
    constexpr std::uint16_t kUsbPid = 0xC631U;  // SpaceMouse Pro Wireless (cabled)

    constexpr std::uint8_t kItfHid   = 0U;      // interface 0 (6-DOF 3Dconnexion HID IN + OUT on MI_00 / EP1)
    constexpr std::uint8_t kItfCdc   = 1U;      // occupies interfaces 1 and 2 (Windows inbox usbser.sys / Linux cdc_acm)
    constexpr std::uint8_t kItfTotal = 3U;

    constexpr std::uint8_t kEpHidOut = 0x01U;
    constexpr std::uint8_t kEpHidIn  = 0x81U;
    constexpr std::uint8_t kEpCdcCmd = 0x82U;
    constexpr std::uint8_t kEpCdcOut = 0x03U;
    constexpr std::uint8_t kEpCdcIn  = 0x83U;

    constexpr std::uint8_t kStrLang    = 0U;
    constexpr std::uint8_t kStrManuf   = 1U;
    constexpr std::uint8_t kStrProduct = 2U;
    constexpr std::uint8_t kStrSerial  = 3U;
    constexpr std::uint8_t kStrCdc     = 4U;
    constexpr std::uint8_t kStrHid     = 5U;

    // 3Dconnexion SpaceMouse Pro Wireless (cabled) (0x256F:0xC631) HID Report Descriptor:
    //   Report ID 1: 6-DOF Translation + Rotation (X, Y, Z, Rx, Ry, Rz) as 6 x int16 in [-350, +350] (12 bytes)
    //   Report ID 3: 32 Buttons (4 bytes)
    //   Report ID 4: LED Output (1 byte)
    const std::uint8_t kHidReportDescriptor[] = {
        0x05U, 0x01U,              // Usage Page (Generic Desktop)
        0x09U, 0x08U,              // Usage (Multi-Axis Controller)
        0xA1U, 0x01U,              // Collection (Application)

        // Report ID 1: 6-DOF Translation + Rotation (X, Y, Z, Rx, Ry, Rz)
        0xA1U, 0x00U,              //   Collection (Physical)
        0x85U, 0x01U,              //     Report ID (1)
        0x16U, 0xA2U, 0xFEU,       //     Logical Minimum (-350)
        0x26U, 0x5EU, 0x01U,       //     Logical Maximum (+350)
        0x36U, 0x88U, 0xFAU,       //     Physical Minimum (-1400)
        0x46U, 0x78U, 0x05U,       //     Physical Maximum (+1400)
        0x55U, 0x0CU,              //     Unit Exponent (-4)
        0x65U, 0x11U,              //     Unit (SI Linear: cm)
        0x09U, 0x30U,              //     Usage (X)
        0x09U, 0x31U,              //     Usage (Y)
        0x09U, 0x32U,              //     Usage (Z)
        0x09U, 0x33U,              //     Usage (Rx)
        0x09U, 0x34U,              //     Usage (Ry)
        0x09U, 0x35U,              //     Usage (Rz)
        0x75U, 0x10U,              //     Report Size (16)
        0x95U, 0x06U,              //     Report Count (6)
        0x81U, 0x02U,              //     Input (Data,Var,Abs)
        0xC0U,                     //   End Collection

        // Report ID 3: Keys (32 buttons)
        0xA1U, 0x00U,              //   Collection (Physical)
        0x85U, 0x03U,              //     Report ID (3)
        0x15U, 0x00U,              //     Logical Minimum (0)
        0x25U, 0x01U,              //     Logical Maximum (1)
        0x75U, 0x01U,              //     Report Size (1)
        0x95U, 0x20U,              //     Report Count (32)
        0x05U, 0x09U,              //     Usage Page (Button)
        0x19U, 0x01U,              //     Usage Minimum (Button 1)
        0x29U, 0x20U,              //     Usage Maximum (Button 32)
        0x81U, 0x02U,              //     Input (Data,Var,Abs)
        0xC0U,                     //   End Collection

        // Report ID 4: LEDs
        0xA1U, 0x02U,              //   Collection (Logical)
        0x85U, 0x04U,              //     Report ID (4)
        0x05U, 0x08U,              //     Usage Page (LEDs)
        0x09U, 0x4BU,              //     Usage (Generic Indicator)
        0x15U, 0x00U,              //     Logical Minimum (0)
        0x25U, 0x01U,              //     Logical Maximum (1)
        0x95U, 0x01U,              //     Report Count (1)
        0x75U, 0x01U,              //     Report Size (1)
        0x91U, 0x02U,              //     Output (Data,Var,Abs)
        0x95U, 0x01U,              //     Report Count (1)
        0x75U, 0x07U,              //     Report Size (7)
        0x91U, 0x03U,              //     Output (Const,Var,Abs)
        0xC0U,                     //   End Collection

        0xC0U                      // End Collection
    };

    const tusb_desc_device_t kDeviceDescriptor = {
        sizeof(tusb_desc_device_t),
        TUSB_DESC_DEVICE,
        0x0200U,
        TUSB_CLASS_MISC,
        MISC_SUBCLASS_COMMON,
        MISC_PROTOCOL_IAD,
        CFG_TUD_ENDPOINT0_SIZE,
        kUsbVid,
        kUsbPid,
        0x0100U,
        kStrManuf,
        kStrProduct,
        kStrSerial,
        1U
    };

    constexpr std::size_t kConfigTotalLen =
        TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_HID_INOUT_DESC_LEN;

    const std::uint8_t kConfigurationDescriptor[kConfigTotalLen] = {
        TUD_CONFIG_DESCRIPTOR(1, kItfTotal, kStrLang, kConfigTotalLen, 0x00, 250),

        // Interface 0: 6-DOF 3Dconnexion SpaceMouse HID (MI_00, EP 0x81 IN & 0x01 OUT required by 3DxWare / 3dxsrv)
        TUD_HID_INOUT_DESCRIPTOR(kItfHid, kStrHid, HID_ITF_PROTOCOL_NONE,
                                 sizeof(kHidReportDescriptor), kEpHidOut, kEpHidIn, 32, 8),

        // Interfaces 1 & 2: USB-CDC Serial for calibration & 1200-baud buttonless BOOTSEL
        TUD_CDC_DESCRIPTOR(kItfCdc, kStrCdc, kEpCdcCmd, 8, kEpCdcOut, kEpCdcIn, 64)
    };

    char g_serialStr[PICO_UNIQUE_BOARD_ID_SIZE_BYTES * 2 + 1]{};

    const char* const kStringDescriptors[] = {
        "",
        "3Dconnexion",
        "SpaceMouse Pro Wireless (cabled)",
        g_serialStr,
        "pico-3d-mouse Telemetry & Calibration CDC",
        "SpaceMouse Pro Wireless (cabled)"
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
        if (report_id == 1U && reqlen >= 12U)
        {
            return 12U;
        }
        if (report_id == 3U && reqlen >= 4U)
        {
            return 4U;
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
