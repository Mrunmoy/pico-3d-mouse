p3d_add_module(p3d_ui_widget_firmware_updater
    ALIAS p3d::ui::widget_firmware_updater
    HEADERS
        include/ui/widgets/FirmwareUpdaterWidget.hpp
    SOURCES
        FirmwareUpdaterWidget.cpp
    PUBLIC_DEPS
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
)

p3d_add_tests()
