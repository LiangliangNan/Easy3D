#include <easy3d/core/surface_mesh.h>
#include <easy3d/fileio/surface_mesh_io.h>

#include <memory>

#include <pybind11/pybind11.h>


#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_fileio_surface_mesh_io(pybind11::module_ &m)
{
	{ // easy3d::SurfaceMeshIO file:easy3d/fileio/surface_mesh_io.h line:43
		pybind11::class_<easy3d::SurfaceMeshIO, std::shared_ptr<easy3d::SurfaceMeshIO>> cl(m, "SurfaceMeshIO", "Implementation of file input/output operations for SurfaceMesh.\n \n\n\n     ");
		cl.def( pybind11::init( [](){ return new easy3d::SurfaceMeshIO(); } ) );
        // Liangliang: wrap the raw pointer in a std::shared_ptr to ensure proper memory management and shared ownership between Python and C++.
        cl.def_static(
                "load",
                [](const std::string& file_name) -> std::shared_ptr<easy3d::SurfaceMesh> {
                    return std::shared_ptr<easy3d::SurfaceMesh>(easy3d::SurfaceMeshIO::load(file_name));
                },
                "Reads a surface mesh from a file",
                pybind11::return_value_policy::automatic, // Or use "take_ownership"
                pybind11::arg("file_name")
        );
//        cl.def_static("load", (class easy3d::SurfaceMesh * (*)(const std::string &)) &easy3d::SurfaceMeshIO::load, "Reads a surface mesh from a file.\n \n\n File extension determines file format (ply, obj, off, stl, poly) and type (i.e. binary or ASCII).\n \n\n The file name.\n \n\n The pointer of the surface mesh (nullptr if failed).\n\nC++: easy3d::SurfaceMeshIO::load(const std::string &) --> class easy3d::SurfaceMesh *", pybind11::return_value_policy::automatic, pybind11::arg("file_name"));
		cl.def_static("save", (bool (*)(const std::string &, const class easy3d::SurfaceMesh *)) &easy3d::SurfaceMeshIO::save, "Saves a surface mesh to a file.\n \n\n File extension determines file format (ply, obj, off, stl, poly) and type (i.e. binary or ASCII).\n \n\n The file name.\n \n\n The surface mesh.\n \n\n The status of the operation\n      \n\n true if succeeded\n      \n\n false if failed\n\nC++: easy3d::SurfaceMeshIO::save(const std::string &, const class easy3d::SurfaceMesh *) --> bool", pybind11::arg("file_name"), pybind11::arg("mesh"));
	}

    {
		// easy3d::io::load_sm(const std::string &, class easy3d::SurfaceMesh *) file:easy3d/fileio/surface_mesh_io.h line:70
		m.def("load_sm", (bool (*)(const std::string &, class easy3d::SurfaceMesh *)) &easy3d::io::load_sm, "Reads a surface mesh from a  format file.\n\nC++: easy3d::io::load_sm(const std::string &, class easy3d::SurfaceMesh *) --> bool", pybind11::arg("file_name"), pybind11::arg("mesh"));

		// easy3d::io::save_sm(const std::string &, const class easy3d::SurfaceMesh *) file:easy3d/fileio/surface_mesh_io.h line:72
		m.def("save_sm", (bool (*)(const std::string &, const class easy3d::SurfaceMesh *)) &easy3d::io::save_sm, "Saves a surface mesh to a  format file.\n\nC++: easy3d::io::save_sm(const std::string &, const class easy3d::SurfaceMesh *) --> bool", pybind11::arg("file_name"), pybind11::arg("mesh"));

		// easy3d::io::load_ply(const std::string &, class easy3d::SurfaceMesh *) file:easy3d/fileio/surface_mesh_io.h line:75
		m.def("load_ply", (bool (*)(const std::string &, class easy3d::SurfaceMesh *)) &easy3d::io::load_ply, "Reads a surface mesh from a  format file.\n\nC++: easy3d::io::load_ply(const std::string &, class easy3d::SurfaceMesh *) --> bool", pybind11::arg("file_name"), pybind11::arg("mesh"));

		// easy3d::io::save_ply(const std::string &, const class easy3d::SurfaceMesh *, bool) file:easy3d/fileio/surface_mesh_io.h line:77
		m.def("save_ply", [](const std::string & a0, const class easy3d::SurfaceMesh * a1) -> bool { return easy3d::io::save_ply(a0, a1); }, "", pybind11::arg("file_name"), pybind11::arg("mesh"));
		m.def("save_ply", (bool (*)(const std::string &, const class easy3d::SurfaceMesh *, bool)) &easy3d::io::save_ply, "Saves a surface mesh to a  format file.\n\nC++: easy3d::io::save_ply(const std::string &, const class easy3d::SurfaceMesh *, bool) --> bool", pybind11::arg("file_name"), pybind11::arg("mesh"), pybind11::arg("binary"));

		// easy3d::io::load_off(const std::string &, class easy3d::SurfaceMesh *) file:easy3d/fileio/surface_mesh_io.h line:80
		m.def("load_off", (bool (*)(const std::string &, class easy3d::SurfaceMesh *)) &easy3d::io::load_off, "Reads a surface mesh from a  format file.\n\nC++: easy3d::io::load_off(const std::string &, class easy3d::SurfaceMesh *) --> bool", pybind11::arg("file_name"), pybind11::arg("mesh"));

		// easy3d::io::save_off(const std::string &, const class easy3d::SurfaceMesh *) file:easy3d/fileio/surface_mesh_io.h line:82
		m.def("save_off", (bool (*)(const std::string &, const class easy3d::SurfaceMesh *)) &easy3d::io::save_off, "Saves a surface mesh to a  format file.\n\nC++: easy3d::io::save_off(const std::string &, const class easy3d::SurfaceMesh *) --> bool", pybind11::arg("file_name"), pybind11::arg("mesh"));

		// easy3d::io::load_obj(const std::string &, class easy3d::SurfaceMesh *) file:easy3d/fileio/surface_mesh_io.h line:85
		m.def("load_obj", (bool (*)(const std::string &, class easy3d::SurfaceMesh *)) &easy3d::io::load_obj, "Reads a surface mesh from a  format file.\n\nC++: easy3d::io::load_obj(const std::string &, class easy3d::SurfaceMesh *) --> bool", pybind11::arg("file_name"), pybind11::arg("mesh"));

		// easy3d::io::save_obj(const std::string &, const class easy3d::SurfaceMesh *) file:easy3d/fileio/surface_mesh_io.h line:87
		m.def("save_obj", (bool (*)(const std::string &, const class easy3d::SurfaceMesh *)) &easy3d::io::save_obj, "Saves a surface mesh to a  format file.\n\nC++: easy3d::io::save_obj(const std::string &, const class easy3d::SurfaceMesh *) --> bool", pybind11::arg("file_name"), pybind11::arg("mesh"));

		// easy3d::io::load_stl(const std::string &, class easy3d::SurfaceMesh *) file:easy3d/fileio/surface_mesh_io.h line:90
		m.def("load_stl", (bool (*)(const std::string &, class easy3d::SurfaceMesh *)) &easy3d::io::load_stl, "Reads a surface mesh from a  format file.\n\nC++: easy3d::io::load_stl(const std::string &, class easy3d::SurfaceMesh *) --> bool", pybind11::arg("file_name"), pybind11::arg("mesh"));

		// easy3d::io::save_stl(const std::string &, const class easy3d::SurfaceMesh *) file:easy3d/fileio/surface_mesh_io.h line:92
		m.def("save_stl", (bool (*)(const std::string &, const class easy3d::SurfaceMesh *)) &easy3d::io::save_stl, "Saves a surface mesh to a  format file.\n\nC++: easy3d::io::save_stl(const std::string &, const class easy3d::SurfaceMesh *) --> bool", pybind11::arg("file_name"), pybind11::arg("mesh"));

		// easy3d::io::load_trilist(const std::string &, class easy3d::SurfaceMesh *) file:easy3d/fileio/surface_mesh_io.h line:96
		m.def("load_trilist", (bool (*)(const std::string &, class easy3d::SurfaceMesh *)) &easy3d::io::load_trilist, "Reads a set of triangles (each line has coordinates of 3 points)\n Mainly used for easily saving triangles for debugging.\n\nC++: easy3d::io::load_trilist(const std::string &, class easy3d::SurfaceMesh *) --> bool", pybind11::arg("file_name"), pybind11::arg("mesh"));

		// easy3d::io::load_geojson(const std::string &, class easy3d::SurfaceMesh *) file:easy3d/fileio/surface_mesh_io.h line:100
		m.def("load_geojson", (bool (*)(const std::string &, class easy3d::SurfaceMesh *)) &easy3d::io::load_geojson, "Reads Geojson format files. 2D polygons are stored as faces of a 3D surface mesh\n (all Z-coordinates are set to 0).\n\nC++: easy3d::io::load_geojson(const std::string &, class easy3d::SurfaceMesh *) --> bool", pybind11::arg("file_name"), pybind11::arg("mesh"));
	}
}
