p3d_add_module(p3d_drivers_adc_channel
    ALIAS p3d::drivers::adc_channel
    SOURCES OversampledAdcChannel.cpp
    PUBLIC_DEPS p3d::drivers::interfaces p3d::hal
)

p3d_add_tests()
