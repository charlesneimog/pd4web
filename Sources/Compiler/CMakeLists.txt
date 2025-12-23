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

# libgit
set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
set(OPENSSL_USE_STATIC_LIBS TRUE)
set(OPENSSL_MSVC_STATIC_RT TRUE)
set(BUILD_SHARED_LIBS OFF)


cpmaddpackage(
    NAME
    libgit2
    GITHUB_REPOSITORY
    libgit2/libgit2
    VERSION
    1.9.2
    OPTIONS
    "OPENSSL_USE_STATIC_LIBS ON"
    "BUILD_SHARED_LIBS OFF"
    "BUILD_CLI OFF"
    "BUILD_TESTS OFF"
    "BUILD_EXAMPLES OFF"
)


cpmaddpackage(
    URI
    "gh:jimmy-park/openssl-cmake#3.6.0"
    OPTIONS
    "OPENSSL_USE_STATIC_LIBS ON"
    "BUILD_SHARED_LIBS OFF"
    "BUILD_EXAMPLES OFF"
    "BUILD_TESTING OFF")

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
    PUBLIC OpenSSL::SSL
           OpenSSL::Crypto
           httplib
           tree-sitter
           tree-sitter-cpp
           tree-sitter-c
           cxxopts
           fkYAML
           nlohmann_json
           libgit2
           libgit2package
           Boost::process)
set_property(TARGET pd4web_compiler PROPERTY LINK_SEARCH_START_STATIC TRUE)
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
if(PY_PD4WEB_BUILD)
    find_package(Python3 3.11 REQUIRED COMPONENTS Interpreter Development.SABIModule)
    python3_add_library(pypd4web MODULE pypd4web.cpp USE_SABI 3.11)
    target_compile_definitions(pypd4web PRIVATE Py_LIMITED_API=0x030B0000)
    target_link_libraries(pypd4web PRIVATE pd4web_compiler)

    if(WIN32)
        execute_process(
            COMMAND ${Python_EXECUTABLE} -c "import sys; print(sys.base_prefix)"
            OUTPUT_VARIABLE PYTHON_BASE_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE)
        target_link_directories(pypd4web PRIVATE "${PYTHON_BASE_PREFIX}/libs")
    endif()

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

    install(
        TARGETS pypd4web
        LIBRARY DESTINATION pd4web
        RUNTIME DESTINATION pd4web
        ARCHIVE DESTINATION pd4web)
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/pd4web/__init__.py DESTINATION pd4web)
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../Pd4Web/ DESTINATION pd4web/Pd4Web)
    install(DIRECTORY ${CMAKE_BIN_FOLDER}/ DESTINATION pd4web/bin/cmake/bin)
    install(DIRECTORY ${CMAKE_SHARE_FOLDER}/ DESTINATION pd4web/bin/cmake/share)
    install(FILES ${PD4WEB_NINJA_BINARY} DESTINATION pd4web/bin)

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
