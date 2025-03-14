cmake_minimum_required(VERSION 3.25)
project(pdlua)
set(LIB_DIR ${PD4WEB_EXTERNAL_DIR}/${PROJECT_NAME})

if(NOT TARGET lua)
  add_library(lua "${LIB_DIR}/lua/onelua.c")
  target_compile_definitions(lua PUBLIC "-DMAKE_LIB")
  include_directories("${LIB_DIR}/lua")
endif()

pd_add_external(pdlua "${LIB_DIR}/pdlua.c")
target_link_libraries(pdlua PUBLIC lua)
target_include_directories(pdlua PRIVATE "${PD4WEB_EXTERNAL_DIR}/../")

set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} --preload-file \"${LIB_DIR}/pd.lua@/pd.lua\"")
set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} --preload-file \"${LIB_DIR}/pdlua/tutorial/examples/pdx.lua@/pdx.lua\""
)

set(PIXJS_VERSION "8.6.6")
set(PIXJS_URL
    "https://cdnjs.cloudflare.com/ajax/libs/pixi.js/8.6.6/pixi.min.js")

set(PIXJS_DEST "${CMAKE_BINARY_DIR}/pixi.min.js")

# Baixa o arquivo se ele não existir
if(NOT EXISTS ${PIXJS_DEST})
  message(STATUS "Download Pixi.js...")
  file(DOWNLOAD ${PIXJS_URL} ${PIXJS_DEST} SHOW_PROGRESS)
endif()

set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} --pre-js \"${PIXJS_DEST}\"")
