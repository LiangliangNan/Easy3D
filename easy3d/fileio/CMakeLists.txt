set(module fileio)
set(private_dependencies 3rd_lastools 3rd_rply)
set(public_dependencies easy3d::core easy3d::util)

set(${module}_headers
        image_io.h
        graph_io.h
        ply_reader_writer.h
        point_cloud_io.h
        point_cloud_io_ptx.h
        point_cloud_io_vg.h
        surface_mesh_io.h
        poly_mesh_io.h
        translator.h
        )

set(${module}_sources
        image_io.cpp
        graph_io.cpp
        graph_io_ply.cpp
        ply_reader_writer.cpp
        point_cloud_io.cpp
        point_cloud_io_bin.cpp
        point_cloud_io_las.cpp
        point_cloud_io_ply.cpp
        point_cloud_io_ptx.cpp
        point_cloud_io_vg.cpp
        point_cloud_io_xyz.cpp
        surface_mesh_io.cpp
        surface_mesh_io_geojson.cpp
        surface_mesh_io_obj.cpp
        surface_mesh_io_off.cpp
        surface_mesh_io_ply.cpp
        surface_mesh_io_sm.cpp
        surface_mesh_io_stl.cpp
        poly_mesh_io.cpp
        poly_mesh_io_mesh.cpp
        poly_mesh_io_plm.cpp
        poly_mesh_io_pm.cpp
        translator.cpp
        )

		
add_module(${module} "${${module}_headers}" "${${module}_sources}" "${private_dependencies}" "${public_dependencies}")
set(LASTOOLS_INCLUDE_DIR ${Easy3D_THIRD_PARTY}/lastools/LASzip/src ${Easy3D_THIRD_PARTY}/lastools/LASlib/inc)
target_include_directories(easy3d_${module} PRIVATE ${LASTOOLS_INCLUDE_DIR})
install_module(${module})