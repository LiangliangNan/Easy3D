cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})


set(${PROJECT_NAME}_HEADERS
    kdtree_search.h
    kdtree_search_ann.h
    kdtree_search_eth.h
    kdtree_search_flann.h
    kdtree_search_nanoflann.h
    )

set(${PROJECT_NAME}_SOURCES
    kdtree_search.cpp
    kdtree_search_ann.cpp
    kdtree_search_eth.cpp
    kdtree_search_flann.cpp
    kdtree_search_nanoflann.cpp
    )

	
add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_HEADERS} )

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "easy3d")

target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} PUBLIC core kd_tree)
