p3d_add_module(p3d_domain_app_controller
    ALIAS p3d::domain::app_controller
    SOURCES AppController.cpp
    PUBLIC_DEPS p3d::domain::axis_processor p3d::drivers::interfaces p3d::hal
)

p3d_add_tests()
