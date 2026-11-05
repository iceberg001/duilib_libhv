cmake_minimum_required(VERSION 3.15)
project(libhv CXX)

set(CMAKE_CXX_STANDARD 17)

# ==================== 路径 ====================
set(SRC_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(LIB_DIR     "${CMAKE_CURRENT_SOURCE_DIR}/lib")

# ==================== 源文件 ====================
file(GLOB NETMODULE_SRC CONFIGURE_DEPENDS ${SRC_DIR}/networkmodule/*.cpp)
file(GLOB MEMPOOL_SRC   CONFIGURE_DEPENDS ${SRC_DIR}/mempool/*.cpp)
set(SERVERCOMMON_SRC
    ${SRC_DIR}/imodule.cpp
    ${SRC_DIR}/module_manager.cpp
)
set(ALL_SOURCES ${NETMODULE_SRC} ${MEMPOOL_SRC} ${SERVERCOMMON_SRC})

# ==================== 静态库目标 ====================
add_library(libhv STATIC ${ALL_SOURCES})

target_include_directories(libhv PUBLIC ${INCLUDE_DIR} ${SRC_DIR})

target_compile_definitions(libhv PUBLIC HV_STATICLIB)

if(MSVC)
    set_target_properties(libhv PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )
endif()

# ==================== 链接 hv_static ====================
if(MSVC)
    target_link_directories(libhv PUBLIC
        $<$<CONFIG:Debug>:${LIB_DIR}/Debug>
        $<$<CONFIG:Release>:${LIB_DIR}/Release>
    )
endif()
target_link_libraries(libhv hv_static)
