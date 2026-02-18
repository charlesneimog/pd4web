cmake_minimum_required(VERSION 3.28)
project(pd4web_thorvg_test LANGUAGES C CXX)

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
    message(FATAL_ERROR "This requires emcc compiler")
endif()

# ──────────────────────────────────────
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -flto -pthread -matomics -mbulk-memory -msimd128 --use-port=emdawnwebgpu")

# ──────────────────────────────────────
set(CPM_FILE ${CMAKE_BINARY_DIR}/CPM.cmake)
set(CPM_VERSION "0.42.0")
if(NOT EXISTS "${CPM_FILE}")
    file(DOWNLOAD "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_VERSION}/CPM.cmake" ${CPM_FILE})
endif()
include(${CPM_FILE})

# ╭──────────────────────────────────────╮
# │               nanovgXC               │
# ╰──────────────────────────────────────╯
cpmaddpackage("gh:charlesneimog/nanovgXC#master")

# ╭──────────────────────────────────────╮
# │                Thorvg                │
# ╰──────────────────────────────────────╯
cpmaddpackage("gh:thorvg/thorvg@1.0.1")
file(GLOB THORVG_COMMON "${thorvg_SOURCE_DIR}/src/common/*.cpp")
file(GLOB THORVG_RENDERER "${thorvg_SOURCE_DIR}/src/renderer/*.cpp")
file(GLOB THORVG_GL "${thorvg_SOURCE_DIR}/src/renderer/gl_engine/*.cpp")
file(GLOB THORVG_WEBGPU "${thorvg_SOURCE_DIR}/src/renderer/wg_engine/*.cpp")
file(GLOB THORVG_SVG_LOADER "${thorvg_SOURCE_DIR}/src/loaders/svg/*.cpp")
file(GLOB THORVG_RAW_LOADER "${thorvg_SOURCE_DIR}/src/loaders/raw/*.cpp")
file(GLOB THORVG_TTF_LOADER "${thorvg_SOURCE_DIR}/src/loaders/ttf/*.cpp")

add_library(
    thorvg STATIC
    # global
    ${THORVG_RENDERER}
    ${THORVG_COMMON}
    ${THORVG_SVG_LOADER}
    # Testes
    ${THORVG_GL}
    ${THORVG_WEBGPU}
    # oi
    ${THORVG_RAW_LOADER}
    ${THORVG_TTF_LOADER})

target_include_directories(thorvg PUBLIC "${thorvg_SOURCE_DIR}/inc")
target_include_directories(thorvg PUBLIC "${thorvg_SOURCE_DIR}/src/renderer")
target_include_directories(thorvg PUBLIC "${thorvg_SOURCE_DIR}/src/renderer/gl_engine")
target_include_directories(thorvg PUBLIC "${thorvg_SOURCE_DIR}/src/renderer/wg_engine")
target_include_directories(thorvg PUBLIC "${thorvg_SOURCE_DIR}/src/common")
target_include_directories(thorvg PUBLIC "${thorvg_SOURCE_DIR}/src/loaders/svg")
target_include_directories(thorvg PUBLIC "${thorvg_SOURCE_DIR}/src/loaders/raw")
target_include_directories(thorvg PUBLIC "${thorvg_SOURCE_DIR}/src/loaders/ttf")
target_include_directories(thorvg PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

target_compile_definitions(thorvg PRIVATE THORVG_GL_RASTER_SUPPORT=1 THORVG_THREAD_SUPPORT=1 THORVG_WG_RASTER_SUPPORT=1 THORVG_GL_TARGET_GLES=1 THORVG_FILE_IO_SUPPORT=1
                                          THORVG_TTF_LOADER_SUPPORT=1)

add_executable(pd4web_thorvg_test main.cpp)
target_link_options(
    pd4web_thorvg_test
    PRIVATE
    # Web GPU
    -sUSE_GLFW=3
    # WebGl
    -sUSE_WEBGL2=1
    -sMAX_WEBGL_VERSION=2
    -sMIN_WEBGL_VERSION=2
    # Others
    -sINITIAL_MEMORY=512mb
    -sASSERTIONS
    -sASYNCIFY=1
    --preload-file
    "${CMAKE_CURRENT_SOURCE_DIR}/dejavu-sans-webfont.ttf@dejavu-sans-webfont.ttf")

target_link_libraries(pd4web_thorvg_test PUBLIC nanovg thorvg)
