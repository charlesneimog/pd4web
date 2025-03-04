cmake_minimum_required(VERSION 3.25)
project(pdlua)
set(LIB_DIR ${PD4WEB_EXTERNAL_DIR}/${PROJECT_NAME})

# ╭──────────────────────────────────────╮
# │               OBJECTS                │
# ╰──────────────────────────────────────╯

add_library(lua STATIC ${LIB_DIR}/lua/onelua.c)
if(EMSCRIPTEN)
  target_compile_definitions(lua PRIVATE LUA_USE_LINUX)
endif()

pd_add_external(pdlua ${LIB_DIR}/pdlua.c)
target_include_directories(pdlua PRIVATE ${LIB_DIR}/lua)
target_link_libraries(pdlua PRIVATE lua)
