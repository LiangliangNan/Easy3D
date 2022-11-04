set(module kdtree)
set(private_dependencies 3rd_kdtree)
set(public_dependencies easy3d::core)

set(${module}_headers
        kdtree_search.h
        kdtree_search_ann.h
        kdtree_search_eth.h
        kdtree_search_flann.h
        kdtree_search_nanoflann.h
        )

set(${module}_sources
        kdtree_search.cpp
        kdtree_search_ann.cpp
        kdtree_search_eth.cpp
        kdtree_search_flann.cpp
        kdtree_search_nanoflann.cpp
        )

add_module(${module} "${${module}_headers}" "${${module}_sources}" "${private_dependencies}" "${public_dependencies}")
install_module(${module})