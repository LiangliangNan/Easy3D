add_subdirectory(algo)
add_subdirectory(core)
add_subdirectory(fileio)
add_subdirectory(kdtree)
add_subdirectory(util)
add_subdirectory(viewer)
add_subdirectory(gui)

if (EASY3D_HAS_CGAL)
    add_subdirectory(algo_ext)  # experimental; not stable
endif ()

