# Module helpers used by every CMakeLists.txt under src/.

include_guard(GLOBAL)

# ── warnings ─────────────────────────────────────────────────

set(P3D_STRICT_WARNING_FLAGS
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

# Scope strict warnings to our own source files so vendor INTERFACE_SOURCES
# injected by pico-sdk (e.g. double_math.c) do not trigger third-party warnings.
function(p3d_strict_sources)
    set_source_files_properties(${ARGN} PROPERTIES
        COMPILE_OPTIONS "${P3D_STRICT_WARNING_FLAGS}"
    )
endfunction()

# ── modules ──────────────────────────────────────────────────

# p3d_add_module(<target>
#     ALIAS <p3d::name>
#     [INTERFACE]                    header-only target
#     [SOURCES <files>...]
#     [PUBLIC_DEPS <targets>...]
#     [PRIVATE_DEPS <targets>...])
function(p3d_add_module name)
    cmake_parse_arguments(arg "INTERFACE" "ALIAS" "SOURCES;PUBLIC_DEPS;PRIVATE_DEPS" ${ARGN})
    if(NOT arg_ALIAS)
        message(FATAL_ERROR "p3d_add_module(${name}): ALIAS is required")
    endif()

    set(include_dir ${CMAKE_CURRENT_SOURCE_DIR}/include)

    if(arg_INTERFACE)
        add_library(${name} INTERFACE)
        if(EXISTS ${include_dir})
            target_include_directories(${name} INTERFACE ${include_dir})
        endif()
        target_link_libraries(${name} INTERFACE ${arg_PUBLIC_DEPS})
    else()
        add_library(${name} STATIC ${arg_SOURCES})
        if(EXISTS ${include_dir})
            target_include_directories(${name} PUBLIC ${include_dir})
        endif()
        target_link_libraries(${name} PUBLIC ${arg_PUBLIC_DEPS} PRIVATE ${arg_PRIVATE_DEPS})
        target_compile_features(${name} PUBLIC cxx_std_17)
        p3d_strict_sources(${arg_SOURCES})
    endif()

    add_library(${arg_ALIAS} ALIAS ${name})
endfunction()

# ── tests ────────────────────────────────────────────────────

function(p3d_tests_enabled out_var)
    set(enabled ${P3D_BUILD_TESTS})
    if(NOT P3D_BACKEND STREQUAL "host")
        set(enabled OFF)
    endif()
    set(${out_var} ${enabled} PARENT_SCOPE)
endfunction()

function(p3d_add_tests)
    set(dir test)
    if(ARGC GREATER 0)
        set(dir ${ARGV0})
    endif()
    p3d_tests_enabled(enabled)
    if(enabled)
        add_subdirectory(${dir})
    endif()
endfunction()

function(p3d_add_test_executable name)
    cmake_parse_arguments(arg "" "" "SOURCES;DEPS" ${ARGN})
    add_executable(${name} ${arg_SOURCES})
    target_link_libraries(${name} PRIVATE ${arg_DEPS} GTest::gtest_main GTest::gmock)
    p3d_strict_sources(${arg_SOURCES})
    gtest_discover_tests(${name} DISCOVERY_TIMEOUT 30)
endfunction()
