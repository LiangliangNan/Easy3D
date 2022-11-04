set(module viewer)
set(private_dependencies 3rd_glew 3rd_glfw)
set(public_dependencies easy3d::core easy3d::renderer easy3d::util easy3d::fileio)

set(${module}_headers
        viewer.h
        comp_viewer.h
        )

set(${module}_sources
        viewer.cpp
        comp_viewer.cpp
        )

add_module(${module} "${${module}_headers}" "${${module}_sources}" "${private_dependencies}" "${public_dependencies}")
install_module(${module})