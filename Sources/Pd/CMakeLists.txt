cmake_minimum_required(VERSION 3.28)
project(pd4web)

# ──────────────────────────────────────
set(PDCMAKE_FILE ${CMAKE_BINARY_DIR}/pd.cmake)
set(PDCMAKE_VERSION "v0.2.18")
if(NOT EXISTS "${PDCMAKE_FILE}")
    file(DOWNLOAD "https://raw.githubusercontent.com/pure-data/pd.cmake/refs/tags/v2.0.18/pd.cmake" ${PDCMAKE_FILE})
endif()

include("${PDCMAKE_FILE}")

# ╭──────────────────────────────────────╮
# │                Objeto                │
# ╰──────────────────────────────────────╯
pd_add_external(pd4web.compiler "${CMAKE_CURRENT_SOURCE_DIR}/pd4web.compiler.cpp")
target_link_libraries(pd4web.compiler PRIVATE pd4web_compiler)
target_include_directories(pd4web.compiler PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/../Compiler/")
set_target_properties(pd4web.compiler PROPERTIES CMAKE_POSITION_INDEPENDENT_CODE ON)

# Pd4Web Data
pd_add_datafile(pd4web.compiler "${CMAKE_CURRENT_SOURCE_DIR}/pd4web.compiler.cpp")
pd_add_datafile(pd4web.compiler "${CMAKE_CURRENT_SOURCE_DIR}/pd4web.pd")
pd_add_datafile(pd4web.compiler "${CMAKE_CURRENT_SOURCE_DIR}/README.deken.pd")

# pd.cmake 2.0.18 honors DESTINATION for install rules, but its build-time directory copy always
# writes directly to PD_OUTPUT_PATH. Stage directories ourselves so the build artifact has the
# same layout as the installed package.
if(PD_OUTPUT_PATH)
    if(IS_ABSOLUTE "${PD_OUTPUT_PATH}")
        set(PD4WEB_PD_PACKAGE_DIR "${PD_OUTPUT_PATH}")
    else()
        set(PD4WEB_PD_PACKAGE_DIR "${CMAKE_CURRENT_BINARY_DIR}/${PD_OUTPUT_PATH}")
    endif()
else()
    set(PD4WEB_PD_PACKAGE_DIR "${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}")
endif()
cmake_path(NORMAL_PATH PD4WEB_PD_PACKAGE_DIR)

set(PD4WEB_PD_INSTALL_DIR "${PD_LIB_DIR}/${CMAKE_PROJECT_NAME}")

function(pd4web_add_data_directory source destination)
    add_custom_command(
        TARGET pd4web.compiler
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory "${source}"
                "${PD4WEB_PD_PACKAGE_DIR}/${destination}"
        VERBATIM)
    install(DIRECTORY "${source}/" DESTINATION "${PD4WEB_PD_INSTALL_DIR}/${destination}")
endfunction()

pd4web_add_data_directory("${CMAKE_CURRENT_SOURCE_DIR}/1-hello-world" "1-hello-world")
pd4web_add_data_directory("${CMAKE_CURRENT_SOURCE_DIR}/2-p5js" "2-p5js")
pd4web_add_data_directory("${CMAKE_CURRENT_SOURCE_DIR}/../Pd4Web" "Pd4Web/Pd4Web")

if(APPLE)
    set(PD4WEB_CMAKE_BIN_DIR "${PD4WEB_CMAKE_FOLDER}/CMake.app/Contents/bin")
    set(PD4WEB_CMAKE_SHARE_DIR "${PD4WEB_CMAKE_FOLDER}/CMake.app/Contents/share")
else()
    set(PD4WEB_CMAKE_BIN_DIR "${PD4WEB_CMAKE_FOLDER}/bin")
    set(PD4WEB_CMAKE_SHARE_DIR "${PD4WEB_CMAKE_FOLDER}/share")
endif()

pd4web_add_data_directory("${PD4WEB_CMAKE_BIN_DIR}" "Pd4Web/bin/cmake/bin")
pd4web_add_data_directory("${PD4WEB_CMAKE_SHARE_DIR}" "Pd4Web/bin/cmake/share")

add_custom_command(
    TARGET pd4web.compiler
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${PD4WEB_PD_PACKAGE_DIR}/Pd4Web/bin"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${PD4WEB_NINJA_BINARY}"
            "${PD4WEB_PD_PACKAGE_DIR}/Pd4Web/bin"
    VERBATIM)
install(FILES "${PD4WEB_NINJA_BINARY}" DESTINATION "${PD4WEB_PD_INSTALL_DIR}/Pd4Web/bin")

set_target_properties(
    pd4web.compiler
    PROPERTIES C_VISIBILITY_PRESET hidden
               CXX_VISIBILITY_PRESET hidden
               VISIBILITY_INLINES_HIDDEN YES)

if(UNIX AND NOT APPLE)
    target_compile_options(pd4web.compiler PRIVATE -ffunction-sections -fdata-sections)
    target_link_options(pd4web.compiler PRIVATE -Wl,--gc-sections)
elseif(APPLE)
    target_compile_options(pd4web.compiler PRIVATE -ffunction-sections -fdata-sections)
    target_link_options(pd4web.compiler PRIVATE -Wl,-dead_strip)

elseif(MSVC)
    target_compile_options(pd4web.compiler PRIVATE /Gy)
    target_link_options(
        pd4web.compiler
        PRIVATE
        /OPT:REF
        /OPT:ICF)
elseif(MINGW)
    target_compile_options(pd4web.compiler PRIVATE -ffunction-sections -fdata-sections)
    target_link_options(pd4web.compiler PRIVATE -Wl,--gc-sections)
endif()
