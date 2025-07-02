cmake_minimum_required(VERSION 3.25)
project(pd4web)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(
    WARNING
        "This CMake configuration is intended solely for generating best LSP support and nothing more."
)

add_executable(pd4web Sources/pd4web.cpp Sources/externals.cpp)

if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/pure-data)
    find_package(Git REQUIRED)
    set(LIBPD_TAG "0.55-0")
    execute_process(
        COMMAND ${GIT_EXECUTABLE} clone https://github.com/pure-data/pure-data.git --branch
                ${LIBPD_TAG} --recursive
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RESULT_VARIABLE GIT_RESULT)
    if(NOT GIT_RESULT EQUAL "0")
        message(FATAL_ERROR "Failed to clone puredata repository.")
    endif()
endif()

set(PD_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/pure-data/src)
include(Sources/Libraries/libpd.cmake)

target_include_directories(pd4web PRIVATE ${PD_SOURCE_DIR})

target_link_libraries(pd4web PRIVATE embind libpd)
target_compile_options(pd4web PRIVATE -sUSE_PTHREADS=1 -sWASM_WORKERS=1)
target_link_options(
    pd4web
    PRIVATE
    -sPTHREAD_POOL_SIZE=navigator.hardwareConcurrency
    -sMODULARIZE=1
    -sEXPORT_NAME='Pd4WebModule'
    -sAUDIO_WORKLET=1
    -sUSE_PTHREADS=1
    -sWASM_WORKERS=1
    -sWASM=1)
