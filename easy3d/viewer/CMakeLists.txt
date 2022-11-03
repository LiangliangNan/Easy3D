set(module viewer)
set(dependencies 3rd_glew 3rd_glfw easy3d::core easy3d::renderer easy3d::util easy3d::fileio easy3d::algo)

set(${module}_headers
        viewer.h
        comp_viewer.h
        )

set(${module}_sources
        viewer.cpp
        comp_viewer.cpp
        )

add_module(${module} "${${module}_headers}" "${${module}_sources}" "${dependencies}")
install_module(${module})