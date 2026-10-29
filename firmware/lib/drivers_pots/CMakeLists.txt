add_library(drivers_pots STATIC
    pots.cpp
)

add_library(firmware::drivers::pots ALIAS drivers_pots)

firmware_strict_sources(pots.cpp)

target_include_directories(drivers_pots PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(drivers_pots PUBLIC
    hardware_adc
    hardware_gpio
)
