cmake_minimum_required(VERSION 3.28)
project(pd4web LANGUAGES C CXX)

if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    message(FATAL_ERROR "Use the pd4web compiler to compile and develop the Pd4Web core")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

option(PD_PD4WEB_BUILD "Build the Pure Data external" ON)
option(PY_PD4WEB_BUILD "Build the Python module" OFF)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_program(CLANG_C NAMES clang)
    find_program(CLANG_CXX NAMES clang++)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++")
elseif(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY
        "MultiThreaded"
        CACHE STRING "" FORCE)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(CMAKE_OSX_DEPLOYMENT_TARGET "14.0")
    find_program(GIT_EXECUTABLE git)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")

else()
    message(FATAL_ERROR "Unknown system")
endif()

include(${CMAKE_CURRENT_SOURCE_DIR}/Resources/cmake/getcmake.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/Resources/cmake/getninja.cmake)

add_subdirectory(Sources/Compiler ${CMAKE_BINARY_DIR}/compiler)
if(PD_PD4WEB_BUILD)
    set(PDLIBDIR "${CMAKE_BINARY_DIR}/pd4web")
    add_subdirectory(Sources/Pd ${CMAKE_BINARY_DIR}/pd)
else()
    message(STATUS "Skipping Pd external build")
endif()
