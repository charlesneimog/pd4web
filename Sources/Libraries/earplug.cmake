cmake_minimum_required(VERSION 3.25)

set(PDCMAKE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/pd.cmake
    CACHE PATH "Path to pd.cmake")

message(STATUS "PDCMAKE_DIR: ${PDCMAKE_DIR}")
include(${PDCMAKE_DIR}/pd.cmake)

set(LIB_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/Externals/else
    CACHE STRING "PATH where is ROOT of else folder")

include_directories(${LIB_DIR}/Code_source/shared/aubio/src)
include_directories(${LIB_DIR}/Code_source/shared)

project(earplug)

set(ENABLE_TILDE_TARGET_WARNING off)

pd_add_external(knob "${LIB_DIR}/Code_source/Compiled/control/knob.c")
