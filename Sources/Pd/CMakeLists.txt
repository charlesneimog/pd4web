cmake_minimum_required(VERSION 3.13)

project(pdobj)

# ──────────────────────────────────────
set(PDCMAKE_FILE ${CMAKE_BINARY_DIR}/pd.cmake)
set(PDCMAKE_VERSION "v0.2.1")
if(NOT EXISTS "${PDCMAKE_FILE}")
    file(DOWNLOAD
         https://raw.githubusercontent.com/pure-data/pd.cmake/refs/tags/${PDCMAKE_VERSION}/pd.cmake
         ${PDCMAKE_FILE})
endif()
include(${PDCMAKE_FILE})

# ╭──────────────────────────────────────╮
# │                Objeto                │
# ╰──────────────────────────────────────╯
pd_add_external(pdobj "${CMAKE_CURRENT_SOURCE_DIR}/pdobj.cpp" EXTERNAL_NAME pd4web)
target_link_libraries(pdobj PRIVATE pd4web_compiler)
target_include_directories(pdobj PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../Compiler/")

