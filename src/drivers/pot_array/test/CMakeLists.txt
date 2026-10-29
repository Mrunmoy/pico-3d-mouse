p3d_add_test_executable(test_potentiometer_array
    SOURCES test_potentiometer_array.cpp
    DEPS p3d::drivers::pot_array p3d::drivers_test_support
)
