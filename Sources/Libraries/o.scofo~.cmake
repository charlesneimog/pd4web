cmake_minimum_required(VERSION 3.25)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -matomics -mbulk-memory")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -matomics -mbulk-memory")

set(PDCMAKE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/pd.cmake
    CACHE PATH "Path to pd.cmake")
include(${PDCMAKE_DIR}/pd.cmake)
set(LIB_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/Externals/o.scofo~
    CACHE STRING "PATH where is ROOT of pd-pmpd folder")

set(BUILD_PD_OBJECT ON)
add_subdirectory(${LIB_DIR})
