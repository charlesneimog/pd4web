cmake_minimum_required(VERSION 3.25)
project("@PROJECT_NAME@")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(FetchContent)

# ╭──────────────────────────────────────╮
# │               pd.cmake               │
# ╰──────────────────────────────────────╯
set(PDCMAKE_FILE
    "${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pd.cmake"
    CACHE INTERNAL "pd cmake file path")
include("${PDCMAKE_FILE}")

# ╭──────────────────────────────────────╮
# │                Nanovg                │
# ╰──────────────────────────────────────╯
FetchContent_Declare(nanovg SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/nanovg")
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/nanovg" "${CMAKE_CURRENT_BINARY_DIR}/nanovg-build")

# ╭──────────────────────────────────────╮
# │              Pd sources              │
# ╰──────────────────────────────────────╯
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -matomics -mbulk-memory")
set(PD4WEB_EXTERNAL_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/Externals/")

@PD_SOURCE_DIR@
include("${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/libpd.cmake")
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src")

add_compile_definitions(PDTHREADS PDINSTANCE)

@PD_CMAKE_EXTRADEFINITIONS@

# ╭──────────────────────────────────────╮
# │       Debug or Release options       │
# ╰──────────────────────────────────────╯
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(WARNING "Building in Debug mode")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -flto -pthread -matomics -mbulk-memory -msimd128")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -flto -pthread -matomics -mbulk-memory -msimd128")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -flto -pthread -matomics -mbulk-memory -msimd128")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -flto -pthread -matomics -mbulk-memory -msimd128")
endif()

# ╭──────────────────────────────────────╮
# │          Pd4Web executable           │
# ╰──────────────────────────────────────╯
add_executable(pd4web "${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pd4web.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/externals.cpp")
target_include_directories(pd4web PUBLIC "${nanovg_SOURCE_DIR}/src")

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/Externals/pdlua")
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/Externals/pdlua")
    include_directories("${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/Externals/pdlua/lua")
endif()

target_include_directories(pd4web PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src"Jk)
set_target_properties(pd4web PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/WebPatch")

target_link_libraries(pd4web PRIVATE embind libpd nanovg pdlua)
target_link_options(
    pd4web
    PRIVATE
    -sMODULARIZE=1
    -sEXPORT_NAME='Pd4WebModule'
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
    -sMIN_WEBGL_VERSION=2
    -sOFFSCREENCANVAS_SUPPORT
    -sOFFSCREEN_FRAMEBUFFER)

# Externals includes
@LIBRARIES_SCRIPT_INCLUDE@

# Project Externals Libraries
@PD4WEB_EXTERNAL_OBJECTS_TARGET@

# Preload Files
@PD4WEB_PRELOADED_PATCH@
