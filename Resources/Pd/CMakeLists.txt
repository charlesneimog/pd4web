cmake_minimum_required(VERSION 3.20)
project(pd4web)

set(PDCMAKE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/../../Sources/Libraries/pd.cmake/"
    CACHE PATH "Path to pd.cmake")
include(${PDCMAKE_DIR}/pd.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# ╭──────────────────────────────────────╮
# │            Pd4Web Python             │
# ╰──────────────────────────────────────╯
# check if pyinstaller is installed
find_program(PYINSTALLER_EXECUTABLE pyinstaller)

if(NOT PYINSTALLER_EXECUTABLE)
    message(
        FATAL_ERROR
            "PyInstaller not found. Please install PyInstaller using 'python -m pip install pyinstaller'."
    )
else()
    message(STATUS "PyInstaller found: ${PYINSTALLER_EXECUTABLE}")
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(PLATFORM "l-")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(PLATFORM "w-")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(PLATFORM "m-")
else()
    message(FATAL_ERROR "Unknown platform")
endif()

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64")
    set(ARCHITECTURE "x86")
elseif(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm" OR ${CMAKE_SYSTEM_PROCESSOR} MATCHES "aarch64")
    set(ARCHITECTURE "arm")
else()
    message(FATAL_ERROR "Unknown architecture")
endif()

set(ARCH "${PLATFORM}${ARCHITECTURE}")

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    message(STATUS "${CMAKE_CURRENT_SOURCE_DIR}/hooks/")
    set(PYINSTALLER_COMMAND
        ${PYINSTALLER_EXECUTABLE} --onefile ${CMAKE_SOURCE_DIR}/pd-pd4web.py --name
        "pd4web-${ARCH}" --additional-hooks-dir "${CMAKE_CURRENT_SOURCE_DIR}/hooks/" --add-data
        "${CMAKE_SOURCE_DIR}/../../Sources:pd4web" --distpath "${CMAKE_BINARY_DIR}")

    # MACOS
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(PYINSTALLER_COMMAND
    ${PYINSTALLER_EXECUTABLE} --onefile ${CMAKE_SOURCE_DIR}/pd-pd4web.py --name
    "pd4web-${ARCH}" --additional-hooks-dir "${CMAKE_CURRENT_SOURCE_DIR}/hooks/" --add-data
    "${CMAKE_SOURCE_DIR}/../../Sources:pd4web" --distpath "${CMAKE_BINARY_DIR}")
else()
    message(FATAL_ERROR "Unknown platform")
endif()

add_custom_target(
    pd_pd4web ALL
    COMMAND ${PYINSTALLER_COMMAND}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running PyInstaller to create the pd4web executable")

# ╭──────────────────────────────────────╮
# │              Pd Object               │
# ╰──────────────────────────────────────╯
pd_add_external(pd4web pd4web.cpp)
pd_add_datafile(pd4web pd4web-help.pd)
pd_add_datafile(pd4web "${CMAKE_BINARY_DIR}/pd4web-${ARCH}")

add_dependencies(pd_pd4web pd4web)
