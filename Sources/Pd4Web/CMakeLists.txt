cmake_minimum_required(VERSION 3.25)
project(pd4web)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ╭──────────────────────────────────────╮
# │               PureData               │
# ╰──────────────────────────────────────╯
if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/pure-data)
    find_package(Git REQUIRED)
    set(LIBPD_TAG "0.56-0test1")
    execute_process(
        COMMAND ${GIT_EXECUTABLE} clone https://github.com/pure-data/pure-data.git --branch
                ${LIBPD_TAG} --recursive
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RESULT_VARIABLE GIT_RESULT)
    if(NOT GIT_RESULT EQUAL "0")
        message(FATAL_ERROR "Failed to clone puredata repository.")
    endif()
endif()
set(PD_SOURCES_PATH ${CMAKE_CURRENT_BINARY_DIR}/pure-data/src)

# ╭──────────────────────────────────────╮
# │                Pdlua                 │
# ╰──────────────────────────────────────╯
if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/pd-lua)
    find_package(Git REQUIRED)
    set(PDLUA_TAG "0.12.23")
    execute_process(
        COMMAND ${GIT_EXECUTABLE} clone https://github.com/agraef/pd-lua --recursive --branch
                ${PDLUA_TAG} --recursive
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RESULT_VARIABLE GIT_RESULT)
    if(NOT GIT_RESULT EQUAL "0")
        message(FATAL_ERROR "Failed to clone pdlua repository.")
    endif()
endif()

# for now, it is necessary to replace glist_getrtext
file(DOWNLOAD
     https://raw.githubusercontent.com/ben-wes/pd-lua/refs/heads/fix/glist_getrtext/pdlua.c
     ${CMAKE_CURRENT_BINARY_DIR}/pd-lua/pdlua.c)

add_library(lua "${CMAKE_CURRENT_BINARY_DIR}/pd-lua/lua/onelua.c")
target_compile_definitions(lua PUBLIC "-DMAKE_LIB")

add_custom_target(
    update_gfw
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/pd4weblua_gfx.c"
            "${CMAKE_CURRENT_BINARY_DIR}/pd-lua/pdlua_gfx.h"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/pdlua.h"
            "${CMAKE_CURRENT_BINARY_DIR}/pd-lua/pdlua.h"
    COMMENT "Updating pdlua_gfx.h and pdlua.h")

add_dependencies(lua update_gfw)
target_compile_definitions(lua PUBLIC PD4WEB_LUA)

set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} --preload-file=${CMAKE_CURRENT_SOURCE_DIR}/DejaVuSans.ttf@/DejaVuSans.ttf"
)
set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} --preload-file=${CMAKE_CURRENT_BINARY_DIR}/pd-lua/pd.lua@/pd.lua")
set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} --preload-file=${CMAKE_CURRENT_BINARY_DIR}/pd-lua/pdlua/tutorial/examples/pdx.lua@/pdx.lua"
)
set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} --preload-file=${CMAKE_CURRENT_BINARY_DIR}/pd-lua/pdlua/tutorial/examples/pdx.lua@/pdx.lua"
)
set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} --preload-file=${CMAKE_CURRENT_SOURCE_DIR}/Gui/@/Gui/")

# ╭──────────────────────────────────────╮
# │                nanovg                │
# ╰──────────────────────────────────────╯
set(NANOVG_DIR "${CMAKE_CURRENT_BINARY_DIR}/nanovg")
set(NANOVG_ZIP "${CMAKE_CURRENT_BINARY_DIR}/nanovg.zip")
if(NOT EXISTS ${NANOVG_DIR})
    message(STATUS "Downloading NanoVG...")
    file(
        DOWNLOAD https://github.com/rgb2hsv/nanovg/archive/refs/heads/development.zip ${NANOVG_ZIP}
        SHOW_PROGRESS
        STATUS DOWNLOAD_STATUS)

    message(STATUS "Unzipping NanoVG...")
    execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${NANOVG_ZIP}
                    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

    file(GLOB NANOVG_EXTRACTED_DIR "${CMAKE_CURRENT_BINARY_DIR}/nanovg-*")
    file(RENAME ${NANOVG_EXTRACTED_DIR} ${NANOVG_DIR})
