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
# │               Servidor               │
# ╰──────────────────────────────────────╯
# https://github.com/yhirose/cpp-httplib/tree/v0.22.0
set(OPENSSL_USE_STATIC_LIBS ON)
include(FetchContent)
FetchContent_Declare(httplib
                     URL https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.22.0.zip)

FetchContent_MakeAvailable(httplib)

# ╭──────────────────────────────────────╮
# │                Objeto                │
# ╰──────────────────────────────────────╯
pd_add_external(pdobj "${CMAKE_CURRENT_SOURCE_DIR}/pdobj.cpp" EXTERNAL_NAME pd4web)
target_include_directories(pdobj PRIVATE "${httplib_SOURCE_DIR}/")
target_link_libraries(pdobj pd4web_compiler httplib)
