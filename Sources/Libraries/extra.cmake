cmake_minimum_required(VERSION 3.25)
project(extra)
set(LIB_DIR ${PD4WEB_EXTERNAL_DIR}/../pure-data/extra)

add_definitions(-DPD)

set(BOB_FILE "${LIB_DIR}/bob~/bob~.c")
set(BONK_FILE "${LIB_DIR}/bonk~/bonk~.c")
set(SIGMUND_FILE "${LIB_DIR}/sigmund~/sigmund~.c")

if(EXISTS "${BOB_FILE}")
  pd_add_external(bob~ "${BOB_FILE}")
endif()

if(EXISTS "${BONK_FILE}")
  pd_add_external(bonk~ "${BONK_FILE}")
endif()

if(EXISTS "${SIGMUND_FILE}")
  pd_add_external(sigmund~ "${SIGMUND_FILE}")
endif()
