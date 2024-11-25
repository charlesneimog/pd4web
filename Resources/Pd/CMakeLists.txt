cmake_minimum_required(VERSION 3.25)
project(pd4web)

# Set the path to pd.cmake
set(PDCMAKE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/../../Sources/Libraries/pd.cmake/"
    CACHE PATH "Path to pd.cmake")

include(${PDCMAKE_DIR}/pd.cmake)
add_subdirectory(cpp-httplib EXCLUDE_FROM_ALL)

# Define the main target
pd_add_external(pd4web pd4web.cpp) #LINK_LIBRARIES httplib)
target_include_directories(pd4web PRIVATE cpp-httplib)
set_target_properties(
  pd4web
  PROPERTIES CXX_STANDARD 20
             CXX_STANDARD_REQUIRED ON
             CXX_EXTENSIONS OFF)

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MT")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MT")
  foreach(target pd4web)
    set_target_properties(${target} PROPERTIES COMPILE_FLAGS "/MT")
  endforeach()
endif()

# Add data files associated with the pd4web target
pd_add_datafile(pd4web pd4web-help.pd)
pd_add_datafile(pd4web README.deken.pd)
pd_add_datafile(pd4web pd4web.cpp)

file(GLOB PROJECT_FILES "Projects/*.pd")
set(COUNT 1)

foreach(file ${PROJECT_FILES})
  get_filename_component(file_name ${file} NAME)
  string(REGEX REPLACE "\\.[^.]*$" "" project_folder ${file_name})
  pd_add_datafile(pd4web "Projects/${file_name}" DESTINATION "Examples/${COUNT}-${project_folder}")
  math(EXPR COUNT "${COUNT}+1")
endforeach(file ${PROJECT_FILES})