cmake_minimum_required(VERSION 3.25)
project(pd4web)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++")
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
cmake_policy(SET CMP0077 NEW)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Emscripten")
    message(STATUS "Compilando com Emscripten")
    add_subdirectory(Sources/Pd4Web)
elseif(${PD4WEB_BUILD_PYBIND11})
    add_subdirectory(Sources/Compiler ${CMAKE_BINARY_DIR}/compiler)
else()
    add_subdirectory(Sources/Compiler ${CMAKE_BINARY_DIR}/compiler)
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

endif()
