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
