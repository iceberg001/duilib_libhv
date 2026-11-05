cmake_minimum_required(VERSION 3.15)
project(duilib CXX)

file(GLOB Control_src CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Control/*.cpp")
list(FILTER Control_src EXCLUDE REGEX "Control/UIMenu\\.cpp$")
file(GLOB Core_src CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Core/*.cpp")
file(GLOB Layout_src CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Layout/*.cpp")
file(GLOB Utils_src_cpp CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Utils/*.cpp")
file(GLOB Utils_src_c CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Utils/*.c")
file(GLOB Root_src CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

set(duilib_HEADERS
    UIlib.h
    StdAfx.h
)

add_library(duilib_static STATIC
    ${Control_src}
    ${Core_src}
    ${Layout_src}
    ${Utils_src_cpp}
    ${Utils_src_c}
    ${Root_src}
    ${duilib_HEADERS}
)

target_compile_definitions(duilib_static PRIVATE _WINDOWS UILIB_STATIC UNICODE _UNICODE _HAS_STD_BYTE=0 _CRT_SECURE_NO_WARNINGS)

target_include_directories(duilib_static PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Control>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Core>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Layout>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Utils>
)

target_link_libraries(duilib_static comctl32 gdi32 user32)

set_target_properties(duilib_static PROPERTIES OUTPUT_NAME duilib_static)

# 静态 CRT (/MT)
set_property(TARGET duilib_static PROPERTY
    MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
)
