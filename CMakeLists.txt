cmake_minimum_required(VERSION 3.19)
project(pd4web)

# ╭──────────────────────────────────────╮
# │                LibPd                 │
# ╰──────────────────────────────────────╯
cmake_policy(SET CMP0077 NEW)
set(PD_EXTRA off)
add_subdirectory(Sources/libpd)
target_compile_options(libpd_static PUBLIC -w)

# ╭──────────────────────────────────────╮
# │          Pd4Web Executable           │
# ╰──────────────────────────────────────╯
add_executable(pd4web Sources/pd4web.cpp Sources/externals.cpp)
target_include_directories(pd4web PRIVATE Sources/libpd/libpd_wrapper
                                          Sources/libpd/pure-data/src)
target_link_libraries(pd4web PRIVATE embind libpd_static)
target_compile_options(pd4web PRIVATE -sUSE_PTHREADS=1 -sWASM_WORKERS=1)
target_link_options(
  pd4web
  PRIVATE
  -sPTHREAD_POOL_SIZE=navigator.hardwareConcurrency
  -sMODULARIZE=1
  -sEXPORT_NAME='Pd4WebModule'
  -sAUDIO_WORKLET=1
  -sUSE_PTHREADS=1
  -sWASM_WORKERS=1
  -sWASM=1)
