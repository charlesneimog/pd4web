cmake_minimum_required(VERSION 3.25)
project("@PROJECT_NAME@")

# WebAssembly externals are linked into the final module. Disable shared libraries both for projects that honor
# BUILD_SHARED_LIBS and for projects that explicitly pass SHARED to add_library().
set(BUILD_SHARED_LIBS
    OFF
    CACHE BOOL "Build libraries as static archives" FORCE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(thorvg_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/thorvg")
set(PDCMAKE_FILE "${CMAKE_BINARY_DIR}/pd.cmake")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -matomics -mbulk-memory")
set(PD4WEB_EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/Externals/")

include(FetchContent)

# ╭──────────────────────────────────────╮
# │       Override the add_library       │
# ╰──────────────────────────────────────╯
function(add_library target)
    set(args ${ARGN})

    list(
        FIND
        args
        STATIC
        static_index)
    list(
        FIND
        args
        SHARED
        shared_index)

    set(pd4web_archive_name "")

    # Preserve the originally requested library type.
    if(NOT
       static_index
       EQUAL
       -1)
        set(pd4web_archive_name "${target}_pd4web_static")
    elseif(
        NOT
        shared_index
        EQUAL
        -1)
        set(pd4web_archive_name "${target}_pd4web_shared")
    endif()

    # Emscripten cannot use the regular SHARED libraries here, so build them as static archives instead.
    if(EMSCRIPTEN
       AND NOT
           shared_index
           EQUAL
           -1)
        list(REMOVE_ITEM args SHARED)
        list(PREPEND args STATIC)
    endif()

    _add_library(${target} ${args})

    if(NOT TARGET ${target})
        return()
    endif()

    get_target_property(aliased_target ${target} ALIASED_TARGET)
    if(aliased_target)
        return()
    endif()

    get_target_property(imported ${target} IMPORTED)
    if(imported)
        return()
    endif()

    if(EMSCRIPTEN AND pd4web_archive_name)
        set_target_properties(${target} PROPERTIES ARCHIVE_OUTPUT_NAME "${pd4web_archive_name}")
    endif()
endfunction()

# ╭──────────────────────────────────────╮
# │               pd.cmake               │
# ╰──────────────────────────────────────╯
include("${PDCMAKE_FILE}")
include("${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/thorvg.cmake")

# ╭──────────────────────────────────────╮
# │              Pd sources              │
# ╰──────────────────────────────────────╯
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
    -sPTHREAD_POOL_SIZE=4
    -sWASMFS=1
    -sWASM=1
    -sWASM_WORKERS=1
    -sAUDIO_WORKLET=1
    -sUSE_WEBGL2=1
    -sMAX_WEBGL_VERSION=2
    -sMIN_WEBGL_VERSION=2
    -pthread)

# Externals includes
@LIBRARIES_SCRIPT_INCLUDE@

# Project Externals Libraries
@PD4WEB_EXTERNAL_OBJECTS_TARGET@

# Preload Files
@PD4WEB_PRELOADED_PATCH@
