set(module algo_ext)
set(dependencies easy3d_core)

set(${module}_headers
        overlapping_faces.h
        surfacer.h
        self_intersection.h
        )

set(${module}_sources
        overlapping_faces.cpp
        surfacer.cpp
        self_intersection.cpp
        )

add_module(${module} "${${module}_headers}" "${${module}_sources}" "${dependencies}")
target_link_libraries(easy3d_${module} PRIVATE ${CGAL_LIBRARIES} ${CGAL_3RD_PARTY_LIBRARIES})
install_module(${module})