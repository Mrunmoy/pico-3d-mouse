p3d_add_test_executable(test_main_window
    SOURCES test_main_window.cpp
    DEPS
        p3d::ui::main_window
        p3d::ui::mocks
)
