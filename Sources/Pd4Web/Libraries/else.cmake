cmake_minimum_required(VERSION 3.25)
project(else)

set(CPM_FILE ${CMAKE_BINARY_DIR}/CPM.cmake)
set(CPM_VERSION "0.42.1")
if(NOT EXISTS "${CPM_FILE}")
    file(DOWNLOAD
         "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_VERSION}/CPM.cmake"
         ${CPM_FILE})
endif()
include(${CPM_FILE})

cpmaddpackage("gh:xiph/ogg#v1.3.6")
set(LIB_DIR ${PD4WEB_EXTERNAL_DIR}/${PROJECT_NAME})
add_subdirectory(${LIB_DIR} EXCLUDE_FROM_ALL)

