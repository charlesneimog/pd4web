cmake_minimum_required(VERSION 3.20)
project(pd4web)

set(PDCMAKE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/../../Sources/Libraries/pd.cmake/"
    CACHE PATH "Path to pd.cmake")
include(${PDCMAKE_DIR}/pd.cmake)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# ╭──────────────────────────────────────╮
# │            Pd4Web Python             │
# ╰──────────────────────────────────────╯
find_program(PYINSTALLER_EXECUTABLE pyinstaller)
if(NOT PYINSTALLER_EXECUTABLE)
  message(
    FATAL_ERROR
      "PyInstaller not found. Please install PyInstaller using 'python -m pip install pyinstaller'."
  )
else()
  message(STATUS "PyInstaller found: ${PYINSTALLER_EXECUTABLE}")
endif()

# Executable name
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  set(PLATFORM "l-")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  set(PLATFORM "w-")
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
  set(PLATFORM "m-")
else()
  message(FATAL_ERROR "Unknown platform")
endif()

# x86_64

message(STATUS "CMAKE_SYSTEM_PROCESSOR: ${CMAKE_SYSTEM_PROCESSOR}")

if(${CMAKE_SYSTEM_PROCESSOR} MATCHES "x86_64" OR ${CMAKE_SYSTEM_PROCESSOR}
                                                 MATCHES "AMD64")
  set(ARCHITECTURE "x86")
elseif(${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm" OR ${CMAKE_SYSTEM_PROCESSOR}
                                                  MATCHES "aarch64")
  set(ARCHITECTURE "arm")
else()
  message(FATAL_ERROR "Unknown architecture")
endif()
set(ARCH "${PLATFORM}${ARCHITECTURE}")

# ╭──────────────────────────────────────╮
# │      Compile using pyinstaller       │
# ╰──────────────────────────────────────╯

# ╭──────────────────────────────────────╮
# │                Linux                 │
# ╰──────────────────────────────────────╯
if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  # find where is tar
  find_program(TAR_BIN tar)

  if(TAR_BIN)
    message(STATUS "Found binary: ${TAR_BIN}")
  else()
    message(FATAL_ERROR "Binary of tar was not found, please install it")
  endif()

  # add tar as binary

  set(PYINSTALLER_COMMAND
      ${PYINSTALLER_EXECUTABLE} --onefile ${CMAKE_SOURCE_DIR}/pd-pd4web.py
      --name "pd4web-${ARCH}" --add-binary "${TAR_BIN}:bin"
      --additional-hooks-dir "${CMAKE_CURRENT_SOURCE_DIR}/hooks/" --distpath
      "${CMAKE_BINARY_DIR}")

  # ╭──────────────────────────────────────╮
  # │                MacOS                 │
  # ╰──────────────────────────────────────╯
elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
  set(PYINSTALLER_COMMAND
      ${PYINSTALLER_EXECUTABLE} --onefile --name "pd4web-${ARCH}" --distpath
      "${CMAKE_BINARY_DIR}" --additional-hooks-dir
      "${CMAKE_CURRENT_SOURCE_DIR}/hooks/" ${CMAKE_SOURCE_DIR}/pd-pd4web.py)

  # ╭──────────────────────────────────────╮
  # │               Windows                │
  # ╰──────────────────────────────────────╯

elseif(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  set(PYINSTALLER_COMMAND
      ${PYINSTALLER_EXECUTABLE} --onefile --name "pd4web-${ARCH}" --distpath
      "${CMAKE_BINARY_DIR}" --additional-hooks-dir
      "${CMAKE_CURRENT_SOURCE_DIR}/hooks/" ${CMAKE_SOURCE_DIR}/pd-pd4web.py)

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

if(WIN32)
  pd_add_datafile(pd4web "${CMAKE_BINARY_DIR}/pd4web-${ARCH}.exe")
else()
  pd_add_datafile(pd4web "${CMAKE_BINARY_DIR}/pd4web-${ARCH}")
endif()

set_target_properties(
  pd4web
  PROPERTIES CXX_STANDARD 17
             CXX_STANDARD_REQUIRED ON
             CXX_EXTENSIONS OFF)

add_dependencies(pd_pd4web pd4web)
