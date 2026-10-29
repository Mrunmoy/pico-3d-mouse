p3d_add_test_executable(test_ui_services
    SOURCES test_services.cpp
    DEPS
        p3d::ui::services
        p3d::ui::mocks
)
