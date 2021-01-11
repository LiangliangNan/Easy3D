cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
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
        manifold_builder.h
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
        spline_interpolation.h
        surface_mesh.h
        poly_mesh.h
        polygon.h
        types.h
        vec.h
        )

set(${PROJECT_NAME}_SOURCES
        graph.cpp
        manifold_builder.cpp
        model.cpp
        point_cloud.cpp
        surface_mesh.cpp
        poly_mesh.cpp
        )


add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "easy3d")

target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} util)


