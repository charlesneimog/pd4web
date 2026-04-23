cmake_minimum_required(VERSION 3.25)
project(pdlua)
set(LIB_DIR ${PD4WEB_EXTERNAL_DIR}/${PROJECT_NAME})

add_subdirectory(${LIB_DIR} EXCLUDE_FROM_ALL)
add_library(pdlua ALIAS lua)

target_compile_definitions(lua PRIVATE PD_MULTICHANNEL=1)

# builder.cpp (from pd4web) copy this file for the root
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file \"${LIB_DIR}/pd.lua@/pd.lua\"")
set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} --preload-file \"${LIB_DIR}/pdlua/tutorial/examples/pdx.lua@/pdx.lua\"")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file \"${LIB_DIR}/InterRegular.ttf@/InterRegular.ttf\"")
