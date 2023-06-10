set(module algo)
set(private_dependencies 3rd_poisson 3rd_ransac 3rd_kdtree 3rd_triangle 3rd_tetgen 3rd_polypartition 3rd_glutess 3rd_opcode)
set(public_dependencies easy3d::core easy3d::util easy3d::kdtree)

set(${module}_headers
        collider.h
        delaunay.h
        delaunay_2d.h
        delaunay_3d.h
        extrusion.h
        surface_mesh_geometry.h
        gaussian_noise.h
        point_cloud_normals.h
        point_cloud_poisson_reconstruction.h
        point_cloud_ransac.h
        point_cloud_simplification.h
        polygon_partition.h
        surface_mesh_components.h
        surface_mesh_curvature.h
        surface_mesh_enumerator.h
        surface_mesh_factory.h
        surface_mesh_fairing.h
        surface_mesh_features.h
        surface_mesh_geodesic.h
        surface_mesh_hole_filling.h
        surface_mesh_parameterization.h
        surface_mesh_polygonization.h
        surface_mesh_remeshing.h
        surface_mesh_sampler.h
        surface_mesh_simplification.h
        surface_mesh_smoothing.h
        surface_mesh_stitching.h
        surface_mesh_subdivision.h
        surface_mesh_tetrahedralization.h
        surface_mesh_topology.h
        surface_mesh_triangulation.h
        tessellator.h
        text_mesher.h
        triangle_mesh_kdtree.h
        )

set(${module}_sources
        collider.cpp
        delaunay.cpp
        delaunay_2d.cpp
        delaunay_3d.cpp
        extrusion.cpp
        surface_mesh_geometry.cpp
        gaussian_noise.cpp
        point_cloud_normals.cpp
        point_cloud_poisson_reconstruction.cpp
        point_cloud_ransac.cpp
        point_cloud_simplification.cpp
        polygon_partition.cpp
        surface_mesh_components.cpp
        surface_mesh_curvature.cpp
        surface_mesh_enumerator.cpp
        surface_mesh_factory.cpp
        surface_mesh_fairing.cpp
        surface_mesh_features.cpp
        surface_mesh_geodesic.cpp
        surface_mesh_hole_filling.cpp
        surface_mesh_parameterization.cpp
        surface_mesh_polygonization.cpp
        surface_mesh_remeshing.cpp
        surface_mesh_sampler.cpp
        surface_mesh_simplification.cpp
        surface_mesh_smoothing.cpp
        surface_mesh_stitching.cpp
        surface_mesh_subdivision.cpp
        surface_mesh_tetrahedralization.cpp
        surface_mesh_topology.cpp
        surface_mesh_triangulation.cpp
        tessellator.cpp
        text_mesher.cpp
        triangle_mesh_kdtree.cpp
        )

add_module(${module} "${${module}_headers}" "${${module}_sources}" "${private_dependencies}" "${public_dependencies}")
target_include_directories(easy3d_${module} PRIVATE ${Easy3D_THIRD_PARTY}/eigen ${Easy3D_THIRD_PARTY}/ransac)

# It's "Boost", not "BOOST" or "boost". Case matters.
find_package(Boost)
if (Boost_FOUND)
    target_include_directories(easy3d_${module} PRIVATE ${Boost_INCLUDE_DIRS})
    target_compile_definitions(easy3d_${module} PRIVATE HAS_BOOST)
endif ()

install_module(${module})