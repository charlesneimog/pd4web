cmake_minimum_required(VERSION 3.25)
project(ambi~) # MUST BE THE SAME NAME AS IS NAME FROM Libraries.yaml

set(LIB_DIR ${PD4WEB_EXTERNAL_DIR}/${PROJECT_NAME})

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
