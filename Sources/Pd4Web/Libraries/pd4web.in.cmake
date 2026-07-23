cmake_minimum_required(VERSION 3.25)
project("@PROJECT_NAME@")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(FetchContent)

# ╭──────────────────────────────────────╮
# │       Always STATIC Libraries        │
# ╰──────────────────────────────────────╯
function(add_library target)
    set(args ${ARGN})

    if(EMSCRIPTEN)
        list(
            FIND
            args
            SHARED
            shared_index)
        if(NOT
           shared_index
           EQUAL
           -1)
            list(REMOVE_ITEM args SHARED)
            list(PREPEND args STATIC)
        endif()
    endif()

    _add_library(${target} ${args})
endfunction()

# ╭──────────────────────────────────────╮
# │               pd.cmake               │
# ╰──────────────────────────────────────╯
set(PDCMAKE_FILE
    "${CMAKE_BINARY_DIR}/pd.cmake"
    CACHE INTERNAL "pd cmake file path")
include("${PDCMAKE_FILE}")

# ThorVG is installed by the pd4web compiler and copied into the generated project.
set(thorvg_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/thorvg")
include("${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/thorvg.cmake")

# ╭──────────────────────────────────────╮
# │              Pd sources              │
# ╰──────────────────────────────────────╯
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -matomics -mbulk-memory")
set(PD4WEB_EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/Externals/")

@PD_SOURCE_DIR@
include("${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/libpd.cmake")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/")

add_compile_definitions(PDTHREADS PDINSTANCE)

@PD_CMAKE_EXTRADEFINITIONS@

# ╭──────────────────────────────────────╮
# │       Debug or Release options       │
# ╰──────────────────────────────────────╯
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(WARNING "Building in Debug mode")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -flto -pthread -matomics -mbulk-memory -msimd128 -m32")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -flto -pthread -matomics -mbulk-memory -msimd128 -m32")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -flto -pthread -matomics -mbulk-memory -msimd128 -m32")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -flto -pthread -matomics -mbulk-memory -msimd128 -m32")
endif()

# ╭──────────────────────────────────────╮
# │          Pd4Web executable           │
# ╰──────────────────────────────────────╯
add_executable(
    pd4web
    "${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pd4web.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/RenderCommand.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/ThorVGRenderer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pd4web_externals.cpp")

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/Externals/pdlua")
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/Externals/pdlua")
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/Externals/pdlua/luas/lua")
endif()

target_include_directories(pd4web PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src")
set_target_properties(pd4web PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/WebPatch")

target_link_libraries(
    pd4web
    PRIVATE embind
            libpd
            thorvg
            pdlua)
target_link_options(
    pd4web
    PRIVATE
    -sMODULARIZE=1
    -sEXPORT_NAME='Pd4WebModule'
    -sEXPORTED_RUNTIME_METHODS=["FS"]
    -sINITIAL_MEMORY=@MEMORY_SIZE@MB
    -sEXPORT_ES6=${PD4WEB_AS_ES6}
    -sUSE_PTHREADS=1
    -sPTHREAD_POOL_SIZE=4
    -sWASMFS=1
    -sWASM=1
    -sWASM_WORKERS=1
    -sAUDIO_WORKLET=1
    -sUSE_WEBGL2=1
    -sMAX_WEBGL_VERSION=2
    -sMIN_WEBGL_VERSION=2)

# Externals includes
@LIBRARIES_SCRIPT_INCLUDE@

# Project Externals Libraries
@PD4WEB_EXTERNAL_OBJECTS_TARGET@

# Preload Files
@PD4WEB_PRELOADED_PATCH@
