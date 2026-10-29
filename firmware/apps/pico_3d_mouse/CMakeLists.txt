add_executable(pico_3d_mouse
    main.cpp
)

firmware_strict_sources(main.cpp)

target_link_libraries(pico_3d_mouse PRIVATE
    p3d::domain::app_controller
    p3d::domain::calibration_store
    p3d::drivers::adc_channel
    p3d::drivers::pot_array
    p3d::platform::rp2040
)

# USB-CDC stdio on, UART stdio off — stream telemetry over USB-C.
pico_enable_stdio_usb(pico_3d_mouse 1)
pico_enable_stdio_uart(pico_3d_mouse 0)

# Generates pico_3d_mouse.uf2, pico_3d_mouse.bin, pico_3d_mouse.hex, pico_3d_mouse.map, pico_3d_mouse.dis
pico_add_extra_outputs(pico_3d_mouse)
