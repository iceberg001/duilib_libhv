cmake_minimum_required(VERSION 3.15)
project(net_api_test CXX)

set(CMAKE_CXX_STANDARD 17)

# ==================== 路径 ====================
set(RES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/res")
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")

# ==================== 查找 Python ====================
find_package(Python3 COMPONENTS Interpreter REQUIRED)

# ==================== 静态运行时 (/MT, /MTd) ====================
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

# ==================== 打包资源为 zip ====================
set(ZIP_FILE "${SRC_DIR}/resources.zip")
file(GLOB RES_FILES CONFIGURE_DEPENDS
    "${RES_DIR}/skin.xml"
    "${RES_DIR}/image/*.png"
    "${RES_DIR}/image/*.jpg"
    "${RES_DIR}/*.ico"
)
add_custom_command(
    OUTPUT ${ZIP_FILE}
    COMMAND ${Python3_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/pack_resources.py" "${RES_DIR}" "${ZIP_FILE}"
    DEPENDS ${RES_FILES}
    COMMENT "Packing resources into zip..."
    VERBATIM
)

# ==================== 子项目：duilib (UI库) + libhv (网络库) ====================
add_subdirectory(3rdparty/duilib)
add_subdirectory(3rdparty/libhv)

# ==================== 源文件 ====================
file(GLOB SRC_FILES CONFIGURE_DEPENDS src/*.cpp)
set(ALL_SOURCES ${SRC_FILES} src/resource.rc ${ZIP_FILE})

add_executable(net_api_test ${ALL_SOURCES})

if(MSVC)
    target_compile_options(net_api_test PRIVATE /wd4819)
endif()

target_include_directories(net_api_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/wup)

# ==================== 编译定义 ====================
target_compile_definitions(net_api_test PRIVATE
    HV_STATICLIB UNICODE _UNICODE UILIB_STATIC
    _CRT_SECURE_NO_WARNINGS
)

# ==================== 链接库 ====================
target_link_libraries(net_api_test
    duilib_static
    libhv
    comctl32 shlwapi imm32 ws2_32 gdiplus ole32 uuid
)

# ==================== 复制 config.ini 到构建目录 ====================
add_custom_command(TARGET net_api_test POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/config.ini"
    "$<TARGET_FILE_DIR:net_api_test>/config.ini"
    COMMENT "Copying config.ini to output directory"
)
