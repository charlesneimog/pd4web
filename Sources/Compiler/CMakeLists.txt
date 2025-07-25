cmake_minimum_required(VERSION 3.13)
project(pd4web-compiler)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

cmake_policy(SET CMP0077 NEW)
set(BUILD_SHARED_LIBS
    OFF
    CACHE BOOL "Build shared libraries")
set(BUILD_TESTS
    OFF
    CACHE BOOL "Build tests")
set(BUILD_CLI
    OFF
    CACHE BOOL "Build command-line interface")

# ╭──────────────────────────────────────╮
# │              httplib.h               │
# ╰──────────────────────────────────────╯
set(HTTPLIB_FILE ${CMAKE_BINARY_DIR}/httplib.h)
set(HTTPLIB_VERSION "v0.22.0")
if(NOT EXISTS "${HTTPLIB_FILE}")
  file(
    DOWNLOAD
    "https://raw.githubusercontent.com/yhirose/cpp-httplib/refs/tags/${HTTPLIB_VERSION}/httplib.h"
    ${HTTPLIB_FILE})
endif()

# ╭──────────────────────────────────────╮
# │            YAML (fkYAML)             │
# ╰──────────────────────────────────────╯
set(YAML_FILE ${CMAKE_BINARY_DIR}/yaml.hpp)
set(YAML_VERSION "v0.4.2")
if(NOT EXISTS "${YAML_FILE}")
  file(
    DOWNLOAD
    "https://raw.githubusercontent.com/fktn-k/fkYAML/refs/tags/${YAML_VERSION}/single_include/fkYAML/node.hpp"
    ${YAML_FILE})
endif()

