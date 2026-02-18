#pragma once

/* Version */
#define THORVG_VERSION_STRING "1.0.1"

/* Threads */
#define THORVG_THREAD_SUPPORT 1

/* Engines */
#define THORVG_GL_RASTER_SUPPORT 1

/* Partial Rendering */
#define THORVG_PARTIAL_RENDER_SUPPORT 1

/* Loaders */
#define THORVG_SVG_LOADER_SUPPORT 1

/* SIMD (none for emscripten) */
#define THORVG_AVX_VECTOR_SUPPORT 0
#define THORVG_NEON_VECTOR_SUPPORT 0

/* Bindings */
#define THORVG_CAPI_BINDING_SUPPORT 0

/* Extra */
#define THORVG_LOTTIE_EXPRESSIONS_SUPPORT 0
#define THORVG_OPENMP_SUPPORT 1

/* Misc */
#define WIN32_LEAN_AND_MEAN 1
#define TEST_DIR ""
