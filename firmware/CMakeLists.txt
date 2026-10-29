cmake_minimum_required(VERSION 3.13)

# ── Pico SDK bootstrap ───────────────────────────────────────────
# PICO_SDK_PATH is exported by the Nix devShell. Including the SDK's
# pico_sdk_init.cmake must happen before project() because it injects
# the cross-compile toolchain file for arm-none-eabi.
include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)

project(pico_3d_mouse C CXX ASM)

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

# ── Driver libraries ─────────────────────────────────────────────
add_subdirectory(lib/drivers_pots)

# ── Applications ─────────────────────────────────────────────────
add_subdirectory(apps/adc_bringup)
