cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})

add_library(${PROJECT_NAME} STATIC fontstash.h fontstash.cpp)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "3rd_party")

set(FONTSTASH_INCLUDE_DIR ${EASY3D_THIRD_PARTY}/glog/src ${EASY3D_INCLUDE_DIR}/3rd_party/glog)
target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_INCLUDE_DIR} ${FONTSTASH_INCLUDE_DIR})