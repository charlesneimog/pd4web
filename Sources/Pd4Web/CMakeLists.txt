cmake_minimum_required(VERSION 3.25)
project(pd4web)

option(USE_WEBGPU
       "Enable WebGPU backend (requires Emscripten with WebGPU support)" OFF)
set(LIBPD_TAG "c09ef5b79b47f1e87629a2dec7c74407dc863001")
set(PDLUA_TAG "0.12.23")

include(FetchContent)

# ╭──────────────────────────────────────╮
# │               PureData               │
# ╰──────────────────────────────────────╯
FetchContent_Declare(
  pure_data
  GIT_REPOSITORY https://github.com/pure-data/pure-data.git
  GIT_TAG ${LIBPD_TAG}
  GIT_SHALLOW FALSE
  GIT_PROGRESS TRUE
  SOURCE_SUBDIR src)

FetchContent_MakeAvailable(pure_data)
set(PD_SOURCE_DIR ${pure_data_SOURCE_DIR}/src/)

include(${CMAKE_CURRENT_SOURCE_DIR}/Libraries/libpd.cmake)
target_compile_definitions(libpd PUBLIC PDTHREADS PDINSTANCE)

# ╭──────────────────────────────────────╮
# │                Pdlua                 │
# ╰──────────────────────────────────────╯
FetchContent_Declare(
  pd_lua
  GIT_REPOSITORY https://github.com/agraef/pd-lua.git
  GIT_TAG ${PDLUA_TAG}
  GIT_PROGRESS TRUE
  GIT_SUBMODULES_RECURSE TRUE)

FetchContent_MakeAvailable(pd_lua)
include_directories("${pd_lua_SOURCE_DIR}/")
add_library(lua "${pd_lua_SOURCE_DIR}/lua/onelua.c")
target_compile_definitions(lua PUBLIC "MAKE_LIB")

# for now, it is necessary to replace glist_getrtext
add_custom_target(
  update_gfw
  COMMAND
    ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/pd4weblua_gfx.c"
    "${pd_lua_SOURCE_DIR}/pdlua_gfx.h"
  COMMAND
    ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/pd4weblua.h" "${pd_lua_SOURCE_DIR}/pdlua.h"
  COMMAND
    ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_CURRENT_SOURCE_DIR}/pd4weblua.c" "${pd_lua_SOURCE_DIR}/pdlua.c"
  COMMENT "Checking pdlua.c, pdlua.h and pdlua_gfx.h")

# pd.cmake
set(PDCMAKE_FILE ${CMAKE_BINARY_DIR}/pd.cmake)
set(PDCMAKE_VERSION "v0.2.1")
if(NOT EXISTS "${PDCMAKE_FILE}")
  file(
    DOWNLOAD
    https://raw.githubusercontent.com/pure-data/pd.cmake/refs/tags/${PDCMAKE_VERSION}/pd.cmake
    ${PDCMAKE_FILE})
endif()
include(${PDCMAKE_FILE})

pd_add_external(pdlua "${pd_lua_SOURCE_DIR}/pdlua.c")
target_link_libraries(pdlua PUBLIC lua libpd)
target_include_directories(
  pdlua PUBLIC "${PD_SOURCE_DIR}" "${pd_lua_SOURCE_DIR}/lua/"
               "${CMAKE_CURRENT_SOURCE_DIR}")
add_dependencies(pdlua update_gfw)

