cmake_minimum_required(VERSION 3.25)
project(pd4web LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_SHARED_LIBS OFF)

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
elseif(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")

else()
  message(FATAL_ERROR "Unknown system")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
cmake_policy(SET CMP0077 NEW)

option(PD4WEB_BUILD_PD_EXTERNAL "Build the Pure Data external" ON)
option(PD4WEB_BUILD_PYBIND11 "Build the Python module via pybind11" OFF)

if(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
  message(STATUS "Compilando com Emscripten")
  add_subdirectory(Sources/Pd4Web)
else()
  add_subdirectory(Sources/Compiler ${CMAKE_BINARY_DIR}/compiler)
  if(PD4WEB_BUILD_PD_EXTERNAL)
    add_subdirectory(Sources/Pd ${CMAKE_BINARY_DIR}/pd)

    find_program(EMCC_EXECUTABLE NAMES emcc)
    find_program(EMCPP_EXECUTABLE NAMES em++)
    if(EMCC_EXECUTABLE AND EMCPP_EXECUTABLE)
      message(WARNING "Pd4Web compiling with emcc and em++")
      include(ExternalProject)
      ExternalProject_Add(
        Pd4Web
        SOURCE_DIR ${CMAKE_SOURCE_DIR}/Sources/Pd4Web
        CMAKE_ARGS -DCMAKE_C_COMPILER=${EMCC_EXECUTABLE}
                   -DCMAKE_CXX_COMPILER=${EMCPP_EXECUTABLE}
                   -DCMAKE_INSTALL_PREFIX=${CMAKE_BINARY_DIR}/Sources/Pd4Web
        BINARY_DIR ${CMAKE_BINARY_DIR}/Sources/Pd4Web)
    else()
      message(WARNING "Pd4Web not built, emcc or em++ not found")
    endif()
  else()
    message(STATUS "Skipping Pd external build; PD4WEB_BUILD_PD_EXTERNAL=OFF")
  endif()

endif()
