add_executable(adc_bringup
    main.cpp
)

firmware_strict_sources(main.cpp)

target_link_libraries(adc_bringup PRIVATE
    p3d::domain::app_controller
    p3d::drivers::adc_channel
    p3d::drivers::pot_array
    p3d::platform::rp2040
)

# USB-CDC stdio on, UART stdio off — stream telemetry over USB-C.
pico_enable_stdio_usb(adc_bringup 1)
pico_enable_stdio_uart(adc_bringup 0)

# Generates adc_bringup.uf2, adc_bringup.bin, adc_bringup.hex, adc_bringup.map, adc_bringup.dis
pico_add_extra_outputs(adc_bringup)
