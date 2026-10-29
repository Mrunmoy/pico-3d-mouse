p3d_add_module(p3d_ui_main_window
    ALIAS p3d::ui::main_window
    HEADERS
        include/ui/app/MainWindow.hpp
    SOURCES
        MainWindow.cpp
    PUBLIC_DEPS
        p3d::ui::interfaces
        p3d::ui::models
        p3d::ui::controllers
        p3d::ui::widget_gimbal_radar
        p3d::ui::widget_axis_gauge
        p3d::ui::widget_calibration_panel
        p3d::ui::widget_firmware_updater
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

add_executable(pico_3d_mouse_ui
    main.cpp
)
p3d_strict_sources(main.cpp)
target_link_libraries(pico_3d_mouse_ui PRIVATE
    p3d::ui::main_window
    p3d::ui::services
)

p3d_add_tests()
