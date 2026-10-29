if(P3D_BACKEND STREQUAL "rp2040")
    set(rp2040_sdk_deps pico_stdlib hardware_adc hardware_gpio hardware_timer)
else()
    set(rp2040_sdk_deps "")
endif()

p3d_add_module(p3d_platform_rp2040
    ALIAS p3d::platform::rp2040
    SOURCES
        Rp2040AdcHardware.cpp
        Rp2040GpioOutput.cpp
        Rp2040TimeProvider.cpp
        Rp2040UsbSerialIo.cpp
    PUBLIC_DEPS
        p3d::hal
        ${rp2040_sdk_deps}
)

if(P3D_BACKEND STREQUAL "host")
    target_include_directories(p3d_platform_rp2040 PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/test/sdk_stubs
    )
endif()

p3d_add_tests()
