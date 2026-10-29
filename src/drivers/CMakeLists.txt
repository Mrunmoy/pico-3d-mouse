p3d_add_module(p3d_drivers_interfaces
    ALIAS p3d::drivers::interfaces
    INTERFACE
    PUBLIC_DEPS p3d::hal
)

p3d_tests_enabled(p3d_drivers_tests)
if(p3d_drivers_tests)
    add_subdirectory(test_support)
endif()

add_subdirectory(adc_channel)
add_subdirectory(pot_array)

p3d_add_tests()
