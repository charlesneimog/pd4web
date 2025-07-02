cmake_minimum_required(VERSION 3.25)
project(partialtrack~)
set(LIB_DIR ${PD4WEB_EXTERNAL_DIR}/${PROJECT_NAME})

set(BUILD_PD_OBJECT ON)
add_subdirectory(${LIB_DIR})
