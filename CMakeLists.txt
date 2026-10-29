cmake_minimum_required(VERSION 3.16)

project(pico_3d_mouse_host
    VERSION 0.1.0
    DESCRIPTION "Host off-target libraries and colocated unit tests for pico-3d-mouse"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(P3D_BUILD_TESTS "Build every module's colocated unit tests" ON)
option(P3D_ENABLE_ASAN "Enable AddressSanitizer and UndefinedBehaviorSanitizer" OFF)

if(P3D_ENABLE_ASAN)
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address,undefined)
endif()

if(P3D_BUILD_TESTS)
    enable_testing()
endif()

set(P3D_BACKEND "host" CACHE STRING "Backend for platform layer" FORCE)
add_subdirectory(src)
