p3d_add_test_executable(test_manufacturing_controller
    SOURCES test_manufacturing_controller.cpp
    DEPS
        p3d::ui::controllers
        p3d::ui::mocks
)
