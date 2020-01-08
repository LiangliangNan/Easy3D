# Android should use 3.1 or above
cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})


if (EASY3D_BUILD_SHARED_GLOG_LIBRARY)
    add_library(${PROJECT_NAME} SHARED
        logging.h
        logging.cc
        )
else ()
    add_library(${PROJECT_NAME} STATIC
        logging.h
        logging.cc
        )
endif ()


set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "tinyglog")

set(GLOG_INCLUDE_DIR  ${CMAKE_CURRENT_LIST_DIR}/..)
target_include_directories(${PROJECT_NAME} PUBLIC ${GLOG_INCLUDE_DIR})
