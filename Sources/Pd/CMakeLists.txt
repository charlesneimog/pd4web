cmake_minimum_required(VERSION 3.13)

project(pd4web)

# ──────────────────────────────────────
set(PDCMAKE_FILE ${CMAKE_BINARY_DIR}/pd.cmake)
set(PDCMAKE_VERSION "v0.2.2")
if(NOT EXISTS "${PDCMAKE_FILE}")
  file(
    DOWNLOAD
    https://raw.githubusercontent.com/pure-data/pd.cmake/refs/tags/${PDCMAKE_VERSION}/pd.cmake
    ${PDCMAKE_FILE})
endif()
include(${PDCMAKE_FILE})

# ╭──────────────────────────────────────╮
# │                Objeto                │
# ╰──────────────────────────────────────╯
pd_add_external(pd4web.compiler "${CMAKE_CURRENT_SOURCE_DIR}/pdobj.cpp")
target_link_libraries(pd4web.compiler PRIVATE pd4web_compiler)
target_include_directories(pd4web.compiler
                           PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../Compiler/")
set_target_properties(pd4web.compiler PROPERTIES CMAKE_POSITION_INDEPENDENT_CODE ON)

pd_add_datafile(pd4web.compiler "${CMAKE_CURRENT_SOURCE_DIR}/pdobj.cpp")
pd_add_datafile(pd4web.compiler "${CMAKE_CURRENT_SOURCE_DIR}/pd4web.pd")
pd_add_datafile(pd4web.compiler "${CMAKE_CURRENT_SOURCE_DIR}/README.deken.pd")
pd_add_datafile(pd4web.compiler "${CMAKE_CURRENT_SOURCE_DIR}/1-hello-world")
pd_add_datafile(pd4web.compiler "${CMAKE_CURRENT_SOURCE_DIR}/2-ml5")
pd_add_datafile(pd4web.compiler "${CMAKE_CURRENT_SOURCE_DIR}/../Pd4Web/" DESTINATION "Pd4Web")