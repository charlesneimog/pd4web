cmake_minimum_required(VERSION 3.25)
project("@PROJECT_NAME@")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Pd sources
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -matomics -mbulk-memory")
set(PDCMAKE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/Externals/"
    CACHE STRING "" FORCE)
set(PD4WEB_EXTERNAL_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/Externals/"
    CACHE STRING "" FORCE)
set(PD4WEB_EXTRAS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Extras")

@PD_SOURCE_DIR@
include("${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/libpd.cmake")
@PD_CMAKE_CONTENT@

include_directories("${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pure-data/src")

add_definitions(-DPDTHREADS)
add_definitions(-DPD4WEB)
@PD_CMAKE_EXTRADEFINITIONS@

# Debug or Release options
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(WARNING "Building in Debug mode")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -flto -pthread -matomics -mbulk-memory -msimd128")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -flto -pthread -matomics -mbulk-memory -msimd128")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -flto -pthread -matomics -mbulk-memory -msimd128")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -flto -pthread -matomics -mbulk-memory -msimd128")
endif()

# Pd4Web executable
add_executable(pd4web "${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/pd4web.cpp"
                      "${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/externals.cpp")

target_include_directories(pd4web PRIVATE Pd4Web/pure-data/src)
target_link_libraries(pd4web PRIVATE embind libpd)
set_target_properties(pd4web PROPERTIES RUNTIME_OUTPUT_DIRECTORY
                                        "${CMAKE_CURRENT_SOURCE_DIR}/WebPatch")

target_link_options(
    pd4web
    PRIVATE
    -sMODULARIZE=1
    -sEXPORT_NAME='Pd4WebModule'
    -sINITIAL_MEMORY=@MEMORY_SIZE@MB
    -sUSE_PTHREADS=1
    -sPTHREAD_POOL_SIZE=4
    -sWASMFS=1
    -sWASM=1
    -sWASM_WORKERS=1
    -sAUDIO_WORKLET=1
    -sUSE_WEBGL2=1
    -sMAX_WEBGL_VERSION=2
    -sMIN_WEBGL_VERSION=2
    
    # optimizations
    #-flto
    #-g0
    #-O3
    )

# Externals includes
@LIBRARIES_SCRIPT_INCLUDE@

# Project Externals Libraries
@PD4WEB_EXTERNAL_OBJECTS_TARGET@

# Preload Files
@PD4WEB_PRELOADED_PATCH@
