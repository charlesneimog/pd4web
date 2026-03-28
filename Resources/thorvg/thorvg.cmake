# ────────────────────────────────
# Individual Options (all OFF by default)
option(THORVG_THREADS "Enable multi-threading" OFF)
option(THORVG_SW_RASTER_SUPPORT "Enable SW raster engine" OFF)
option(THORVG_GL_RASTER_SUPPORT "Enable GL raster engine" OFF)
option(THORVG_WG_RASTER_SUPPORT "Enable WG raster engine" OFF)
option(THORVG_PARTIAL_RENDER_SUPPORT "Enable partial rendering" OFF)
option(THORVG_LOTTIE2GIF_TOOL "Enable lottie2gif tool" OFF)
option(THORVG_SVG2PNG_TOOL "Enable svg2png tool" OFF)
option(THORVG_SVG_LOADER_SUPPORT "Enable SVG loader" OFF)
option(THORVG_PNG_LOADER_SUPPORT "Enable PNG loader" OFF)
option(THORVG_JPG_LOADER_SUPPORT "Enable JPG loader" OFF)
option(THORVG_TTF_LOADER_SUPPORT "Enable TTF loader" OFF)
option(THORVG_WEBP_LOADER_SUPPORT "Enable WEBP loader" OFF)
option(THORVG_GIF_SAVER_SUPPORT "Enable GIF saver" OFF)
option(THORVG_LOG_ENABLED "Enable logging" OFF)
option(THORVG_FILE_IO_SUPPORT "Enable File I/O" OFF)
option(THORVG_AVX_VECTOR_SUPPORT "Enable AVX SIMD" OFF)
option(THORVG_NEON_VECTOR_SUPPORT "Enable NEON SIMD" OFF)
option(THORVG_CAPI_BINDING_SUPPORT "Enable C API bindings" OFF)
option(THORVG_LOTTIE_EXPRESSIONS_SUPPORT "Enable Lottie expressions" OFF)
option(THORVG_OPENMP_SUPPORT "Enable OpenMP" OFF)
option(THORVG_GL_TARGET_GLES "Use OpenGL ES" OFF)

# ────────────────────────────────
# Configure config.h
set(CONFIG_H "${CMAKE_CURRENT_BINARY_DIR}/config.h")
file(WRITE "${CONFIG_H}" "// Automatically generated config.h\n#pragma once\n\n")

# Version
file(APPEND "${CONFIG_H}" "#define THORVG_VERSION_STRING \"1.0.0\"\n")

# Apply options individually
if(THORVG_THREADS)
    file(APPEND "${CONFIG_H}" "#define THORVG_THREAD_SUPPORT 1\n")
endif()

if(THORVG_SW_RASTER_SUPPORT)
    file(APPEND "${CONFIG_H}" "#define THORVG_SW_RASTER_SUPPORT 1\n")
endif()

if(THORVG_GL_RASTER_SUPPORT)
    file(APPEND "${CONFIG_H}" "#define THORVG_GL_RASTER_SUPPORT 1\n")
endif()

if(THORVG_WG_RASTER_SUPPORT)
    file(APPEND "${CONFIG_H}" "#define THORVG_WG_RASTER_SUPPORT 1\n")
endif()

if(THORVG_PARTIAL_RENDER_SUPPORT)
    file(APPEND "${CONFIG_H}" "#define THORVG_PARTIAL_RENDER_SUPPORT 1\n")
endif()

if(THORVG_LOTTIE2GIF_TOOL)
    file(APPEND "${CONFIG_H}" "#define THORVG_LOTTIE2GIF_TOOL 1\n")
endif()

if(THORVG_SVG2PNG_TOOL)
    file(APPEND "${CONFIG_H}" "#define THORVG_SVG2PNG_TOOL 1\n")
endif()

if(THORVG_SVG_LOADER_SUPPORT)
    file(APPEND "${CONFIG_H}" "#define THORVG_SVG_LOADER_SUPPORT 1\n")
endif()

if(THORVG_PNG_LOADER_SUPPORT)
    file(APPEND "${CONFIG_H}" "#define THORVG_PNG_LOADER_SUPPORT 1\n")
endif()

if(THORVG_JPG_LOADER_SUPPORT)
    file(APPEND "${CONFIG_H}" "#define THORVG_JPG_LOADER_SUPPORT 1\n")
endif()

if(THORVG_TTF_LOADER_SUPPORT)
    file(APPEND "${CONFIG_H}" "#define THORVG_TTF_LOADER_SUPPORT 1\n")
endif()

if(THORVG_WEBP_LOADER_SUPPORT)
    file(APPEND "${CONFIG_H}" "#define THORVG_WEBP_LOADER_SUPPORT 1\n")
endif()

if(THORVG_GIF_SAVER_SUPPORT)
    file(APPEND "${CONFIG_H}" "#define THORVG_GIF_SAVER_SUPPORT 1\n")
endif()

if(THORVG_LOG_ENABLED)
    file(APPEND "${CONFIG_H}" "#define THORVG_LOG_ENABLED 1\n")
endif()

if(THORVG_FILE_IO_SUPPORT)
    file(APPEND "${CONFIG_H}" "#define THORVG_FILE_IO_SUPPORT 1\n")
endif()

if(THORVG_AVX_VECTOR_SUPPORT)
    file(APPEND "${CONFIG_H}" "#define THORVG_AVX_VECTOR_SUPPORT 1\n")
endif()

if(THORVG_NEON_VECTOR_SUPPORT)
    file(APPEND "${CONFIG_H}" "#define THORVG_NEON_VECTOR_SUPPORT 1\n")
endif()

if(THORVG_CAPI_BINDING_SUPPORT)
    file(APPEND "${CONFIG_H}" "#define THORVG_CAPI_BINDING_SUPPORT 1\n")
endif()

if(THORVG_LOTTIE_EXPRESSIONS_SUPPORT)
    file(APPEND "${CONFIG_H}" "#define THORVG_LOTTIE_EXPRESSIONS_SUPPORT 1\n")
endif()

if(THORVG_OPENMP_SUPPORT)
    file(APPEND "${CONFIG_H}" "#define THORVG_OPENMP_SUPPORT 1\n")
endif()

if(THORVG_GL_TARGET_GLES)
    file(APPEND "${CONFIG_H}" "#define THORVG_GL_TARGET_GLES 1\n")
endif()

# Misc
file(APPEND "${CONFIG_H}" "#define WIN32_LEAN_AND_MEAN 1\n")

# ────────────────────────────────
# Include generated config in target
target_include_directories(thorvg PUBLIC "${CMAKE_CURRENT_BINARY_DIR}")
