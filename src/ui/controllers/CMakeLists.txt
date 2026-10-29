p3d_add_module(p3d_ui_controllers
    ALIAS p3d::ui::controllers
    HEADERS
        include/ui/controllers/ManufacturingController.hpp
    SOURCES
        ManufacturingController.cpp
    PUBLIC_DEPS
        p3d::ui::interfaces
        p3d::ui::models
        Qt6::Core
)

p3d_add_tests()
