cmake_minimum_required(VERSION 3.13)
project(pd4web-compiler)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

cmake_policy(SET CMP0077 NEW)
set(BUILD_SHARED_LIBS OFF)
set(BUILD_TESTS OFF)
set(BUILD_CLI OFF)
set(USE_NSEC OFF)
set(HTTPLIB_USE_BROTLI_IF_AVAILABLE OFF)
set(HTTPLIB_USE_ZSTD_IF_AVAILABLE OFF)
set(TREE_SITTER_CLI "")

# ╭──────────────────────────────────────╮
# │             CPM Package              │
# ╰──────────────────────────────────────╯
set(CPM_FILE ${CMAKE_BINARY_DIR}/CPM.cmake)
set(CPM_VERSION "0.42.0")
if(NOT EXISTS "${CPM_FILE}")
  file(
    DOWNLOAD
    "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_VERSION}/CPM.cmake"
    SHOW_PROGRESS
    ${CPM_FILE})
endif()
include(${CPM_FILE})

# ╭──────────────────────────────────────╮
# │             Dependencies             │
# ╰──────────────────────────────────────╯
cpmaddpackage("gh:yhirose/cpp-httplib#v0.25.0")
cpmaddpackage(
  NAME
  tree-sitter
  GITHUB_REPOSITORY
  tree-sitter/tree-sitter
  VERSION
  0.25.6
  SOURCE_SUBDIR
  lib
  EXCLUDE_FROM_ALL
  YES)
cpmaddpackage("gh:tree-sitter/tree-sitter-c#v0.24.1")

if(NOT EXISTS "${CMAKE_BINARY_DIR}/CMakeCache.txt")
  cpmaddpackage(
    NAME
    tree-sitter-cpp
    VERSION
    0.23.4
    GITHUB_REPOSITORY
    tree-sitter/tree-sitter-cpp
    PATCHES
    ${CMAKE_SOURCE_DIR}/Resources/rename-ts-test.patch
    EXCLUDE_FROM_ALL
    YES)

else()
  cpmaddpackage(
    NAME
    tree-sitter-cpp
    VERSION
    0.23.4
    GITHUB_REPOSITORY
    tree-sitter/tree-sitter-cpp
    EXCLUDE_FROM_ALL
    YES)
endif()

cpmaddpackage("gh:jarro2783/cxxopts#v3.3.1") # parse command line
cpmaddpackage("gh:nlohmann/json#v3.12.0") # read/save json files
cpmaddpackage("gh:fktn-k/fkYAML#v0.4.2") # read/save yaml files

if(MSVC)
  cpmaddpackage(NAME openssl-cmake URL
                https://github.com/jimmy-park/openssl-cmake/archive/main.tar.gz)
endif()

cpmaddpackage("gh:libgit2/libgit2#v1.9.1")

cpmaddpackage(
  NAME
  Boost
  VERSION
  1.86.0
  URL
  https://github.com/boostorg/boost/releases/download/boost-1.86.0/boost-1.86.0-cmake.tar.xz
  URL_HASH
  SHA256=2c5ec5edcdff47ff55e27ed9560b0a0b94b07bd07ed9928b476150e16b0efc57
  EXCLUDE_FROM_ALL
  YES)

# Include

if(tree-sitter_ADDED)
  include_directories("${tree-sitter_SOURCE_DIR}/lib/include")
endif()

# ╭──────────────────────────────────────╮
# │           pd4web_compiler            │
# ╰──────────────────────────────────────╯
add_library(
  pd4web_compiler STATIC
  "${CMAKE_CURRENT_SOURCE_DIR}/git.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/libraries.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/patch.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/paths.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/utils.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/cmd.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/builder.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/pd4web_compiler.cpp")

target_include_directories(pd4web_compiler PUBLIC ${CMAKE_BINARY_DIR})
target_link_libraries(
  pd4web_compiler
  PUBLIC tree-sitter
         tree-sitter-cpp
         tree-sitter-c
         cxxopts
         nlohmann_json
         fkYAML
         httplib
         libgit2
         libgit2package
         Boost::process)

set_target_properties(pd4web_compiler PROPERTIES CMAKE_POSITION_INDEPENDENT_CODE
                                                 ON)

# ╭──────────────────────────────────────╮
# │               pybind11               │
# ╰──────────────────────────────────────╯
if(PD4WEB_BUILD_PYBIND11)
  # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc")
  set(PYBIND11_FINDPYTHON ON)
  find_package(pybind11 REQUIRED)
  add_library(pypd4web MODULE ${CMAKE_CURRENT_SOURCE_DIR}/pybind11.cpp)
  target_link_libraries(pypd4web PRIVATE pybind11::module pd4web_compiler)
  set_target_properties(pypd4web PROPERTIES PREFIX "" OUTPUT_NAME "pypd4web")

  install(TARGETS pypd4web DESTINATION pd4web)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/pd4web/__init__.py
          DESTINATION pd4web)
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../Pd4Web DESTINATION pd4web)
elseif(PD4WEB_BUILD_EXE)
  add_executable(pd4web "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
  target_link_libraries(pd4web PRIVATE pd4web_compiler)
  target_compile_options(pd4web PRIVATE $<$<CONFIG:Debug>:-g>)

  install(TARGETS pd4web RUNTIME DESTINATION bin)
endif()
