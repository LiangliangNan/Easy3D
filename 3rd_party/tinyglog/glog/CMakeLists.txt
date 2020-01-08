# Android should use 3.1 or above
cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})

# option to build either a shared or a static library. default: shared
option (BUILD_SHARED_MINIGLOG "Build shared miniglog library" ON)

if (BUILD_SHARED_MINIGLOG)
    add_library(${PROJECT_NAME} SHARED
        logging.h
        logging.cc
        )
else (BUILD_SHARED_MINIGLOG)
    add_library(${PROJECT_NAME} STATIC
        logging.h
        logging.cc
        )
endif (BUILD_SHARED)


set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "tinyglog")

set(TINYGLOG_INCLUDE_DIR  ${CMAKE_CURRENT_LIST_DIR}/..)
target_include_directories(${PROJECT_NAME} PUBLIC ${TINYGLOG_INCLUDE_DIR})
