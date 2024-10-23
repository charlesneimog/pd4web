cmake_minimum_required(VERSION 3.25)

set(PDCMAKE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/pd.cmake
    CACHE PATH "Path to pd.cmake")

include(${PDCMAKE_DIR}/pd.cmake)
set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/Externals/ambi~)

function(ReplaceLine file line new_line)
    file(READ ${file} FILE_CONTENTS)
    string(REPLACE "${line}" "${new_line}" FILE_CONTENTS "${FILE_CONTENTS}")
    file(WRITE ${file} "${FILE_CONTENTS}")
endfunction()

replaceline(
    "${LIB_DIR}/Libraries/libmysofa/src/hrtf/portable_endian.h"
    "#if defined(__linux__) || defined(__CYGWIN__)"
    "#if defined(__linux__) || defined(__CYGWIN__) || defined(__EMSCRIPTEN__)")

set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -sUSE_ZLIB=1")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -sUSE_ZLIB=1")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -sUSE_ZLIB=1")
set(MATH "")

set(BUILD_TESTS
    OFF
    CACHE BOOL "Build test programs" FORCE)

add_subdirectory(${LIB_DIR})
