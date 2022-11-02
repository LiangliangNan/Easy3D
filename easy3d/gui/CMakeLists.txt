set(module gui)
set(dependencies easy3d_core easy3d_util easy3d_renderer)

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

add_module(${module} "${${module}_headers}" "${${module}_sources}" "${dependencies}")
install_module(${module})