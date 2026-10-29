p3d_add_module(p3d_ui_interfaces
    ALIAS p3d::ui::interfaces
    HEADERS
        include/ui/interfaces/IFirmwareUpdater.hpp
        include/ui/interfaces/IProcessRunner.hpp
        include/ui/interfaces/ISerialTransport.hpp
    SOURCES
        InterfacesAnchor.cpp
    PUBLIC_DEPS
        Qt6::Core
)

p3d_tests_enabled(tests_enabled)
if(tests_enabled)
    add_library(p3d_ui_mocks STATIC
        test_support/include/ui/mocks/MockFirmwareUpdater.hpp
        test_support/include/ui/mocks/MockProcessRunner.hpp
        test_support/include/ui/mocks/MockSerialTransport.hpp
        MocksAnchor.cpp
    )
    target_include_directories(p3d_ui_mocks PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/test_support/include
    )
    target_link_libraries(p3d_ui_mocks PUBLIC
        p3d::ui::interfaces
        GTest::gmock
    )
    p3d_strict_sources(MocksAnchor.cpp)
    add_library(p3d::ui::mocks ALIAS p3d_ui_mocks)
endif()

p3d_add_tests()
