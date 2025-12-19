project(pd4web-compiler)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

cmake_policy(SET CMP0077 NEW)
set(BUILD_TESTS OFF)
set(BUILD_CLI OFF)
set(USE_NSEC OFF)
set(BUILD_SHARED_LIBS
    OFF
    CACHE BOOL "" FORCE)

# http
set(HTTPLIB_USE_BROTLI_IF_AVAILABLE OFF)
set(HTTPLIB_USE_ZSTD_IF_AVAILABLE OFF)
set(HTTPLIB_USE_ZLIB_IF_AVAILABLE OFF)

set(TREE_SITTER_CLI "")
# ╭──────────────────────────────────────╮
# │             CPM Package              │
# ╰──────────────────────────────────────╯
set(CPM_FILE ${CMAKE_BINARY_DIR}/CPM.cmake)
set(CPM_VERSION "0.42.0")
if(NOT EXISTS "${CPM_FILE}")
    file(DOWNLOAD "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_VERSION}/CPM.cmake" ${CPM_FILE})
endif()
include(${CPM_FILE})

# ╭──────────────────────────────────────╮
# │             Dependencies             │
# ╰──────────────────────────────────────╯
# download unpack cmake and ninja binaries
cpmaddpackage("gh:yhirose/cpp-httplib#v0.26.0")
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
cpmaddpackage("gh:jarro2783/cxxopts#v3.3.1")
cpmaddpackage("gh:nlohmann/json#v3.12.0")
cpmaddpackage("gh:fktn-k/fkYAML#v0.4.2")

# # libgit
cpmaddpackage(URI "gh:jimmy-park/openssl-cmake#3.6.0" OPTIONS "OPENSSL_USE_STATIC_LIBS ON" "BUILD_SHARED_LIBS OFF")
cpmaddpackage(URI "gh:libgit2/libgit2#v1.9.2" OPTIONS "openssl_use_static_libs on" "build_shared_libs off")

cpmaddpackage(URI "gh:nabijaczleweli/whereami-cpp#v1.1.1" DOWNLOAD_ONLY)

if(whereami-cpp_ADDED)
    message(STATUS "${whereami-cpp_SOURCE_DIR}/include")
    include_directories("${whereami-cpp_SOURCE_DIR}/include")

else()
    message(FATAL_ERROR "Not found")
endif()

# https://github.com/nabijaczleweli/whereami-cpp.git

# ╭──────────────────────────────────────╮
# │                Boost                 │
# ╰──────────────────────────────────────╯
if(MSVC)
    set(Boost_USE_STATIC_LIBS
        ON
        CACHE BOOL "" FORCE)
    set(BOOST_BUILD_FLAGS
        "runtime-link=static;link=static"
        CACHE STRING "" FORCE)
    set(CMAKE_MSVC_RUNTIME_LIBRARY
        "MultiThreaded"
        CACHE STRING "" FORCE)
endif()

set(BOOST_VERSION "1.89.0")
cpmaddpackage(
    NAME
    Boost
    VERSION
    ${BOOST_VERSION}
    URL
    "https://github.com/boostorg/boost/releases/download/boost-${BOOST_VERSION}/boost-${BOOST_VERSION}-cmake.tar.xz"
    OPTIONS
    "BOOST_ENABLE_CMAKE ON"
    "BUILD_SHARED_LIBS OFF")

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

target_include_directories(pd4web_compiler PUBLIC ${CMAKE_BINARY_DIR} "${tree-sitter_SOURCE_DIR}/lib/include")
target_compile_definitions(pd4web_compiler PRIVATE BOOST_ALL_NO_LIB)

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
           Boost::process)

if(WIN32)
    target_link_libraries(pd4web_compiler PUBLIC Crypt32)
endif()

target_compile_definitions(pd4web_compiler PUBLIC LIBARCHIVE_STATIC)

if(MSVC)
    add_link_options(/LTCG)
endif()

if(APPLE)
    target_link_libraries(pd4web_compiler PUBLIC "-framework CoreFoundation" "-framework CFNetwork")
endif()

# ╭──────────────────────────────────────╮
# │               pybind11               │
# ╰──────────────────────────────────────╯
if(PD4WEB_BUILD_PYBIND11)
    find_package(Python REQUIRED COMPONENTS Interpreter Development)
    execute_process(
        COMMAND "${Python_EXECUTABLE}" -m pybind11 --cmakedir
        OUTPUT_VARIABLE PYBIND11_CMAKE_DIR
        OUTPUT_STRIP_TRAILING_WHITESPACE ERROR_QUIET)

    if(PYBIND11_CMAKE_DIR)
        list(APPEND CMAKE_PREFIX_PATH "${PYBIND11_CMAKE_DIR}")
        find_package(pybind11 CONFIG REQUIRED)
    else()
        set(PYBIND11_FINDPYTHON
            ON
            CACHE BOOL "" FORCE)
        find_package(pybind11 REQUIRED)
    endif()

    add_library(pypd4web MODULE ${CMAKE_CURRENT_SOURCE_DIR}/pybind11.cpp)
    target_link_libraries(pypd4web PRIVATE pybind11::module pd4web_compiler)
    set_target_properties(pypd4web PROPERTIES PREFIX "" OUTPUT_NAME "pypd4web")

    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/dist")
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/dist/pd4web")

    if(APPLE)
        set(CMAKE_SHARE_FOLDER "${PD4WEB_CMAKE_FOLDER}/CMake.app/Contents/share")
        set(CMAKE_BIN_FOLDER "${PD4WEB_CMAKE_FOLDER}/CMake.app/Contents/bin")
    else()
        set(CMAKE_SHARE_FOLDER "${PD4WEB_CMAKE_FOLDER}/share")
        set(CMAKE_BIN_FOLDER "${PD4WEB_CMAKE_FOLDER}/bin")
    endif()

    add_custom_target(
        pd4web_data ALL
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:pypd4web> "${CMAKE_BINARY_DIR}/dist/pd4web"
        COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/pd4web/__init__.py "${CMAKE_BINARY_DIR}/dist/pd4web"
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/../Pd4Web/ "${CMAKE_BINARY_DIR}/dist/pd4web/Pd4Web"
        # cmake
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SHARE_FOLDER} "${CMAKE_BINARY_DIR}/dist/pd4web/bin/cmake/share"
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_BIN_FOLDER} "${CMAKE_BINARY_DIR}/dist/pd4web/bin/cmake/bin"
        # ninja
        COMMAND ${CMAKE_COMMAND} -E copy "${PD4WEB_NINJA_BINARY}" "${CMAKE_BINARY_DIR}/dist/pd4web/bin"
        COMMENT "Copying pypd4web module and __init__.py to dist/"
        DEPENDS pypd4web
        VERBATIM)

    if(UNIX AND NOT APPLE)
        target_compile_options(pypd4web PRIVATE -ffunction-sections -fdata-sections)
        target_link_options(pypd4web PRIVATE -Wl,--gc-sections)
    elseif(APPLE)
        target_compile_options(pypd4web PRIVATE -ffunction-sections -fdata-sections)
        target_link_options(pypd4web PRIVATE -Wl,-dead_strip)
    elseif(MSVC)
        target_compile_options(pypd4web PRIVATE /Gy)
        target_link_options(pypd4web PRIVATE /OPT:REF /OPT:ICF)
    endif()

endif()

add_executable(pd4web ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
target_link_libraries(pd4web PRIVATE pd4web_compiler)
