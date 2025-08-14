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

# ╭──────────────────────────────────────╮
# │             CPM Package              │
# ╰──────────────────────────────────────╯
set(CPM_FILE ${CMAKE_BINARY_DIR}/CPM.cmake)
set(CPM_VERSION "0.42.0")
if(NOT EXISTS "${CPM_FILE}")
  file(
    DOWNLOAD "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_VERSION}/CPM.cmake"
    SHOW_PROGRESS
    ${CPM_FILE})
endif()
include(${CPM_FILE})

# ╭──────────────────────────────────────╮
# │             Dependencies             │
# ╰──────────────────────────────────────╯
CPMAddPackage("gh:yhirose/cpp-httplib#v0.25.0")
CPMAddPackage(
    NAME tree-sitter
    GITHUB_REPOSITORY tree-sitter/tree-sitter
    VERSION 0.25.6
    SOURCE_SUBDIR lib
)
CPMAddPackage("gh:tree-sitter/tree-sitter-c#v0.24.1")
CPMAddPackage(
    NAME tree-sitter-cpp
    VERSION 0.23.4
    GITHUB_REPOSITORY tree-sitter/tree-sitter-cpp
    PATCHES ${CMAKE_SOURCE_DIR}/Resources/rename-ts-test.patch
)
CPMAddPackage("gh:jarro2783/cxxopts#v3.3.1") # parse command line
CPMAddPackage("gh:nlohmann/json#v3.12.0") # read/save json files
CPMAddPackage("gh:fktn-k/fkYAML#v0.4.2") # read/save yaml files
CPMAddPackage(
    NAME openssl-cmake
    URL https://github.com/jimmy-park/openssl-cmake/archive/main.tar.gz
)

CPMAddPackage("gh:libgit2/libgit2#v1.9.1")

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
target_link_libraries(pd4web_compiler PUBLIC 
            tree-sitter tree-sitter-cpp tree-sitter-c 
            cxxopts 
            nlohmann_json fkYAML
            httplib
            # libgit2
            libgit2package
            OpenSSL::SSL OpenSSL::Crypto
)
set_target_properties(pd4web_compiler PROPERTIES CMAKE_POSITION_INDEPENDENT_CODE ON)

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
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/pd4web/__init__.py DESTINATION pd4web)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../Pd4Web DESTINATION pd4web)
else()
    add_executable(pd4web "${CMAKE_CURRENT_SOURCE_DIR}/main.cpp")
    target_link_libraries(pd4web PRIVATE pd4web_compiler)
    target_compile_options(pd4web PRIVATE $<$<CONFIG:Debug>:-g>)

    install(TARGETS pd4web RUNTIME DESTINATION bin)
endif()
