add_executable(adc_bringup
    main.cpp
)

firmware_strict_sources(main.cpp)

target_include_directories(adc_bringup PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(adc_bringup PRIVATE
    pico_stdlib
    firmware::drivers::pots
)

# USB-CDC stdio on, UART stdio off — stream telemetry over USB.
pico_enable_stdio_usb(adc_bringup 1)
pico_enable_stdio_uart(adc_bringup 0)

# Generates adc_bringup.uf2, adc_bringup.bin, adc_bringup.hex, adc_bringup.map, adc_bringup.dis
pico_add_extra_outputs(adc_bringup)
