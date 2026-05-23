cmake_minimum_required(VERSION 3.28)
project(pd4web_thorvg_test LANGUAGES C CXX)

if(NOT
   CMAKE_SYSTEM_NAME
   STREQUAL
   "Emscripten")
    message(FATAL_ERROR "This requires emcc compiler")
endif()

# ──────────────────────────────────────
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

include("thorvg.cmake")

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

target_link_libraries(pd4web_thorvg_test PUBLIC thorvg)
