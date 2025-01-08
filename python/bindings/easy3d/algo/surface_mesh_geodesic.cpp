#include <easy3d/algo/surface_mesh_geodesic.h>
#include <easy3d/core/surface_mesh.h>

#include <memory>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>


#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_algo_surface_mesh_geodesic(pybind11::module_& m)
{
	{ // easy3d::SurfaceMeshGeodesic file:easy3d/algo/surface_mesh_geodesic.h line:31
		pybind11::class_<easy3d::SurfaceMeshGeodesic, std::shared_ptr<easy3d::SurfaceMeshGeodesic>> cl(m, "SurfaceMeshGeodesic", "This class computes geodesic distance from a set of seed vertices.\n \n\n\n \n The method works by a Dykstra-like breadth first traversal from the seed vertices, implemented by a\n heap structure. See the following paper for more details:\n  - Kimmel and Sethian. Computing geodesic paths on manifolds. Proceedings of the National Academy of Sciences,\n    95(15):8431-8435, 1998.");
		cl.def( pybind11::init( [](class easy3d::SurfaceMesh * a0){ return new easy3d::SurfaceMeshGeodesic(a0); } ), "doc" , pybind11::arg("mesh"));
		cl.def( pybind11::init<class easy3d::SurfaceMesh *, bool>(), pybind11::arg("mesh"), pybind11::arg("use_virtual_edges") );

		cl.def( pybind11::init( [](easy3d::SurfaceMeshGeodesic const &o){ return new easy3d::SurfaceMeshGeodesic(o); } ) );
		cl.def("compute", [](easy3d::SurfaceMeshGeodesic &o, const class std::vector<struct easy3d::SurfaceMesh::Vertex> & a0) -> unsigned int { return o.compute(a0); }, "", pybind11::arg("seed"));
		cl.def("compute", [](easy3d::SurfaceMeshGeodesic &o, const class std::vector<struct easy3d::SurfaceMesh::Vertex> & a0, float const & a1) -> unsigned int { return o.compute(a0, a1); }, "", pybind11::arg("seed"), pybind11::arg("max_dist"));
		cl.def("compute", [](easy3d::SurfaceMeshGeodesic &o, const class std::vector<struct easy3d::SurfaceMesh::Vertex> & a0, float const & a1, unsigned int const & a2) -> unsigned int { return o.compute(a0, a1, a2); }, "", pybind11::arg("seed"), pybind11::arg("max_dist"), pybind11::arg("max_num"));
		cl.def("compute", (unsigned int (easy3d::SurfaceMeshGeodesic::*)(const class std::vector<struct easy3d::SurfaceMesh::Vertex> &, float, unsigned int, class std::vector<struct easy3d::SurfaceMesh::Vertex> *)) &easy3d::SurfaceMeshGeodesic::compute, "Compute geodesic distances from specified seed points.\n \n\n The results are store as SurfaceMesh::VertexProperty<float> with a name \"v:geodesic:distance\".\n \n\n The vector of seed vertices.\n \n\n The maximum distance up to which to compute the\n geodesic distances.\n \n\n The maximum number of neighbors up to which to\n compute the geodesic distances.\n \n\n The vector of neighbor vertices.\n \n\n The number of neighbors that have been found.\n\nC++: easy3d::SurfaceMeshGeodesic::compute(const class std::vector<struct easy3d::SurfaceMesh::Vertex> &, float, unsigned int, class std::vector<struct easy3d::SurfaceMesh::Vertex> *) --> unsigned int", pybind11::arg("seed"), pybind11::arg("max_dist"), pybind11::arg("max_num"), pybind11::arg("neighbors"));
		cl.def("__call__", (float (easy3d::SurfaceMeshGeodesic::*)(struct easy3d::SurfaceMesh::Vertex) const) &easy3d::SurfaceMeshGeodesic::operator(), "Access the computed geodesic distance.\n \n\n The vertex for which to return the geodesic distance.\n \n\n The geodesic distance of vertex \n \n\n The function compute() has been called before.\n \n\n The vertex  needs to be a valid vertex handle of the mesh\n used during construction.\n\nC++: easy3d::SurfaceMeshGeodesic::operator()(struct easy3d::SurfaceMesh::Vertex) const --> float", pybind11::arg("v"));
		cl.def("distance_to_texture_coordinates", (void (easy3d::SurfaceMeshGeodesic::*)()) &easy3d::SurfaceMeshGeodesic::distance_to_texture_coordinates, "Use the normalized distances as texture coordinates\n \n\n Stores the normalized distances in a vertex property of type\n TexCoord named \"v:tex\". Re-uses any existing vertex property of the\n same type and name.\n\nC++: easy3d::SurfaceMeshGeodesic::distance_to_texture_coordinates() --> void");
		cl.def("assign", (class easy3d::SurfaceMeshGeodesic & (easy3d::SurfaceMeshGeodesic::*)(const class easy3d::SurfaceMeshGeodesic &)) &easy3d::SurfaceMeshGeodesic::operator=, "C++: easy3d::SurfaceMeshGeodesic::operator=(const class easy3d::SurfaceMeshGeodesic &) --> class easy3d::SurfaceMeshGeodesic &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}

}
