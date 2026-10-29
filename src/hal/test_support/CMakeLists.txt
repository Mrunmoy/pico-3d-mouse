p3d_add_module(p3d_hal_test_support
    ALIAS p3d::hal_test_support
    INTERFACE
    PUBLIC_DEPS p3d::hal GTest::gmock
)
