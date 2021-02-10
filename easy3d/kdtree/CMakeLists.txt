cmake_minimum_required(VERSION 3.1)

get_filename_component(MODULE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(PROJECT_NAME "easy3d_${MODULE_NAME}")
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

target_include_directories(${PROJECT_NAME} PUBLIC ${EASY3D_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} PUBLIC easy3d_core 3rd_kdtree)


# Alias target (recommended by policy CMP0028) and it looks nicer
message(STATUS "Adding target: easy3d::${MODULE_NAME} (${PROJECT_NAME})")
add_library(easy3d::${MODULE_NAME} ALIAS ${PROJECT_NAME})
# Export as easy3d::${MODULE_NAME}
set_property(TARGET ${PROJECT_NAME} PROPERTY EXPORT_NAME easy3d::${MODULE_NAME})