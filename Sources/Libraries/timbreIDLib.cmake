cmake_minimum_required(VERSION 3.25)

set(PDCMAKE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/pd.cmake
    CACHE PATH "Path to pd.cmake")

message(STATUS "PDCMAKE_DIR: ${PDCMAKE_DIR}")
include(${PDCMAKE_DIR}/pd.cmake)

set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/Externals/timbreIDLib)

project(timbreIDLib)
set(ENABLE_TILDE_TARGET_WARNING off)
set(timbreIDLib_Common "${LIB_DIR}/src/tIDLib.c")
include_directories(${LIB_DIR}/include)

# ╭──────────────────────────────────────╮
# │                 FFTW                 │
# ╰──────────────────────────────────────╯

cmake_policy(SET CMP0135 NEW)
option(BUILD_SHARED_LIBS OFF)
option(BUILD_TESTS OFF)
set(FFTW3_FILE ${CMAKE_BINARY_DIR}/fftw-3.3.10.tar.gz)
if(NOT EXISTS ${FFTW3_FILE})
    message(STATUS "Downloading FFTW3")
    file(DOWNLOAD https://www.fftw.org/fftw-3.3.10.tar.gz ${FFTW3_FILE})
endif()

file(ARCHIVE_EXTRACT INPUT ${CMAKE_BINARY_DIR}/fftw-3.3.10.tar.gz DESTINATION ${CMAKE_BINARY_DIR}/)

add_subdirectory("${CMAKE_BINARY_DIR}/fftw-3.3.10" EXCLUDE_FROM_ALL)
set_target_properties(fftw3 PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})
set_target_properties(fftw3 PROPERTIES POSITION_INDEPENDENT_CODE ON)

include_directories("${CMAKE_BINARY_DIR}/fftw-3.3.10/api")

# ╭──────────────────────────────────────╮
# │               OBJECTS                │
# ╰──────────────────────────────────────╯

pd_add_external(attackTime "${LIB_DIR}/src/attackTime.c;${timbreIDLib_Common}" LINK_LIBRARIES fftw3)
