# Generated pd4web projects set thorvg_SOURCE_DIR to their bundled ThorVG 1.1.0 tree.
file(GLOB THORVG_SOURCES
    "${thorvg_SOURCE_DIR}/src/common/*.cpp"
    "${thorvg_SOURCE_DIR}/src/renderer/*.cpp"
    "${thorvg_SOURCE_DIR}/src/renderer/gpu_engine/*.cpp"
    "${thorvg_SOURCE_DIR}/src/renderer/gpu_engine/gl/*.cpp"
    "${thorvg_SOURCE_DIR}/src/loaders/*.cpp"
    "${thorvg_SOURCE_DIR}/src/loaders/raw/*.cpp"
    "${thorvg_SOURCE_DIR}/src/loaders/sfnt/*.cpp"
    "${thorvg_SOURCE_DIR}/src/loaders/svg/*.cpp")

add_library(thorvg STATIC ${THORVG_SOURCES})
target_include_directories(thorvg PUBLIC
    "${thorvg_SOURCE_DIR}/inc"
    "${thorvg_SOURCE_DIR}/src/common"
    "${thorvg_SOURCE_DIR}/src/renderer"
    "${thorvg_SOURCE_DIR}/src/renderer/gpu_engine"
    "${thorvg_SOURCE_DIR}/src/renderer/gpu_engine/gl"
    "${thorvg_SOURCE_DIR}/src/loaders/raw"
    "${thorvg_SOURCE_DIR}/src/loaders/sfnt"
    "${thorvg_SOURCE_DIR}/src/loaders/svg"
    "${CMAKE_CURRENT_BINARY_DIR}")
target_compile_definitions(thorvg PRIVATE
    THORVG_GL_ENGINE_SUPPORT=1
    THORVG_GL_RASTER_SUPPORT=1
    THORVG_GL_TARGET_GLES=1
    THORVG_SVG_LOADER_SUPPORT=1
    THORVG_SFNT_LOADER_SUPPORT=1
    THORVG_TTF_LOADER_SUPPORT=1
    THORVG_OTF_LOADER_SUPPORT=1
    THORVG_FILE_IO_SUPPORT=1)

set(THORVG_CONFIG_H "${CMAKE_CURRENT_BINARY_DIR}/config.h")
file(WRITE "${THORVG_CONFIG_H}" "#pragma once\n")
file(APPEND "${THORVG_CONFIG_H}" "#define THORVG_VERSION_STRING \"1.1.0\"\n")
file(APPEND "${THORVG_CONFIG_H}" "#define THORVG_GL_ENGINE_SUPPORT 1\n")
file(APPEND "${THORVG_CONFIG_H}" "#define THORVG_GL_RASTER_SUPPORT 1\n")
file(APPEND "${THORVG_CONFIG_H}" "#define THORVG_GL_TARGET_GLES 1\n")
file(APPEND "${THORVG_CONFIG_H}" "#define THORVG_SVG_LOADER_SUPPORT 1\n")
file(APPEND "${THORVG_CONFIG_H}" "#define THORVG_SFNT_LOADER_SUPPORT 1\n")
file(APPEND "${THORVG_CONFIG_H}" "#define THORVG_TTF_LOADER_SUPPORT 1\n")
file(APPEND "${THORVG_CONFIG_H}" "#define THORVG_OTF_LOADER_SUPPORT 1\n")
file(APPEND "${THORVG_CONFIG_H}" "#define THORVG_FILE_IO_SUPPORT 1\n")
