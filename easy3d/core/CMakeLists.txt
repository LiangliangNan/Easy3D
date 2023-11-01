set(module core)
set(private_dependencies)
set(public_dependencies easy3d::util)

set(${module}_headers
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
        matrix_algo.h
        model.h
        oriented_line.h
        plane.h
        point_cloud.h
        principal_axes.h
        property.h
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
        )

set(${module}_sources
        graph.cpp
        surface_mesh_builder.cpp
        matrix_algo.cpp
        model.cpp
        point_cloud.cpp
        surface_mesh.cpp
        poly_mesh.cpp
        )

add_module(${module} "${${module}_headers}" "${${module}_sources}" "${private_dependencies}" "${public_dependencies}")
install_module(${module})