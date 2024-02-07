set(module algo_ext)
set(private_dependencies)
set(public_dependencies easy3d::core)

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

add_module(${module} "${${module}_headers}" "${${module}_sources}" "${private_dependencies}" "${public_dependencies}")
target_link_libraries(easy3d_${module} PRIVATE ${CGAL_LIBRARIES} ${CGAL_3RD_PARTY_LIBRARIES})
target_compile_definitions(easy3d_${module} PUBLIC HAS_CGAL)
if (MINGW)
#     set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-O3") # solves "file too big" and "too many sections (81167)" with MinGW
     target_compile_options(easy3d_${module} PRIVATE -O3)   # solves "file too big" and "too many sections (81167)" with MinGW
endif ()
install_module(${module})