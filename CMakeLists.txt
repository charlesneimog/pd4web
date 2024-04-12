cmake_minimum_required(VERSION 3.25)
project(pd4web)

set(CMAKE_CXX_COMPILER emcc)

#╭──────────────────────────────────────╮
#│            Configurations            │
#╰──────────────────────────────────────╯
if (PATCH)
    set(PATCH ${PATCH})
endif()

if (INIT_MEMORY)
    set(INIT_MEMORY ${INIT_MEMORY})
else()
    set(INIT_MEMORY 32)
endif()

if (LIBPD_TAG)
    set(LIBPD_TAG ${LIBPD_TAG})
else()
    set(LIBPD_TAG "master") ## TODO: Need to set next realease
endif()

#╭──────────────────────────────────────╮
#│                libpd                 │
#╰──────────────────────────────────────╯
include(ExternalProject)
find_package(Git REQUIRED)

if(NOT EXISTS "${CMAKE_CURRENT_BINARY_DIR}/libpd/CMakeLists.txt")
    find_package(Git REQUIRED)
    execute_process(
        COMMAND ${GIT_EXECUTABLE} clone https://github.com/libpd/libpd.git --branch ${LIBPD_TAG} --recursive
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RESULT_VARIABLE GIT_RESULT
    )
    if(NOT GIT_RESULT EQUAL "0")
        message(FATAL_ERROR "Failed to clone libpd repository.")
    endif()
endif()

# Add libpd as a subdirectory
cmake_policy(SET CMP0077 NEW)
# set(PD_EXTRA OFF)
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/libpd)

#╭──────────────────────────────────────╮
#│               Objects                │
#╰──────────────────────────────────────╯
file(GLOB OBJECTS_CXX "Sources/Objects/*.cpp")
file(GLOB OBJECTS_C "Sources/Objects/*.c")

if(OBJECTS_CXX AND OBJECTS_C)
    add_library(externals OBJECT ${OBJECTS_CXX} ${OBJECTS_C})
    target_include_directories(externals PRIVATE "${CMAKE_CURRENT_BINARY_DIR}/libpd/pure-data/src/")
endif()

# check if EXTRA_FLAGS is set
if (OBJ_LIB_FLAGS)
    message(STATUS "OBJ_LIB_FLAGS is set: ${OBJ_LIB_FLAGS}")
endif()

if (OBJ_INCLUDES)
    message(STATUS "OBJ_INCLUDES is set: ${OBJ_LIB_FLAGS}")
endif()

#╭──────────────────────────────────────╮
#│                pd4web                │
#╰──────────────────────────────────────╯
file(GLOB PD4WEB_SOURCES "Sources/*.cpp")
add_executable(pd4web ${PD4WEB_SOURCES})
target_include_directories(pd4web PRIVATE "${CMAKE_BINARY_DIR}/libpd/libpd_wrapper/")
target_include_directories(pd4web PRIVATE "${CMAKE_BINARY_DIR}/libpd/pure-data/src/")
target_link_libraries(pd4web PUBLIC embind libpd)

set_target_properties(pd4web PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/webpatch/)
set_target_properties(pd4web PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/webpatch/)
set_target_properties(pd4web PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/webpatch/)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/webpatch)

execute_process(
    COMMAND 
        ${CMAKE_COMMAND} -E copy 
            ${CMAKE_CURRENT_SOURCE_DIR}/Sources/index.html 
            ${CMAKE_CURRENT_SOURCE_DIR}/Sources/index.pd 
            ${CMAKE_CURRENT_SOURCE_DIR}/Sources/enable-threads.js ${CMAKE_CURRENT_SOURCE_DIR}/webpatch
)

#╭──────────────────────────────────────╮
#│           EmScripten Flags           │
#╰──────────────────────────────────────╯

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/Patches@/")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DPD -sUSE_PTHREADS=1 -sAUDIO_WORKLET=1 -sWASM_WORKERS=1")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-command-line-argument -DPD -DPDINSTANCE -sINITIAL_MEMORY=${INIT_MEMORY}MB")

