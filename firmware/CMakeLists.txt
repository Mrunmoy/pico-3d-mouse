cmake_minimum_required(VERSION 3.16)

# ── Pico SDK bootstrap ───────────────────────────────────────────
include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)

project(pico_3d_mouse_firmware C CXX ASM)

set(CMAKE_C_STANDARD   11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT DEFINED PICO_BOARD)
    set(PICO_BOARD pico CACHE STRING "Pico SDK board header")
endif()

pico_sdk_init()

# ── Global C++ ABI flags ─────────────────────────────────────────
add_compile_options(
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics>
)

# ── Helpers ──────────────────────────────────────────────────────
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/firmware_app.cmake)

# ── Modular Source Tree (HAL + Platform + Drivers + Domain) ──────
set(P3D_BACKEND "rp2040" CACHE STRING "Backend for platform layer" FORCE)
set(P3D_BUILD_TESTS OFF CACHE BOOL "Disable host unit tests on target build" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../src ${CMAKE_CURRENT_BINARY_DIR}/src)

# ── Applications ─────────────────────────────────────────────────
add_subdirectory(apps/pico_3d_mouse)
