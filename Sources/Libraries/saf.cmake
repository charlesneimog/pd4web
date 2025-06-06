cmake_minimum_required(VERSION 3.25)
project(saf)

include_directories("${CMAKE_CURRENT_BINARY_DIR}/Pd4Web/Externals/saf/")
set(LIB_DIR ${PD4WEB_EXTERNAL_DIR}/${PROJECT_NAME})
add_subdirectory(${LIB_DIR})