endif()
add_subdirectory(${NANOVG_DIR} ${CMAKE_CURRENT_BINARY_DIR}/nanovg EXCLUDE_FROM_ALL)

# ╭──────────────────────────────────────╮
# │                Pdlua                 │
# ╰──────────────────────────────────────╯
set(PD_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/pure-data/src)
set(PDCMAKE_FILE ${CMAKE_BINARY_DIR}/pd.cmake)
set(PDCMAKE_VERSION "v0.2.0")
if(NOT EXISTS "${PDCMAKE_FILE}")
    file(DOWNLOAD
         https://raw.githubusercontent.com/pure-data/pd.cmake/refs/tags/${PDCMAKE_VERSION}/pd.cmake
         ${PDCMAKE_FILE})
endif()
include(${PDCMAKE_FILE})

pd_add_external(pdlua "${CMAKE_CURRENT_BINARY_DIR}/pd-lua/pdlua.c")
target_link_libraries(pdlua PUBLIC nanovg)
target_link_libraries(pdlua PUBLIC lua)
target_include_directories(
    pdlua PUBLIC "${PD_SOURCE_DIR}" "${NANOVG_DIR}/src" "${CMAKE_CURRENT_BINARY_DIR}/pd-lua/lua/"
                 "${CMAKE_CURRENT_SOURCE_DIR}")

# ╭──────────────────────────────────────╮
# │              Executable              │
# ╰──────────────────────────────────────╯
include(${CMAKE_CURRENT_SOURCE_DIR}/Libraries/libpd.cmake)
add_executable(pd4web ${CMAKE_CURRENT_SOURCE_DIR}/pd4web.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/externals.cpp)

add_definitions(-DPDTHREADS)
add_definitions(-DPDINSTANCE)

target_link_libraries(pd4web PRIVATE embind libpd pdlua)
target_link_options(
    pd4web
    PUBLIC
    # Core settings
    -sMODULARIZE=1
    -sEXPORT_NAME='Pd4WebModule'
    -sINITIAL_MEMORY=512mb
    # Optimization flags -O3 # Maximum optimization level -flto # Link-time optimization Performance
    # tuning
    -sFORCE_FILESYSTEM=0 # Disable filesystem if not needed
    -sTOTAL_STACK=16mb # Optimize stack size
    # Threading support
    -sUSE_PTHREADS=1
    -sPTHREAD_POOL_SIZE=4
    -sPTHREAD_POOL_SIZE_STRICT=4 # Ensure exactly 4 threads
    # Audio and graphics
    -sAUDIO_WORKLET=1
    -sWASM_WORKERS=1
    -sWASM=1
    -sUSE_WEBGL2=1
    -sMIN_WEBGL_VERSION=2
    # Disable debug features in production
    -sASSERTIONS=0 # Disable assertions
    -sSTACK_OVERFLOW_CHECK=0 # Disable stack overflow checking
)

# Include
include_directories("${NANOVG_DIR}/src")
target_include_directories(pd4web PUBLIC "${PD_SOURCE_DIR}" "${NANOVG_DIR}/src"
                                         "${CMAKE_CURRENT_BINARY_DIR}/pd-lua/lua/")
set_target_properties(
    pd4web PROPERTIES LINK_FLAGS "--preload-file \"${CMAKE_CURRENT_SOURCE_DIR}/main.pd@/index.pd\"")

add_custom_command(
    TARGET pd4web
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/pd4web.threads.js"
            "$<TARGET_FILE_DIR:pd4web>/pd4web.threads.js"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/pd4web.sw.js"
            "$<TARGET_FILE_DIR:pd4web>/pd4web.sw.js"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/index.html"
            "$<TARGET_FILE_DIR:pd4web>/index.html"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/pd4web.style.css"
            "$<TARGET_FILE_DIR:pd4web>/pd4web.style.css"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/icon-192.png"
            "$<TARGET_FILE_DIR:pd4web>/icon-192.png"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/favicon.ico"
            "$<TARGET_FILE_DIR:pd4web>/favicon.ico"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/pd4web.style.css"
            "$<TARGET_FILE_DIR:pd4web>/pd4web.style.css"
    COMMENT "Copying extra files to build output directory")

add_compile_options(-H)
