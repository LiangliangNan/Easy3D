cmake_minimum_required(VERSION 3.1)

get_filename_component(MODULE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(PROJECT_NAME "easy3d_${MODULE_NAME}")
project(${PROJECT_NAME})


set(${PROJECT_NAME}_HEADERS
        image_io.h
        graph_io.h
        ply_reader_writer.h
        point_cloud_io.h
        point_cloud_io_ptx.h
        point_cloud_io_vg.h
        surface_mesh_io.h
        poly_mesh_io.h
        resources.h
        )

set(${PROJECT_NAME}_SOURCES
        image_io.cpp
        graph_io.cpp
        graph_io_ply.cpp
        ply_reader_writer.cpp
        point_cloud_io.cpp
        point_cloud_io_bin.cpp
        point_cloud_io_las.cpp
        point_cloud_io_ply.cpp
        point_cloud_io_ptx.cpp
        point_cloud_io_vg.cpp
        point_cloud_io_xyz.cpp
        surface_mesh_io.cpp
        surface_mesh_io_obj.cpp
        surface_mesh_io_off.cpp
        surface_mesh_io_ply.cpp
        surface_mesh_io_sm.cpp
        surface_mesh_io_stl.cpp
        poly_mesh_io.cpp
        poly_mesh_io_mesh.cpp
        poly_mesh_io_plm.cpp
        poly_mesh_io_pm.cpp
        resources.cpp
        )


add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "easy3d")

# The resources directory
target_compile_definitions(
        ${PROJECT_NAME}
        PRIVATE
        "EASY3D_RESOURCES_DIR=\"${EASY3D_ROOT}/resources\""
)

target_include_directories(${PROJECT_NAME} PUBLIC ${EASY3D_INCLUDE_DIR})

set(LASTOOLS_INCLUDE_DIR ${EASY3D_THIRD_PARTY}/lastools/LASzip/src ${EASY3D_THIRD_PARTY}/lastools/LASlib/inc)
target_include_directories(${PROJECT_NAME} PRIVATE ${LASTOOLS_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} PUBLIC easy3d_core easy3d_util 3rd_lastools 3rd_rply)

if (MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_DEPRECATE)
endif ()


# Alias target (recommended by policy CMP0028) and it looks nicer
message(STATUS "Adding target: easy3d::${MODULE_NAME} (${PROJECT_NAME})")
add_library(easy3d::${MODULE_NAME} ALIAS ${PROJECT_NAME})
# Export as easy3d::${MODULE_NAME}
set_property(TARGET ${PROJECT_NAME} PROPERTY EXPORT_NAME easy3d::${MODULE_NAME})