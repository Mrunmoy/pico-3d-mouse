p3d_add_test_executable(test_app_controller
    SOURCES test_app_controller.cpp
    DEPS p3d::domain::app_controller p3d::drivers_test_support p3d::hal_test_support
)