# ╭──────────────────────────────────────╮
# │                nanovg                │
# ╰──────────────────────────────────────╯
FetchContent_Declare(
  nanovg
  URL https://github.com/charlesneimog/nanovg/archive/refs/heads/main.zip)

FetchContent_MakeAvailable(nanovg)
include_directories(${nanovg_SOURCE_DIR}/src)

# ╭──────────────────────────────────────╮
# │              Executable              │
# ╰──────────────────────────────────────╯
add_executable(pd4web ${CMAKE_CURRENT_SOURCE_DIR}/pd4web.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/externals.cpp)


target_compile_definitions(pd4web PUBLIC PD4WEB_LUA)
target_compile_options(pd4web PUBLIC -matomics -mbulk-memory)

target_link_libraries(pd4web PUBLIC embind libpd pdlua)
target_link_options(
  pd4web
  PUBLIC
  # Core settings
  -sMODULARIZE=1
  -sEXPORT_NAME='Pd4WebModule'
  -sINITIAL_MEMORY=512mb
  # Optimization flags -O3 # Maximum optimization level -flto Link-time
  # optimization Performance tuning
  -sFORCE_FILESYSTEM=0 # Disable filesystem if not needed
  # Threading support
  -sUSE_PTHREADS=1
  -sPTHREAD_POOL_SIZE=4
  -sPTHREAD_POOL_SIZE_STRICT=4 # Ensure exactly 4 threads
  # Audio and graphics
  -sAUDIO_WORKLET=1
  -sWASM_WORKERS=1
  -sWASM=1
  # others -sSTACK_OVERFLOW_CHECK=0 # Disable stack overflow checking
  # -sDISABLE_EXCEPTION_CATCHING=1 Debug -sASSERTIONS=0 # Disable assertions
  $<$<CONFIG:Debug>:-g
  -sASSERTIONS=0>)

target_link_options(
  pd4web
  PRIVATE
  "--preload-file=${CMAKE_CURRENT_SOURCE_DIR}/InterRegular.ttf@/InterRegular.ttf"
  "--preload-file=${pd_lua_SOURCE_DIR}/pd.lua@/pd.lua"
  "--preload-file=${pd_lua_SOURCE_DIR}/pdlua/tutorial/examples/pdx.lua@/pdx.lua"
  "--preload-file=${CMAKE_CURRENT_SOURCE_DIR}/main.pd@/index.pd"
  "--preload-file=${CMAKE_CURRENT_SOURCE_DIR}/Gui/@/Gui/")

add_custom_command(
  TARGET pd4web
  POST_BUILD
  COMMAND
    ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/pd4web.threads.js"
    "$<TARGET_FILE_DIR:pd4web>/pd4web.threads.js"
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/pd4web.sw.js"
          "$<TARGET_FILE_DIR:pd4web>/pd4web.sw.js"
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/index.html"
          "$<TARGET_FILE_DIR:pd4web>/index.html"
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/icon-192.png"
          "$<TARGET_FILE_DIR:pd4web>/icon-192.png"
  COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/favicon.ico"
          "$<TARGET_FILE_DIR:pd4web>/favicon.ico"
  COMMENT "Copying extra files to build output directory")

add_dependencies(pd4web update_gfw)
target_compile_features(pd4web PUBLIC cxx_std_23)

if(USE_WEBGPU)
  message(WARNING "Using WebGPU")
  target_link_options(pd4web PUBLIC --use-port=emdawnwebgpu)
  target_compile_definitions(pd4web PUBLIC PD4WEB_WEBGPU)
else()
  target_link_options(pd4web PUBLIC -sUSE_WEBGL2=1 -sMIN_WEBGL_VERSION=2
                      -sOFFSCREEN_FRAMEBUFFER -sOFFSCREENCANVAS_SUPPORT)

  target_link_libraries(pdlua PUBLIC nanovg)
endif()

# ╭──────────────────────────────────────╮
# │            Fix Cmake LSP             │
# ╰──────────────────────────────────────╯
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
execute_process(COMMAND ${CMAKE_CXX_COMPILER} --cflags
                OUTPUT_VARIABLE EM_CFLAGS COMMAND_ERROR_IS_FATAL ANY)
string(STRIP "${EM_CFLAGS}" EM_CFLAGS)
find_package(Python3 REQUIRED)
message("Python: ${Python3_EXECUTABLE}")
set(ENV{EXTRA_FLAGS} "${EM_CFLAGS}")
execute_process(
  COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/Resources/fix_lsp.py
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
