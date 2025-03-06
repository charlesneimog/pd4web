cmake_minimum_required(VERSION 3.25)
project(o.scofo~)
set(LIB_DIR ${PD4WEB_EXTERNAL_DIR}/${PROJECT_NAME})

set(BUILD_PD_OBJECT ON)
set(BUILD_WITH_LUA ON)
add_subdirectory(${LIB_DIR})
