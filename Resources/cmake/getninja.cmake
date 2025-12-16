cmake_minimum_required(VERSION 3.25)
project(GetNinjaBinary)

# --------------------------
# Configuration
# --------------------------
set(NINJA_VERSION "1.13.2")
set(NINJA_OUTPUT_DIR "${CMAKE_BINARY_DIR}/ninja")
set(NINJA_ARCHIVE_DIR "${CMAKE_BINARY_DIR}")

# --------------------------
# Detect platform/architecture
# --------------------------
if(WIN32)
  if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(NINJA_ASSET "ninja-win.zip")
  else()
    message(FATAL_ERROR "Unsupported Windows architecture")
  endif()
elseif(APPLE)
  set(NINJA_ASSET "ninja-mac.zip")
elseif(UNIX)
  if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    set(NINJA_ASSET "ninja-linux.zip")
  elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "aarch64")
    set(NINJA_ASSET "ninja-linux-aarch64.zip")
  else()
    message(
      FATAL_ERROR "Unsupported Linux architecture: ${CMAKE_SYSTEM_PROCESSOR}")
  endif()
else()
  message(FATAL_ERROR "Unsupported platform")
endif()

set(NINJA_URL
    "https://github.com/ninja-build/ninja/releases/download/v${NINJA_VERSION}/${NINJA_ASSET}"
)
set(NINJA_ARCHIVE "${NINJA_ARCHIVE_DIR}/${NINJA_ASSET}")

# --------------------------
# Download archive if missing
# --------------------------
if(NOT EXISTS "${NINJA_ARCHIVE}")
  message(STATUS "Downloading Ninja from ${NINJA_URL}")
  file(DOWNLOAD "${NINJA_URL}" "${NINJA_ARCHIVE}" STATUS status)
  list(GET status 0 status_code)
  if(NOT status_code EQUAL 0)
    list(GET status 1 status_msg)
    message(FATAL_ERROR "Failed to download Ninja: ${status_msg}")
  endif()
endif()

# --------------------------
# Create output folder
# --------------------------
file(MAKE_DIRECTORY "${NINJA_OUTPUT_DIR}")

# --------------------------
# Extract archive
# --------------------------
if(NINJA_ASSET MATCHES "\\.zip$")
  message(STATUS "Extracting ZIP to ${NINJA_OUTPUT_DIR}")
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf "${NINJA_ARCHIVE}"
                  WORKING_DIRECTORY "${NINJA_OUTPUT_DIR}")
elseif(NINJA_ASSET MATCHES "\\.tar\\.gz$")
  message(STATUS "Extracting TAR.GZ to ${NINJA_OUTPUT_DIR}")
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf "${NINJA_ARCHIVE}"
                  WORKING_DIRECTORY "${NINJA_OUTPUT_DIR}")
else()
  message(FATAL_ERROR "Unknown archive format: ${NINJA_ASSET}")
endif()

# --------------------------
# Set path to ninja executable
# --------------------------
if(WIN32)
  set(NINJA_BINARY "${NINJA_OUTPUT_DIR}/ninja.exe")
elseif(LINUX)
  file(RENAME "${NINJA_OUTPUT_DIR}/ninja" "${NINJA_OUTPUT_DIR}/ninja-linux")
  set(NINJA_BINARY "${NINJA_OUTPUT_DIR}/ninja-linux")
elseif(APPLE)
  file(RENAME "${NINJA_OUTPUT_DIR}/ninja" "${NINJA_OUTPUT_DIR}/ninja-mac")
  set(NINJA_BINARY "${NINJA_OUTPUT_DIR}/ninja-mac")
else()
  message(FATAL_ERROR "Unknown plataform for ninja")
endif()

set(PD4WEB_NINJA_BINARY ${NINJA_BINARY})
