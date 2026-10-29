p3d_add_module(p3d_drivers_test_support
    ALIAS p3d::drivers_test_support
    INTERFACE
    PUBLIC_DEPS p3d::drivers::interfaces GTest::gmock
)
