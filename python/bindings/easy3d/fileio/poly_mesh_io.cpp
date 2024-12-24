#include <easy3d/core/poly_mesh.h>
#include <easy3d/fileio/poly_mesh_io.h>

#include <memory>

#include <pybind11/pybind11.h>


#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_fileio_poly_mesh_io(pybind11::module_& m)
{
	{ // easy3d::PolyMeshIO file:easy3d/fileio/poly_mesh_io.h line:43
		pybind11::class_<easy3d::PolyMeshIO, std::shared_ptr<easy3d::PolyMeshIO>> cl(m, "PolyMeshIO", "Implementation of file input/output operations for PolyMesh.\n \n\n\n     ");
		cl.def( pybind11::init( [](){ return new easy3d::PolyMeshIO(); } ) );
		cl.def_static("load", (class easy3d::PolyMesh * (*)(const std::string &)) &easy3d::PolyMeshIO::load, "Reads a polyhedral mesh from a file.\n \n\n File extension determines file format (now only '*.plm' format is supported).\n \n\n The file name.\n \n\n The pointer of the polyhedral mesh (nullptr if failed).\n\nC++: easy3d::PolyMeshIO::load(const std::string &) --> class easy3d::PolyMesh *", pybind11::return_value_policy::automatic, pybind11::arg("file_name"));
		cl.def_static("save", (bool (*)(const std::string &, const class easy3d::PolyMesh *)) &easy3d::PolyMeshIO::save, "Saves a polyhedral mesh to a file.\n \n\n File extension determines file format (now only '*.plm' format is supported).\n \n\n The file name.\n \n\n The polyhedral mesh.\n \n\n The status of the operation\n      \n\n true if succeeded\n      \n\n false if failed\n\nC++: easy3d::PolyMeshIO::save(const std::string &, const class easy3d::PolyMesh *) --> bool", pybind11::arg("file_name"), pybind11::arg("mesh"));
	}

    // easy3d::io::load_pm(const std::string &, class easy3d::PolyMesh *) file:easy3d/fileio/poly_mesh_io.h line:71
    m.def("load_pm", (bool (*)(const std::string &, class easy3d::PolyMesh *)) &easy3d::io::load_pm, "Reads a polyhedral mesh from a  format file. This is the built-in binary format of Easy3D.\n\nC++: easy3d::io::load_pm(const std::string &, class easy3d::PolyMesh *) --> bool", pybind11::arg("file_name"), pybind11::arg("mesh"));

    // easy3d::io::save_pm(const std::string &, const class easy3d::PolyMesh *) file:easy3d/fileio/poly_mesh_io.h line:73
    m.def("save_pm", (bool (*)(const std::string &, const class easy3d::PolyMesh *)) &easy3d::io::save_pm, "Saves a polyhedral mesh to a  format file. This is the built-in binary format of Easy3D.\n\nC++: easy3d::io::save_pm(const std::string &, const class easy3d::PolyMesh *) --> bool", pybind11::arg("file_name"), pybind11::arg("mesh"));

    // easy3d::io::load_plm(const std::string &, class easy3d::PolyMesh *) file:easy3d/fileio/poly_mesh_io.h line:76
    m.def("load_plm", (bool (*)(const std::string &, class easy3d::PolyMesh *)) &easy3d::io::load_plm, "Reads a polyhedral mesh from a  format file. This is the built-in ASCII format of Easy3D.\n\nC++: easy3d::io::load_plm(const std::string &, class easy3d::PolyMesh *) --> bool", pybind11::arg("file_name"), pybind11::arg("mesh"));

    // easy3d::io::save_plm(const std::string &, const class easy3d::PolyMesh *) file:easy3d/fileio/poly_mesh_io.h line:78
    m.def("save_plm", (bool (*)(const std::string &, const class easy3d::PolyMesh *)) &easy3d::io::save_plm, "Saves a polyhedral mesh to a  format file. This is the built-in ASCII format of Easy3D.\n\nC++: easy3d::io::save_plm(const std::string &, const class easy3d::PolyMesh *) --> bool", pybind11::arg("file_name"), pybind11::arg("mesh"));

    // easy3d::io::load_mesh(const std::string &, class easy3d::PolyMesh *) file:easy3d/fileio/poly_mesh_io.h line:81
    m.def("load_mesh", (bool (*)(const std::string &, class easy3d::PolyMesh *)) &easy3d::io::load_mesh, "Reads a polyhedral mesh from a  format file. This ASCII format is supported by Tetgen and Medit.\n\nC++: easy3d::io::load_mesh(const std::string &, class easy3d::PolyMesh *) --> bool", pybind11::arg("file_name"), pybind11::arg("mesh"));

    // easy3d::io::save_mesh(const std::string &, const class easy3d::PolyMesh *) file:easy3d/fileio/poly_mesh_io.h line:83
    m.def("save_mesh", (bool (*)(const std::string &, const class easy3d::PolyMesh *)) &easy3d::io::save_mesh, "Saves a polyhedral mesh to a  format file. This ASCII format is supported by Tetgen and Medit.\n\nC++: easy3d::io::save_mesh(const std::string &, const class easy3d::PolyMesh *) --> bool", pybind11::arg("file_name"), pybind11::arg("mesh"));

}
