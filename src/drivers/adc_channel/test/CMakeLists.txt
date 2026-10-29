p3d_add_test_executable(test_oversampled_adc_channel
    SOURCES test_oversampled_adc_channel.cpp
    DEPS p3d::drivers::adc_channel p3d::hal_test_support
)
