cmake_minimum_required(VERSION 3.25)
project(earplug~)
set(LIB_DIR ${PD4WEB_EXTERNAL_DIR}/${PROJECT_NAME})

pd_add_external(earplug~ "${LIB_DIR}/earplug~.c")
pd_add_datafile(earplug~ "${LIB_DIR}/earplug_data.txt")

