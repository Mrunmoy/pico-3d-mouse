set(rp2040_common_sources
    Rp2040AdcHardware.cpp
    Rp2040BootControl.cpp
    Rp2040FlashStorage.cpp
    Rp2040GpioOutput.cpp
    Rp2040HidTransport.cpp
    Rp2040TimeProvider.cpp
    Rp2040UsbSerialIo.cpp
)

if(P3D_BACKEND STREQUAL "rp2040")
    list(APPEND rp2040_common_sources Rp2040UsbDescriptors.cpp)
    set(rp2040_sdk_deps
        pico_stdlib
        pico_stdio_usb
        pico_bootrom
        pico_unique_id
        pico_usb_reset_interface
        hardware_adc
        hardware_flash
        hardware_gpio
        hardware_sync
        hardware_timer
        tinyusb_device_unmarked
    )
else()
    set(rp2040_sdk_deps "")
endif()

p3d_add_module(p3d_platform_rp2040
    ALIAS p3d::platform::rp2040
    SOURCES ${rp2040_common_sources}
    PUBLIC_DEPS
        p3d::hal
        ${rp2040_sdk_deps}
)

if(P3D_BACKEND STREQUAL "rp2040")
    target_compile_definitions(p3d_platform_rp2040 PUBLIC
        CFG_TUD_HID=1
        PICO_STDIO_USB_USE_DEFAULT_DESCRIPTORS=0
        PICO_STDIO_USB_ENABLE_RESET_VIA_BAUD_RATE=1
        PICO_STDIO_USB_ENABLE_RESET_VIA_VENDOR_INTERFACE=1
    )
else()
    target_include_directories(p3d_platform_rp2040 PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/test/sdk_stubs
    )
endif()

p3d_add_tests()
