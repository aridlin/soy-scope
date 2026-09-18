cmake_minimum_required(VERSION 3.20)
cmake_policy(SET CMP0091 NEW)

project(soy_scope LANGUAGES CXX)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    enable_testing()
    add_subdirectory(linux)
    return()
elseif(NOT WIN32)
    message(FATAL_ERROR "Soy Scope supports Windows and Linux")
endif()

add_executable(soy_scope
    src/main.cpp
    resources/soy_scope.rc
)

target_compile_features(soy_scope PRIVATE cxx_std_17)
set_property(TARGET soy_scope PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
target_include_directories(soy_scope PRIVATE vendor)
target_compile_definitions(soy_scope PRIVATE
    UNICODE
    _UNICODE
    NOMINMAX
    WIN32_LEAN_AND_MEAN
    FTUI_KEEP_CONSOLE
)

target_link_libraries(soy_scope PRIVATE
    d2d1
    d3d11
    dcomp
    dxgi
    dwrite
    gdi32
    ole32
    uuid
    user32
    windowscodecs
)