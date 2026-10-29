p3d_add_module(p3d_ui_services
    ALIAS p3d::ui::services
    HEADERS
        include/ui/services/PicotoolFirmwareUpdater.hpp
        include/ui/services/QtProcessRunner.hpp
        include/ui/services/QtSerialTransport.hpp
    SOURCES
        PicotoolFirmwareUpdater.cpp
        QtProcessRunner.cpp
        QtSerialTransport.cpp
    PUBLIC_DEPS
        p3d::ui::interfaces
        Qt6::Core
        Qt6::SerialPort
)

p3d_add_tests()
