cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})


add_library(${PROJECT_NAME} STATIC tetgen.cxx predicates.cxx)

#Set properties on a target. 
#We use this here to set -DTETLIBRARY for when compiling the
#library
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS TETLIBRARY)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "3rd_party")
