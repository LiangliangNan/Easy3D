add_subdirectory(algo)
add_subdirectory(core)
add_subdirectory(fileio)
add_subdirectory(kdtree)
add_subdirectory(util)
add_subdirectory(viewer)

if(EASY3D_ENABLE_CGAL)
    find_package(CGAL)
    if(CGAL_FOUND)
        add_subdirectory(algo_ext)  # experimental; not stable
    else()
        message(WARNING "'algo_ext' requires CGAL but CGAL was not found")
    endif()
endif()

