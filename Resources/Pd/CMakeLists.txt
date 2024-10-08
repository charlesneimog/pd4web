cmake_minimum_required(VERSION 3.20)
project(pd4web)

set(PDCMAKE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/../../Sources/Libraries/pd.cmake/"
    CACHE PATH "Path to pd.cmake")
include(${PDCMAKE_DIR}/pd.cmake)

add_subdirectory(cpp-httplib EXCLUDE_FROM_ALL)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

pd_add_external(pd4web pd4web.cpp)
pd_add_datafile(pd4web pd4web-help.pd)
pd_add_datafile(pd4web Projects/Project1.pd)
target_link_libraries(pd4web httplib)

set_target_properties(
  pd4web
  PROPERTIES CXX_STANDARD 17
             CXX_STANDARD_REQUIRED ON
             CXX_EXTENSIONS OFF)
