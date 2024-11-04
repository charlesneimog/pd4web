cmake_minimum_required(VERSION 3.25)
project(o.scofo~)
set(LIB_DIR ${PD4WEB_EXTERNAL_DIR}/${PROJECT_NAME})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -matomics -mbulk-memory")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -matomics -mbulk-memory")

set(BUILD_PD_OBJECT ON)
add_subdirectory(${LIB_DIR})