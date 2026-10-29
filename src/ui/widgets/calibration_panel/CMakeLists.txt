p3d_add_module(p3d_ui_widget_calibration_panel
    ALIAS p3d::ui::widget_calibration_panel
    HEADERS
        include/ui/widgets/CalibrationPanelWidget.hpp
    SOURCES
        CalibrationPanelWidget.cpp
    PUBLIC_DEPS
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

p3d_add_tests()
