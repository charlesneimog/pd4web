project(pd4web)

add_executable(pd4web ${CMAKE_CURRENT_SOURCE_DIR}/pd4web.cpp
                      ${CMAKE_CURRENT_SOURCE_DIR}/externals.cpp)
if(NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR}/pure-data)
    find_package(Git REQUIRED)
    set(LIBPD_TAG "0.55-2")
    execute_process(
        COMMAND ${GIT_EXECUTABLE} clone https://github.com/pure-data/pure-data.git --branch
                ${LIBPD_TAG} --recursive
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        RESULT_VARIABLE GIT_RESULT)
    if(NOT GIT_RESULT EQUAL "0")
        message(FATAL_ERROR "Failed to clone puredata repository.")
    endif()
endif()

set(PD_SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/pure-data/src)
include(${CMAKE_CURRENT_SOURCE_DIR}/Libraries/libpd.cmake)
target_include_directories(pd4web PRIVATE ${PD_SOURCE_DIR})
target_link_libraries(pd4web PRIVATE embind libpd)
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

set_target_properties(
    pd4web PROPERTIES LINK_FLAGS "--preload-file \"${CMAKE_CURRENT_SOURCE_DIR}/main.pd@/index.pd\"")

add_custom_command(
    TARGET pd4web
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/pd4web.threads.js"
            "$<TARGET_FILE_DIR:pd4web>/pd4web.threads.js"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/pd4web.midi.js"
            "$<TARGET_FILE_DIR:pd4web>/pd4web.midi.js"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/index.html"
            "$<TARGET_FILE_DIR:pd4web>/index.html"
    COMMAND ${CMAKE_COMMAND} -E copy "${CMAKE_CURRENT_SOURCE_DIR}/pd4web.style.css"
            "$<TARGET_FILE_DIR:pd4web>/pd4web.style.css"
    COMMENT "Copying extra files to build output directory")

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/main_pd_dependency
    COMMAND ${CMAKE_COMMAND} -E copy_if_different "${CMAKE_CURRENT_SOURCE_DIR}/main.pd"
            "${CMAKE_CURRENT_BINARY_DIR}/main_pd_dependency"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/main.pd"
    COMMENT "Trigger rebuild when main.pd changes")

add_custom_target(main_pd_trigger ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/main_pd_dependency)

add_dependencies(pd4web main_pd_trigger)
