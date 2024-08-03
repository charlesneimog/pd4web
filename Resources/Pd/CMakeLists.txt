cmake_minimum_required(VERSION 3.20)
project(pd4web)

set(PDCMAKE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/../../Sources/Libraries/pd.cmake/"
    CACHE PATH "Path to pd.cmake")
include(${PDCMAKE_DIR}/pd.cmake)

pd_add_external(pd4web pd4web.cpp)
pd_add_datafile(pd4web pd4web-help.pd)
pd_add_datafile(pd4web dist/pd-pd4web)
