p3d_add_module(p3d_ui_widget_axis_gauge
    ALIAS p3d::ui::widget_axis_gauge
    HEADERS
        include/ui/widgets/AxisGaugeWidget.hpp
    SOURCES
        AxisGaugeWidget.cpp
    PUBLIC_DEPS
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

p3d_add_tests()
