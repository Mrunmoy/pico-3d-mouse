p3d_add_module(p3d_ui_models
    ALIAS p3d::ui::models
    HEADERS
        include/ui/models/DeviceTelemetryModel.hpp
    SOURCES
        DeviceTelemetryModel.cpp
    PUBLIC_DEPS
        Qt6::Core
)

p3d_add_tests()
