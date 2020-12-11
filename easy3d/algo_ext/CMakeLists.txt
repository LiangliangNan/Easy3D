cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})


set(${PROJECT_NAME}_HEADERS
        duplicate_faces.h
        surfacer.h
        self_intersection.h
        )

set(${PROJECT_NAME}_SOURCES
        duplicate_faces.cpp
        surfacer.cpp
        self_intersection.cpp
        )


add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "easy3d")

target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_THIRD_PARTY})
target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_glog_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} core)

include(../../cmake/UseCGAL.cmake)


