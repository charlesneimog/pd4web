cmake_minimum_required(VERSION 3.25)
project(pdlua)
set(LIB_DIR ${PD4WEB_EXTERNAL_DIR}/${PROJECT_NAME})

if(NOT TARGET lua)
  add_library(lua "${LIB_DIR}/lua/onelua.c")
  target_compile_definitions(lua PUBLIC "-DMAKE_LIB")
endif()

# ╭──────────────────────────────────────╮
# │              DEJAVU TTF              │
# ╰──────────────────────────────────────╯
set(DEJAVU_BASE "${CMAKE_BINARY_DIR}/dejavu-sans-ttf")
set(DEJAVU_TMP "${CMAKE_BINARY_DIR}/dejavu-sans-tmp")
set(DEJAVU_ZIP "${CMAKE_BINARY_DIR}/dejavu-sans-ttf.zip")
set(DEJAVU_TTF "${DEJAVU_BASE}/ttf/DejaVuSans.ttf")

if(NOT EXISTS "${DEJAVU_TTF}")
  message(STATUS "Downloading DejaVu Sans TTF…")
  file(
    DOWNLOAD
    https://github.com/dejavu-fonts/dejavu-fonts/releases/download/version_2_37/dejavu-sans-ttf-2.37.zip
    "${DEJAVU_ZIP}"
    SHOW_PROGRESS)

  message(STATUS "Unzipping DejaVu Sans TTF…")
  file(MAKE_DIRECTORY "${DEJAVU_TMP}")
  file(ARCHIVE_EXTRACT INPUT "${DEJAVU_ZIP}" DESTINATION "${DEJAVU_TMP}")

  file(GLOB FONT_DIR "${DEJAVU_TMP}/dejavu-sans-ttf-*")
  list(GET FONT_DIR 0 FONT_PATH)
  file(MAKE_DIRECTORY "${DEJAVU_BASE}")
  file(RENAME "${FONT_PATH}/ttf" "${DEJAVU_BASE}/ttf")
endif()

set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} --preload-file=${DEJAVU_TTF}@/dejavu.ttf")

# ╭──────────────────────────────────────╮
# │                NANOVG                │
# ╰──────────────────────────────────────╯
set(NANOVG_DIR "${CMAKE_BINARY_DIR}/nanovg")
set(NANOVG_ZIP "${CMAKE_BINARY_DIR}/nanovg.zip")
if(NOT EXISTS ${NANOVG_DIR})
  message(STATUS "Downloading NanoVG...")
  file(
    DOWNLOAD
    https://github.com/rgb2hsv/nanovg/archive/refs/heads/development.zip
    ${NANOVG_ZIP}
    SHOW_PROGRESS
    STATUS DOWNLOAD_STATUS)

  message(STATUS "Unzipping NanoVG...")
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf ${NANOVG_ZIP}
                  WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

  file(GLOB NANOVG_EXTRACTED_DIR "${CMAKE_BINARY_DIR}/nanovg-*")
  file(RENAME ${NANOVG_EXTRACTED_DIR} ${NANOVG_DIR})
endif()
add_subdirectory(${NANOVG_DIR} EXCLUDE_FROM_ALL)
include_directories("${NANOVG_DIR}/src")

include_directories("${LIB_DIR}/lua")
pd_add_external(pdlua "${LIB_DIR}/pdlua.c")
target_link_libraries(pdlua PUBLIC lua nanovg)
target_include_directories(pdlua PRIVATE "${PD4WEB_EXTERNAL_DIR}/../")

# set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --preload-file
# \"${DEJAVU_DIR}/ttf/DejaVuSans.ttf@/dejavu.ttf\""
# )
set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} --preload-file \"${LIB_DIR}/pd.lua@/pd.lua\"")
set(CMAKE_EXE_LINKER_FLAGS
    "${CMAKE_EXE_LINKER_FLAGS} --preload-file \"${LIB_DIR}/pdlua/tutorial/examples/pdx.lua@/pdx.lua\""
)
