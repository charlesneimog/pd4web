cmake_minimum_required(VERSION 3.25)
project(pd4web)

# Set the path to pd.cmake
set(PDCMAKE_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/../../Sources/Libraries/pd.cmake/"
    CACHE PATH "Path to pd.cmake")

include(${PDCMAKE_DIR}/pd.cmake)

# Add cpp-httplib as a subdirectory, but exclude it from the default build
add_subdirectory(cpp-httplib EXCLUDE_FROM_ALL)

# Set C++ standard to C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Ensure static linking of the C++ runtime library

# Define the main target
pd_add_external(pd4web pd4web.cpp LINK_LIBRARIES httplib)

if(MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MT")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /MT")

  # Adicionar as opções de compilação para os alvos específicos
  foreach(target pd4web)
    set_target_properties(${target} PROPERTIES COMPILE_FLAGS "/MT")
  endforeach()
endif()

# Add data files associated with the pd4web target
pd_add_datafile(pd4web pd4web-help.pd)

file(GLOB PROJECT_FILES "Projects/*.pd")

foreach(file ${PROJECT_FILES})
  get_filename_component(file_name ${file} NAME)
  pd_add_datafile(pd4web "Projects/${file_name}")
endforeach(file ${PROJECT_FILES})
  
pd_add_datafile(pd4web README.deken.pd)

set_target_properties(
  pd4web
  PROPERTIES CXX_STANDARD 20
             CXX_STANDARD_REQUIRED ON
             CXX_EXTENSIONS OFF)