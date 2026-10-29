set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets SerialPort)

add_subdirectory(interfaces)
add_subdirectory(services)
add_subdirectory(models)
add_subdirectory(widgets)
add_subdirectory(controllers)
add_subdirectory(app)