# ╭──────────────────────────────────────╮
# │          libtreesitter-cpp           │
# ╰──────────────────────────────────────╯
set(BUILD_SHARED_LIBS OFF)
set(TREESITTER_URL
    https://github.com/tree-sitter/tree-sitter/archive/refs/tags/v0.25.6.tar.gz)
set(TREESITTER_TAR ${CMAKE_BINARY_DIR}/tree-sitter.tar.gz)
set(TREESITTER_SRC_DIR ${CMAKE_BINARY_DIR}/tree-sitter-v0.25.6)
if(NOT EXISTS ${TREESITTER_TAR})
  message(STATUS "Downloading tree-sitter-cpp...")
  file(DOWNLOAD ${TREESITTER_URL} ${TREESITTER_TAR})
endif()

if(NOT EXISTS ${TREESITTER_SRC_DIR})
  message(STATUS "Extracting tree-sitter-cpp...")
  file(ARCHIVE_EXTRACT INPUT ${TREESITTER_TAR} DESTINATION ${CMAKE_BINARY_DIR})
endif()

add_subdirectory(${CMAKE_BINARY_DIR}/tree-sitter-0.25.6/lib
                 ${CMAKE_BINARY_DIR}/tree-sitter EXCLUDE_FROM_ALL)
include_directories("${CMAKE_BINARY_DIR}/tree-sitter-0.25.6/lib/include/")

# ╭──────────────────────────────────────╮
# │         1. Baixar parser C           │
# ╰──────────────────────────────────────╯
set(TREESITTER_C_URL
    https://github.com/tree-sitter/tree-sitter-c/archive/refs/tags/v0.24.1.tar.gz
)
set(TREESITTER_C_TAR ${CMAKE_BINARY_DIR}/tree-sitter-c.tar.gz)
set(TREESITTER_C_SRC_DIR ${CMAKE_BINARY_DIR}/tree-sitter-c-0.24.1)

if(NOT EXISTS ${TREESITTER_C_TAR})
  message(STATUS "Downloading tree-sitter-c...")
  file(DOWNLOAD ${TREESITTER_C_URL} ${TREESITTER_C_TAR})
endif()

if(NOT EXISTS ${TREESITTER_C_SRC_DIR})
  message(STATUS "Extracting tree-sitter-c...")
  file(ARCHIVE_EXTRACT INPUT ${TREESITTER_C_TAR} DESTINATION
       ${CMAKE_BINARY_DIR})
endif()
add_library(tree-sitter-c STATIC "${TREESITTER_C_SRC_DIR}/src/parser.c")
set_target_properties(tree-sitter-c PROPERTIES CMAKE_POSITION_INDEPENDENT_CODE
                                               ON)

# ╭──────────────────────────────────────╮
# │         1. Baixar parser C++         │
# ╰──────────────────────────────────────╯
set(TREESITTER_CPP_URL
    https://github.com/tree-sitter/tree-sitter-cpp/archive/refs/tags/v0.23.4.tar.gz
)
set(TREESITTER_CPP_TAR ${CMAKE_BINARY_DIR}/tree-sitter-cpp.tar.gz)
set(TREESITTER_CPP_SRC_DIR ${CMAKE_BINARY_DIR}/tree-sitter-cpp-0.23.4)

if(NOT EXISTS ${TREESITTER_CPP_TAR})
  message(STATUS "Downloading tree-sitter-cpp...")
  file(DOWNLOAD ${TREESITTER_CPP_URL} ${TREESITTER_CPP_TAR})
endif()

if(NOT EXISTS ${TREESITTER_CPP_SRC_DIR})
  message(STATUS "Extracting tree-sitter-cpp...")
  file(ARCHIVE_EXTRACT INPUT ${TREESITTER_CPP_TAR} DESTINATION
       ${CMAKE_BINARY_DIR})
endif()

add_library(tree-sitter-cpp STATIC "${TREESITTER_CPP_SRC_DIR}/src/parser.c"
                                   "${TREESITTER_CPP_SRC_DIR}/src/scanner.c")
set_target_properties(tree-sitter-cpp PROPERTIES CMAKE_POSITION_INDEPENDENT_CODE
                                                 ON)

# ╭──────────────────────────────────────╮
# │               cxxopts                │
# ╰──────────────────────────────────────╯
set(CXXOPTS_FILE ${CMAKE_BINARY_DIR}/cxxopts.hpp)
set(CXXOPTS_VERSION "v3.3.1")
if(NOT EXISTS "${CXXOPTS_FILE}")
  file(
    DOWNLOAD
    "https://raw.githubusercontent.com/jarro2783/cxxopts/refs/tags/${CXXOPTS_VERSION}/include/cxxopts.hpp"
    ${CXXOPTS_FILE})
endif()

# ╭──────────────────────────────────────╮
# │                 json                 │
# ╰──────────────────────────────────────╯
set(JSON_FILE ${CMAKE_BINARY_DIR}/json.hpp)
set(JSON_VERSION "v3.12.0")
if(NOT EXISTS "${JSON_FILE}")
  file(
    DOWNLOAD
    "https://raw.githubusercontent.com/nlohmann/json/refs/tags/${JSON_VERSION}/single_include/nlohmann/json.hpp"
    ${JSON_FILE})
endif()

# ╭──────────────────────────────────────╮
# │    Download e extração do libgit2    │
# ╰──────────────────────────────────────╯
set(LIBGIT2_USE_ZLIB_FROM_SYSTEM ON) # força uso de libz.so do sistema
set(ZLIB_LIBRARY /usr/lib/libz.so) # ajuste o path se necessário
set(GIT_FILE ${CMAKE_BINARY_DIR}/libgit2.tar.gz)
if(NOT EXISTS ${GIT_FILE})
  message(STATUS "Downloading libgit2")
  file(DOWNLOAD
       https://github.com/libgit2/libgit2/archive/refs/tags/v1.9.1.tar.gz
       ${GIT_FILE})
endif()
file(ARCHIVE_EXTRACT INPUT ${GIT_FILE} DESTINATION ${CMAKE_BINARY_DIR})
include_directories(${CMAKE_BINARY_DIR}/libgit2-1.9.1/include)

# ╭──────────────────────────────────────╮
# │               libgit2                │
# ╰──────────────────────────────────────╯
set(BUILD_TESTS OFF)
set(BUILD_CLI OFF)
set(BUILD_CLAR OFF)
set(BUILD_SHARED_LIBS OFF)
add_subdirectory(${CMAKE_BINARY_DIR}/libgit2-1.9.1 ${CMAKE_BINARY_DIR}/libgit2
                 EXCLUDE_FROM_ALL)

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
target_link_libraries(pd4web_compiler PUBLIC libgit2package tree-sitter
                                             tree-sitter-cpp tree-sitter-c)
set_target_properties(pd4web_compiler PROPERTIES CMAKE_POSITION_INDEPENDENT_CODE
                                                 ON)
# ╭──────────────────────────────────────╮
# │               pybind11               │
# ╰──────────────────────────────────────╯
if(PD4WEB_BUILD_PYBIND11)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libstdc++ -static-libgcc")
  set(PYBIND11_FINDPYTHON ON)
  find_package(pybind11 REQUIRED)
  add_library(pypd4web MODULE ${CMAKE_CURRENT_SOURCE_DIR}/pybind11.cpp)
  target_link_libraries(pypd4web PRIVATE pybind11::module pd4web_compiler
                                         tree-sitter-cpp tree-sitter-c)
  set_target_properties(pypd4web PROPERTIES PREFIX "" OUTPUT_NAME "pypd4web")
  install(TARGETS pypd4web DESTINATION ./pd4web)
endif()

# ╭──────────────────────────────────────╮
# │           Executável final           │
# ╰──────────────────────────────────────╯
add_executable(pd4web "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
target_link_libraries(pd4web PRIVATE pd4web_compiler tree-sitter-cpp
                                     tree-sitter-c)
target_compile_options(pd4web PRIVATE $<$<CONFIG:Debug>:-g>)
install(TARGETS pd4web RUNTIME DESTINATION bin)
