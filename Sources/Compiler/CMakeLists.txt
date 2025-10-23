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

# http
set(HTTPLIB_USE_BROTLI_IF_AVAILABLE OFF)
set(HTTPLIB_USE_ZSTD_IF_AVAILABLE OFF)
set(HTTPLIB_USE_ZLIB_IF_AVAILABLE OFF)

# libarchive
set(ENABLE_WERROR
    OFF
    CACHE BOOL "" FORCE)
# Criptografia
set(ENABLE_MBEDTLS
    OFF
    CACHE BOOL "" FORCE)
set(ENABLE_NETTLE
    OFF
    CACHE BOOL "" FORCE)
set(ENABLE_OPENSSL
    OFF
    CACHE BOOL "" FORCE)
set(ENABLE_CNG
    OFF
    CACHE BOOL "" FORCE)

# Compressão / hashing
set(ENABLE_LIBB2
    OFF
    CACHE BOOL "" FORCE)
set(ENABLE_LZ4
    OFF
    CACHE BOOL "" FORCE)
set(ENABLE_LZO
    OFF
    CACHE BOOL "" FORCE)
set(ENABLE_LZMA
    OFF
    CACHE BOOL "" FORCE)
set(ENABLE_ZSTD
    OFF
    CACHE BOOL "" FORCE)
set(ENABLE_BZip2
    OFF
    CACHE BOOL "" FORCE)

# Parsers
set(ENABLE_LIBXML2
    OFF
    CACHE BOOL "" FORCE)
set(ENABLE_EXPAT
    OFF
    CACHE BOOL "" FORCE)
set(ENABLE_WIN32_XMLLITE
    OFF
    CACHE BOOL "" FORCE)
set(ENABLE_PCREPOSIX
    OFF
    CACHE BOOL "" FORCE)
set(ENABLE_PCRE2POSIX
    OFF
    CACHE BOOL "" FORCE)

# Arquivos / utilitários
set(ENABLE_TAR
    OFF
    CACHE BOOL "" FORCE)
set(ENABLE_TAR_SHARED
    OFF
    CACHE BOOL "" FORCE)
set(ENABLE_CPIO
    OFF
    CACHE BOOL "" FORCE)
set(ENABLE_CPIO_SHARED
    OFF
    CACHE BOOL "" FORCE)
set(ENABLE_CAT
    OFF
    CACHE BOOL "" FORCE)
set(ENABLE_CAT_SHARED
    OFF
    CACHE BOOL "" FORCE)
set(ENABLE_UNZIP_SHARED
    OFF
    CACHE BOOL "" FORCE)

# Sistema
set(ENABLE_XATTR
    OFF
    CACHE BOOL "" FORCE)
set(ENABLE_ACL
    OFF
    CACHE BOOL "" FORCE)

# Testes / instalação
set(ENABLE_TEST
    OFF
    CACHE BOOL "" FORCE)
set(ENABLE_COVERAGE
    OFF
    CACHE BOOL "" FORCE)
set(ENABLE_INSTALL
    OFF
    CACHE BOOL "" FORCE)
set(ENABLE_ZLIB OFF)

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
# download unpack cmake and ninja binaries
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

# Tree-sitter
cpmaddpackage("gh:tree-sitter/tree-sitter-c#v0.24.1")
cpmaddpackage(
  NAME
  tree-sitter-cpp
  VERSION
  0.23.4
  GITHUB_REPOSITORY
  tree-sitter/tree-sitter-cpp
  PATCH_COMMAND
  ${CMAKE_COMMAND}
  -E
  echo
  "Applying patch..."
  &&
  patch
  -p1
  -N
  -d
  ${CMAKE_BINARY_DIR}/_deps/tree-sitter-cpp-src
  -i
  ${CMAKE_SOURCE_DIR}/Resources/rename-ts-test.patch
  ||
  true
  EXCLUDE_FROM_ALL
  YES)

# args (for pybind11 and pd4web executable)
cpmaddpackage("gh:jarro2783/cxxopts#v3.3.1") # parse command line

# files
cpmaddpackage("gh:nlohmann/json#v3.12.0") # read/save json files
cpmaddpackage("gh:fktn-k/fkYAML#v0.4.2") # read/save yaml files

# libgit
set(OPENSSL_USE_STATIC_LIBS
    TRUE
    CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS
    OFF
    CACHE BOOL "" FORCE)
cpmaddpackage("gh:jimmy-park/openssl-cmake#3.6.0")
cpmaddpackage("gh:libgit2/libgit2#v1.9.1")
cpmaddpackage(
  NAME
  libarchive
  GITHUB_REPOSITORY
  libarchive/libarchive
  GIT_TAG
  v3.8.2
  OPTIONS
  "ENABLE_MBEDTLS OFF"
  "ENABLE_NETTLE OFF"
  "ENABLE_OPENSSL OFF"
  "ENABLE_CNG OFF"
  "ENABLE_LIBB2 OFF"
  "ENABLE_LZ4 OFF"
  "ENABLE_LZO OFF"
  "ENABLE_LZMA OFF"
  "ENABLE_ZSTD OFF"
  "ENABLE_BZip2 OFF"
  "ENABLE_LIBXML2 OFF"
  "ENABLE_EXPAT OFF"
  "ENABLE_WIN32_XMLLITE OFF"
  "ENABLE_PCREPOSIX OFF"
  "ENABLE_PCRE2POSIX OFF"
  "ENABLE_TAR OFF"
  "ENABLE_TAR_SHARED OFF"
  "ENABLE_CPIO OFF"
  "ENABLE_CPIO_SHARED OFF"
  "ENABLE_CAT OFF"
  "ENABLE_CAT_SHARED OFF"
  "ENABLE_UNZIP_SHARED OFF"
  "ENABLE_XATTR OFF"
  "ENABLE_ACL OFF"
  "ENABLE_TEST OFF"
  "ENABLE_COVERAGE OFF"
  "ENABLE_INSTALL OFF"
  "HAVE_ID_T ON")

# boost (process)
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
  "${CMAKE_CURRENT_SOURCE_DIR}/builder.cpp"
  "${CMAKE_CURRENT_SOURCE_DIR}/pd4web_compiler.cpp")

target_include_directories(
  pd4web_compiler PUBLIC ${CMAKE_BINARY_DIR}
                         "${tree-sitter_SOURCE_DIR}/lib/include")
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
         OpenSSL::SSL
         OpenSSL::Crypto
         Boost::process
         archive_static)

target_compile_definitions(pd4web_compiler PRIVATE CPPHTTPLIB_OPENSSL_SUPPORT
                                                   LIBARCHIVE_STATIC)

if(MSVC)
  add_link_options(/LTCG)
endif()

# ╭──────────────────────────────────────╮
# │               pybind11               │
# ╰──────────────────────────────────────╯
if(PD4WEB_BUILD_PYBIND11)
  set(PYBIND11_FINDPYTHON ON)
  find_package(pybind11 REQUIRED)
  add_library(pypd4web MODULE ${CMAKE_CURRENT_SOURCE_DIR}/pybind11.cpp)
  target_link_libraries(pypd4web PRIVATE pybind11::module pd4web_compiler)
  set_target_properties(pypd4web PROPERTIES PREFIX "" OUTPUT_NAME "pypd4web")

  install(TARGETS pypd4web DESTINATION pd4web)
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/pd4web/__init__.py
          DESTINATION pd4web)
  install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../Pd4Web DESTINATION pd4web)
endif()
