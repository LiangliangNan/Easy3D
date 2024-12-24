#include <easy3d/algo/surface_mesh_triangulation.h>
#include <easy3d/core/surface_mesh.h>
#include <memory>

#include <pybind11/pybind11.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_algo_surface_mesh_triangulation(pybind11::module_& m)
{
	{ // easy3d::SurfaceMeshTriangulation file:easy3d/algo/surface_mesh_triangulation.h line:31
		pybind11::class_<easy3d::SurfaceMeshTriangulation, std::shared_ptr<easy3d::SurfaceMeshTriangulation>> cl(m, "SurfaceMeshTriangulation", "Triangulate a polygonal mesh into a pure triangle mesh.\n \n\n\n \n Triangulate n-gons into n-2 triangles. Find the triangulation that minimizes the sum of squared triangle\n      areas. See the following paper for more details:\n          - Peter Liepa. Filling holes in meshes. SGP, 2003.");
		cl.def( pybind11::init<class easy3d::SurfaceMesh *>(), pybind11::arg("mesh") );

		cl.def( pybind11::init( [](easy3d::SurfaceMeshTriangulation const &o){ return new easy3d::SurfaceMeshTriangulation(o); } ) );

		pybind11::enum_<easy3d::SurfaceMeshTriangulation::Objective>(cl, "Objective", pybind11::arithmetic(), "triangulation objective: find the triangulation that minimizes the\n sum of squared triangle areas, or the one that maximizes the minimum\n angle.")
			.value("MIN_AREA", easy3d::SurfaceMeshTriangulation::MIN_AREA)
			.value("MAX_ANGLE", easy3d::SurfaceMeshTriangulation::MAX_ANGLE)
			.export_values();

		cl.def("triangulate", [](easy3d::SurfaceMeshTriangulation &o) -> void { return o.triangulate(); }, "");
		cl.def("triangulate", (void (easy3d::SurfaceMeshTriangulation::*)(enum easy3d::SurfaceMeshTriangulation::Objective)) &easy3d::SurfaceMeshTriangulation::triangulate, "triangulate all faces\n\nC++: easy3d::SurfaceMeshTriangulation::triangulate(enum easy3d::SurfaceMeshTriangulation::Objective) --> void", pybind11::arg("obj"));
		cl.def("triangulate", [](easy3d::SurfaceMeshTriangulation &o, struct easy3d::SurfaceMesh::Face const & a0) -> void { return o.triangulate(a0); }, "", pybind11::arg("f"));
		cl.def("triangulate", (void (easy3d::SurfaceMeshTriangulation::*)(struct easy3d::SurfaceMesh::Face, enum easy3d::SurfaceMeshTriangulation::Objective)) &easy3d::SurfaceMeshTriangulation::triangulate, "triangulate a particular face f\n\nC++: easy3d::SurfaceMeshTriangulation::triangulate(struct easy3d::SurfaceMesh::Face, enum easy3d::SurfaceMeshTriangulation::Objective) --> void", pybind11::arg("f"), pybind11::arg("obj"));
		cl.def("assign", (class easy3d::SurfaceMeshTriangulation & (easy3d::SurfaceMeshTriangulation::*)(const class easy3d::SurfaceMeshTriangulation &)) &easy3d::SurfaceMeshTriangulation::operator=, "C++: easy3d::SurfaceMeshTriangulation::operator=(const class easy3d::SurfaceMeshTriangulation &) --> class easy3d::SurfaceMeshTriangulation &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}

}
