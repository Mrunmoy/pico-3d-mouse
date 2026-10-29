p3d_add_module(p3d_drivers_pot_array
    ALIAS p3d::drivers::pot_array
    SOURCES PotentiometerArray.cpp
    PUBLIC_DEPS p3d::drivers::interfaces
)

p3d_add_tests()
