set(module gui)
set(private_dependencies)
set(public_dependencies easy3d::core easy3d::util easy3d::renderer)

set(${module}_headers
        picker.h
        picker_model.h
        picker_point_cloud.h
        picker_surface_mesh.h
        )

set(${module}_sources
        picker.cpp
        picker_model.cpp
        picker_point_cloud.cpp
        picker_surface_mesh.cpp
        )

add_module(${module} "${${module}_headers}" "${${module}_sources}" "${private_dependencies}" "${public_dependencies}")
install_module(${module})