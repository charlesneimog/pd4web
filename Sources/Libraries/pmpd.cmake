cmake_minimum_required(VERSION 3.25)

set(PDCMAKE_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}/Resources/pd.cmake
    CACHE PATH "Path to pd.cmake")
include(${PDCMAKE_DIR}/pd.cmake)

set(LIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Pd4Web/Externals/pmpd)

# ╭──────────────────────────────────────╮
# │           PMPD Compilation           │
# ╰──────────────────────────────────────╯

project(pmpd)
configure_file("${LIB_DIR}/pmpd_version.c.in" "${CMAKE_BINARY_DIR}/pmpd_version.c" @ONLY)

include_directories(${CMAKE_BINARY_DIR})
include_directories(${LIB_DIR}/)
set(ENABLE_TILDE_TARGET_WARNING off)

# Inline is not supported by emscripten, the sugestition is to use "inline static" Get the list of
# .c files
function(replaceline file line new_line)
    file(READ ${file} FILE_CONTENTS)
    string(REPLACE "${line}" "${new_line}" FILE_CONTENTS "${FILE_CONTENTS}")
    message(STATUS "Replacing line in ${file}")
    file(WRITE ${file} "${FILE_CONTENTS}")
endfunction()

set(FILES_TO_FIX "${LIB_DIR}/pmpd~.c" "${LIB_DIR}/pmpd2d~.c" "${LIB_DIR}/pmpd3d~.c")

replaceline(
    "${LIB_DIR}/pmpd~.c"
    "inline int validate_index(t_pmpd_tilde *x, int idx, t_int count, const char* type)"
    "inline static int validate_index(t_pmpd_tilde *x, int idx, t_int count, const char* type)")
replaceline(
    "${LIB_DIR}/pmpd~.c"
    "inline int validate_count(t_pmpd_tilde *x, t_int count, t_int count_max, const char* type)"
    "inline static int validate_count(t_pmpd_tilde *x, t_int count, t_int count_max, const char* type)"
)
replaceline(
    "${LIB_DIR}/pmpd2d~.c"
    "inline int validate_index(t_pmpd2d_tilde *x, int idx, t_int count, const char* type)"
    "inline static int validate_index(t_pmpd2d_tilde *x, int idx, t_int count, const char* type)")
replaceline(
    "${LIB_DIR}/pmpd2d~.c"
    "inline int validate_count(t_pmpd2d_tilde *x, t_int count, t_int count_max, const char* type)"
    "inline static int validate_count(t_pmpd2d_tilde *x, t_int count, t_int count_max, const char* type)"
)
replaceline(
    "${LIB_DIR}/pmpd3d~.c"
    "inline int validate_index(t_pmpd3d_tilde *x, int idx, t_int count, const char* type)"
    "inline static int validate_index(t_pmpd3d_tilde *x, int idx, t_int count, const char* type)")
replaceline(
    "${LIB_DIR}/pmpd3d~.c"
    "inline int validate_count(t_pmpd3d_tilde *x, t_int count, t_int count_max, const char* type)"
    "inline static int validate_count(t_pmpd3d_tilde *x, t_int count, t_int count_max, const char* type)"
)

set(PMPD_COMMON_FILES "${LIB_DIR}/pmpd_version.h;${CMAKE_BINARY_DIR}/pmpd_version.c")

pd_add_external(iAmbient2D "${LIB_DIR}/iAmbient2D.c" "${PMPD_COMMON_FILES}")
pd_add_external(iAmbient3D "${LIB_DIR}/iAmbient3D.c" "${PMPD_COMMON_FILES}")
pd_add_external(iCircle2D "${LIB_DIR}/iCircle2D.c" "${PMPD_COMMON_FILES}")
pd_add_external(iCircle3D "${LIB_DIR}/iCircle3D.c" "${PMPD_COMMON_FILES}")
pd_add_external(iCylinder3D "${LIB_DIR}/iCylinder3D.c" "${PMPD_COMMON_FILES}")
pd_add_external(iLine2D "${LIB_DIR}/iLine2D.c" "${PMPD_COMMON_FILES}")
pd_add_external(iPlane3D "${LIB_DIR}/iPlane3D.c" "${PMPD_COMMON_FILES}")
pd_add_external(iSeg2D "${LIB_DIR}/iSeg2D.c" "${PMPD_COMMON_FILES}")
pd_add_external(iSphere3D "${LIB_DIR}/iSphere3D.c" "${PMPD_COMMON_FILES}")
pd_add_external(link "${LIB_DIR}/link.c" "${PMPD_COMMON_FILES}")
pd_add_external(link2D "${LIB_DIR}/link2D.c" "${PMPD_COMMON_FILES}")
pd_add_external(link3D "${LIB_DIR}/link3D.c" "${PMPD_COMMON_FILES}")
pd_add_external(mass "${LIB_DIR}/mass.c" "${PMPD_COMMON_FILES}")
pd_add_external(mass2D "${LIB_DIR}/mass2D.c" "${PMPD_COMMON_FILES}")
pd_add_external(mass3D "${LIB_DIR}/mass3D.c" "${PMPD_COMMON_FILES}")
pd_add_external(pmpd2d "${LIB_DIR}/pmpd2d.c" "${PMPD_COMMON_FILES}")
pd_add_external(pmpd3d "${LIB_DIR}/pmpd3d.c" "${PMPD_COMMON_FILES}")
pd_add_external(pmpd_tilde "${LIB_DIR}/pmpd~.c" "${LIB_DIR}/pmpd~.c" "${PMPD_COMMON_FILES}")
pd_add_external(pmpd2d_tilde "${LIB_DIR}/pmpd2d~.c" "${PMPD_COMMON_FILES}")
pd_add_external(pmpd3d_tilde "${LIB_DIR}/pmpd3d~.c" "${PMPD_COMMON_FILES}")
pd_add_external(tCircle2D "${LIB_DIR}/tCircle2D.c" "${PMPD_COMMON_FILES}")
pd_add_external(tCircle3D "${LIB_DIR}/tCircle3D.c" "${PMPD_COMMON_FILES}")
pd_add_external(tCube3D "${LIB_DIR}/tCube3D.c" "${PMPD_COMMON_FILES}")
pd_add_external(tCylinder3D "${LIB_DIR}/tCylinder3D.c" "${PMPD_COMMON_FILES}")
pd_add_external(tLine2D "${LIB_DIR}/tLine2D.c" "${PMPD_COMMON_FILES}")
pd_add_external(tLink2D "${LIB_DIR}/tLink2D.c" "${PMPD_COMMON_FILES}")
pd_add_external(tLink3D "${LIB_DIR}/tLink3D.c" "${PMPD_COMMON_FILES}")
pd_add_external(tPlane3D "${LIB_DIR}/tPlane3D.c" "${PMPD_COMMON_FILES}")
pd_add_external(tSeg2D "${LIB_DIR}/tSeg2D.c" "${PMPD_COMMON_FILES}")
pd_add_external(tSphere3D "${LIB_DIR}/tSphere3D.c" "${PMPD_COMMON_FILES}")
pd_add_external(tSquare2D "${LIB_DIR}/tSquare2D.c" "${PMPD_COMMON_FILES}")

file(GLOB PMP_FILES "${LIB_DIR}/*.c")
list(APPEND PMP_FILES "${PMPD_COMMON_FILES}")
pd_add_external(pmpd ${PMP_FILES})

include(GenerateExportHeader)
generate_export_header(pmpd)
