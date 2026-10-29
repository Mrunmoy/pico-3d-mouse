p3d_add_module(p3d_ui_widget_gimbal_radar
    ALIAS p3d::ui::widget_gimbal_radar
    HEADERS
        include/ui/widgets/GimbalRadarWidget.hpp
    SOURCES
        GimbalRadarWidget.cpp
    PUBLIC_DEPS
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

p3d_add_tests()
