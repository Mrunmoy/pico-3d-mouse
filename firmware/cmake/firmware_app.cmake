# ── Strict warning flags for our own sources ─────────────────────
#
# pico-sdk injects its C sources (tinyusb, hardware_flash, pico_float, ...)
# directly into the consuming executable target via INTERFACE_SOURCES on
# its libraries. Applying warning flags at source-file granularity scopes
# them strictly to our code.

set(FIRMWARE_STRICT_FLAGS
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wdouble-promotion
    -Wformat=2
    -Wundef
    -Wconversion
    -Wsign-conversion
    $<$<COMPILE_LANGUAGE:CXX>:-Wnon-virtual-dtor>
    $<$<COMPILE_LANGUAGE:CXX>:-Woverloaded-virtual>
)

function(firmware_strict_sources)
    set_source_files_properties(${ARGN} PROPERTIES
        COMPILE_OPTIONS "${FIRMWARE_STRICT_FLAGS}"
    )
endfunction()
