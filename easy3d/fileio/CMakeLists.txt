cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})


set(${PROJECT_NAME}_HEADERS
    ply_reader_writer.h
    point_cloud_io.h
    point_cloud_io_ptx.h
#    point_cloud_io_vg.h
    surface_mesh_io.h
    )

set(${PROJECT_NAME}_SOURCES
    ply_reader_writer.cpp
    point_cloud_io.cpp
    point_cloud_io_bin.cpp
    point_cloud_io_las.cpp
    point_cloud_io_ply.cpp
    point_cloud_io_ptx.cpp
#    point_cloud_io_vg.cpp
    point_cloud_io_xyz.cpp
    surface_mesh_io.cpp
    surface_mesh_io_obj.cpp
    surface_mesh_io_off.cpp
    surface_mesh_io_ply.cpp
    surface_mesh_io_poly.cpp
    surface_mesh_io_stl.cpp
    )

	
add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_HEADERS} )

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Easy3D")

target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_glog_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_lastools_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_rply_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} core LAStools rply)

if (MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_DEPRECATE)
endif()
