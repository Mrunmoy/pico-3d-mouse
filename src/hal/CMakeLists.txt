p3d_add_module(p3d_hal
    ALIAS p3d::hal
    INTERFACE
)

p3d_tests_enabled(p3d_hal_tests)
if(p3d_hal_tests)
    add_subdirectory(test_support)
endif()

p3d_add_tests()
