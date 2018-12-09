cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})


set(${PROJECT_NAME}_HEADERS
    ./box.h
    ./IO.h
    ./line.h
    ./mat.h
    ./math_types.h
    ./plane.h
    ./point_cloud.h
    ./properties.h
    ./quat.h
    ./rect.h
    ./segment.h
    ./surface_mesh.h
    ./types.h
    ./vec.h
    )

set(${PROJECT_NAME}_SOURCES
    ./IO.cpp
    ./IO_obj.cpp
    ./IO_off.cpp
    ./IO_poly.cpp
    ./IO_stl.cpp
    ./point_cloud.cpp
    ./surface_mesh.cpp
    )


add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES
                      FOLDER "Easy3D")

target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_INCLUDE_DIR})
