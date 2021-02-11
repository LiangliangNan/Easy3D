cmake_minimum_required(VERSION 3.1)

get_filename_component(MODULE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(PROJECT_NAME "easy3d_${MODULE_NAME}")
project(${PROJECT_NAME})


set(${PROJECT_NAME}_HEADERS
        box.h
        constant.h
        curve.h
        eigen_solver.h
        graph.h
        hash.h
        heap.h
        line.h
        surface_mesh_builder.h
        mat.h
        matrix.h
        model.h
        oriented_line.h
        plane.h
        point_cloud.h
        principal_axes.h
        properties.h
        quat.h
        random.h
        rect.h
        segment.h
        signal.h
        spline_curve_fitting.h
        spline_curve_interpolation.h
        spline_interpolation.h
        surface_mesh.h
        poly_mesh.h
        polygon.h
        types.h
        vec.h
        version.h
        )

set(${PROJECT_NAME}_SOURCES
        graph.cpp
        surface_mesh_builder.cpp
        model.cpp
        point_cloud.cpp
        surface_mesh.cpp
        poly_mesh.cpp
        version.cpp
        )


add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "easy3d")

target_compile_definitions(
        ${PROJECT_NAME}
        PRIVATE
        "EASY3D_MAJOR_VERSION=${EASY3D_MAJOR_VERSION}"
        "EASY3D_MINOR_VERSION=${EASY3D_MINOR_VERSION}"
        "EASY3D_PATCH_VERSION=${EASY3D_PATCH_VERSION}"
        "EASY3D_VERSION=\"${EASY3D_VERSION}\""
)

target_include_directories(${PROJECT_NAME} PUBLIC ${EASY3D_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} PUBLIC easy3d_util)


# Alias target (recommended by policy CMP0028) and it looks nicer
message(STATUS "Adding target: easy3d::${MODULE_NAME} (${PROJECT_NAME})")
add_library(easy3d::${MODULE_NAME} ALIAS ${PROJECT_NAME})
# Export as easy3d::${MODULE_NAME}
set_property(TARGET ${PROJECT_NAME} PROPERTY EXPORT_NAME easy3d::${MODULE_NAME})
