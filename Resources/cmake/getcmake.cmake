set(PD4WEB_CMAKE_VERSION "4.2.1")
set(CMAKE_OUTPUT_DIR "${CMAKE_BINARY_DIR}/cmake")
set(CMAKE_ARCHIVE_DIR "${CMAKE_BINARY_DIR}")

# --------------------------
# Detect platform/architecture
# --------------------------
if(WIN32)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(CMAKE_ASSET "cmake-${PD4WEB_CMAKE_VERSION}-windows-x86_64.zip")
  else()
    message(FATAL_ERROR "Unsupported Windows architecture")
  endif()
elseif(APPLE)
  set(CMAKE_ASSET "cmake-${PD4WEB_CMAKE_VERSION}-macos-universal.tar.gz")
elseif(UNIX)
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    set(CMAKE_ASSET "cmake-${PD4WEB_CMAKE_VERSION}-linux-x86_64.tar.gz")
  elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    set(CMAKE_ASSET "cmake-${PD4WEB_CMAKE_VERSION}-linux-aarch64.tar.gz")
  else()
    message(
      FATAL_ERROR "Unsupported Linux architecture: ${CMAKE_SYSTEM_PROCESSOR}")
  endif()
else()
  message(FATAL_ERROR "Unsupported platform")
endif()

set(CMAKE_URL
    "https://github.com/Kitware/CMake/releases/download/v${PD4WEB_CMAKE_VERSION}/${CMAKE_ASSET}"
)
set(CMAKE_ARCHIVE "${CMAKE_ARCHIVE_DIR}/${CMAKE_ASSET}")

# --------------------------
# Download archive if missing
# --------------------------
if(NOT EXISTS "${CMAKE_ARCHIVE}")
  message(STATUS "Downloading CMake from ${CMAKE_URL}")
  file(DOWNLOAD "${CMAKE_URL}" "${CMAKE_ARCHIVE}" STATUS status)
  list(GET status 0 status_code)
  if(NOT status_code EQUAL 0)
    list(GET status 1 status_msg)
    message(FATAL_ERROR "Failed to download CMake: ${status_msg}")
  endif()
endif()

file(MAKE_DIRECTORY "${CMAKE_OUTPUT_DIR}")

if(CMAKE_ASSET MATCHES "\\.zip$")
  message(STATUS "Extracting ZIP to ${CMAKE_OUTPUT_DIR}")
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf "${CMAKE_ARCHIVE}"
                  WORKING_DIRECTORY "${CMAKE_OUTPUT_DIR}")
elseif(CMAKE_ASSET MATCHES "\\.tar\\.gz$")
  message(STATUS "Extracting TAR.GZ to ${CMAKE_OUTPUT_DIR}")
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf "${CMAKE_ARCHIVE}"
                  WORKING_DIRECTORY "${CMAKE_OUTPUT_DIR}")
else()
  message(FATAL_ERROR "Unknown archive format: ${CMAKE_ASSET}")
endif()

# --------------------------
# Extract archive
# --------------------------
file(MAKE_DIRECTORY "${CMAKE_OUTPUT_DIR}")

# Extract once
execute_process(
  COMMAND ${CMAKE_COMMAND} -E tar xzf "${CMAKE_ARCHIVE}"
  WORKING_DIRECTORY "${CMAKE_OUTPUT_DIR}"
  RESULT_VARIABLE extract_result
)

if(NOT extract_result EQUAL 0)
  message(FATAL_ERROR "Failed to extract ${CMAKE_ASSET}")
endif()

# Detect extracted directory
file(GLOB _cmake_dirs "${CMAKE_OUTPUT_DIR}/cmake-*")
list(LENGTH _cmake_dirs _dir_count)

if(NOT _dir_count EQUAL 1)
  message(FATAL_ERROR "Expected exactly one extracted cmake directory")
endif()

list(GET _cmake_dirs 0 EXTRACTED_DIR)

# Set cmake binary path
if(WIN32)
  set(PD4WEB_CMAKE_BINARY "${EXTRACTED_DIR}/bin/cmake.exe")
elseif(APPLE AND EXISTS "${EXTRACTED_DIR}/CMake.app/Contents/bin/cmake")
  set(PD4WEB_CMAKE_BINARY "${EXTRACTED_DIR}/CMake.app/Contents/bin/cmake")
else()
  set(PD4WEB_CMAKE_BINARY "${EXTRACTED_DIR}/bin/cmake")
endif()

set(PD4WEB_CMAKE_FOLDER "${EXTRACTED_DIR}")

