#include <easy3d/algo/triangle_mesh_kdtree.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/vec.h>

#include <memory>

#include <pybind11/pybind11.h>


#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_algo_triangle_mesh_kdtree(pybind11::module_& m)
{
	{ // easy3d::TriangleMeshKdTree file:easy3d/algo/triangle_mesh_kdtree.h line:24
		pybind11::class_<easy3d::TriangleMeshKdTree, std::shared_ptr<easy3d::TriangleMeshKdTree>> cl(m, "TriangleMeshKdTree", "A k-d tree for triangular surface meshes.\n \n");
		cl.def( pybind11::init( [](const class easy3d::SurfaceMesh * a0){ return new easy3d::TriangleMeshKdTree(a0); } ), "doc" , pybind11::arg("mesh"));
		cl.def( pybind11::init( [](const class easy3d::SurfaceMesh * a0, unsigned int const & a1){ return new easy3d::TriangleMeshKdTree(a0, a1); } ), "doc" , pybind11::arg("mesh"), pybind11::arg("max_faces"));
		cl.def( pybind11::init<const class easy3d::SurfaceMesh *, unsigned int, unsigned int>(), pybind11::arg("mesh"), pybind11::arg("max_faces"), pybind11::arg("max_depth") );

		cl.def("nearest", (struct easy3d::TriangleMeshKdTree::NearestNeighbor (easy3d::TriangleMeshKdTree::*)(const class easy3d::Vec<3, float> &) const) &easy3d::TriangleMeshKdTree::nearest, "Return handle of the nearest neighbor\n\nC++: easy3d::TriangleMeshKdTree::nearest(const class easy3d::Vec<3, float> &) const --> struct easy3d::TriangleMeshKdTree::NearestNeighbor", pybind11::arg("p"));

		{ // easy3d::TriangleMeshKdTree::NearestNeighbor file:easy3d/algo/triangle_mesh_kdtree.h line:32
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::TriangleMeshKdTree::NearestNeighbor, std::shared_ptr<easy3d::TriangleMeshKdTree::NearestNeighbor>> cl(enclosing_class, "NearestNeighbor", "nearest neighbor information");
			cl.def( pybind11::init( [](){ return new easy3d::TriangleMeshKdTree::NearestNeighbor(); } ) );
			cl.def_readwrite("dist", &easy3d::TriangleMeshKdTree::NearestNeighbor::dist);
			cl.def_readwrite("face", &easy3d::TriangleMeshKdTree::NearestNeighbor::face);
			cl.def_readwrite("nearest", &easy3d::TriangleMeshKdTree::NearestNeighbor::nearest);
			cl.def_readwrite("tests", &easy3d::TriangleMeshKdTree::NearestNeighbor::tests);
		}

	}

}
