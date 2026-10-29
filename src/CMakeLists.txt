# Root of the pico-3d-mouse modular code tree.
# Can be built standalone on host (with P3D_BACKEND=host and colocated unit tests)
# or consumed via add_subdirectory() by RP2040 firmware apps (with P3D_BACKEND=rp2040).

cmake_minimum_required(VERSION 3.16)

if(NOT DEFINED PROJECT_NAME)
    project(pico_3d_mouse_src LANGUAGES CXX)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(P3D_BACKEND "host" CACHE STRING "Backend for platform layer: host or rp2040")
set_property(CACHE P3D_BACKEND PROPERTY STRINGS host rp2040)

option(P3D_BUILD_TESTS "Build every module's colocated unit tests (host backend only)" ON)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/p3d.cmake)

if(P3D_BUILD_TESTS AND P3D_BACKEND STREQUAL "host")
    enable_testing()
    find_package(GTest REQUIRED)
    include(GoogleTest)
endif()

# ── Modules (lowest layer first) ─────────────────────────────
add_subdirectory(hal)
add_subdirectory(platform)
add_subdirectory(drivers)
add_subdirectory(domain)
