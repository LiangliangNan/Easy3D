#include <easy3d/core/graph.h>
#include <easy3d/fileio/graph_io.h>

#include <memory>
#include <pybind11/pybind11.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_fileio_graph_io(pybind11::module_& m)
{
    { // easy3d::GraphIO file:easy3d/fileio/graph_io.h line:40
        pybind11::class_<easy3d::GraphIO, std::shared_ptr<easy3d::GraphIO>> cl(m, "GraphIO", "Implementation of file input/output operations for Graph (Currently only PLY format is supported).\n \n");
        cl.def( pybind11::init( [](){ return new easy3d::GraphIO(); } ) );
        // Liangliang: wrap the raw pointer in a std::shared_ptr to ensure proper memory management and shared ownership between Python and C++.
        cl.def_static(
                "load",
                [](const std::string& file_name) -> std::shared_ptr<easy3d::Graph> {
                    return std::shared_ptr<easy3d::Graph>(easy3d::GraphIO::load(file_name));
                },
                "Reads a graph from a file",
                pybind11::return_value_policy::automatic, // Or use "take_ownership"
                pybind11::arg("file_name")
        );
//        cl.def_static("load", (class easy3d::Graph * (*)(const std::string &)) &easy3d::GraphIO::load, "Reads a graph from file \n \n\n The pointer of the graph (nullptr if failed).\n \n\n File extension determines file format (currently only PLY format is supported).\n\nC++: easy3d::GraphIO::load(const std::string &) --> class easy3d::Graph *", pybind11::return_value_policy::automatic, pybind11::arg("file_name"));
        cl.def_static("save", (bool (*)(const std::string &, const class easy3d::Graph *)) &easy3d::GraphIO::save, "Saves  to file \n \n\n File extension determines file format (currently only PLY format is supported).\n \n\n The status of the operation\n      \n\n true if succeeded\n      \n\n false if failed\n\nC++: easy3d::GraphIO::save(const std::string &, const class easy3d::Graph *) --> bool", pybind11::arg("file_name"), pybind11::arg("graph"));
    }
    
	// easy3d::io::load_ply(const std::string &, class easy3d::Graph *) file:easy3d/fileio/graph_io.h line:68
	m.def("load_ply", (bool (*)(const std::string &, class easy3d::Graph *)) &easy3d::io::load_ply, "Loads  from a PLY file \n \n\n The status of the operation\n      \n\n true if succeeded\n      \n\n false if failed\n\nC++: easy3d::io::load_ply(const std::string &, class easy3d::Graph *) --> bool", pybind11::arg("file_name"), pybind11::arg("graph"));

	// easy3d::io::save_ply(const std::string &, const class easy3d::Graph *, bool) file:easy3d/fileio/graph_io.h line:78
	m.def("save_ply", [](const std::string & a0, const class easy3d::Graph * a1) -> bool { return easy3d::io::save_ply(a0, a1); }, "", pybind11::arg("file_name"), pybind11::arg("graph"));
	m.def("save_ply", (bool (*)(const std::string &, const class easy3d::Graph *, bool)) &easy3d::io::save_ply, "Saves  into a PLY file \n \n\n The full path of the file.\n \n\n The graph.\n \n\n  for binary format, otherwise ASCII format.\n \n\n The status of the operation\n      \n\n  if succeeded\n      \n\n  if failed\n\nC++: easy3d::io::save_ply(const std::string &, const class easy3d::Graph *, bool) --> bool", pybind11::arg("file_name"), pybind11::arg("graph"), pybind11::arg("binary"));

}
