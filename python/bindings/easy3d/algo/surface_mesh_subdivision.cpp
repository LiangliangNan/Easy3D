#include <easy3d/algo/surface_mesh_subdivision.h>
#include <easy3d/core/surface_mesh.h>

#include <memory>

#include <pybind11/pybind11.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif


void bind_easy3d_algo_surface_mesh_subdivision(pybind11::module_& m)
{
	{ // easy3d::SurfaceMeshSubdivision file:easy3d/algo/surface_mesh_subdivision.h line:22
		pybind11::class_<easy3d::SurfaceMeshSubdivision, std::shared_ptr<easy3d::SurfaceMeshSubdivision>> cl(m, "SurfaceMeshSubdivision", "SurfaceMeshSubdivision implement several well-known subdivision algorithms.\n \n");
		cl.def( pybind11::init( [](){ return new easy3d::SurfaceMeshSubdivision(); } ) );
		cl.def_static("catmull_clark", (bool (*)(class easy3d::SurfaceMesh *)) &easy3d::SurfaceMeshSubdivision::catmull_clark, "The Catmull-Clark subdivision. \n\nC++: easy3d::SurfaceMeshSubdivision::catmull_clark(class easy3d::SurfaceMesh *) --> bool", pybind11::arg("mesh"));
		cl.def_static("loop", (bool (*)(class easy3d::SurfaceMesh *)) &easy3d::SurfaceMeshSubdivision::loop, "The Loop subdivision. \n\nC++: easy3d::SurfaceMeshSubdivision::loop(class easy3d::SurfaceMesh *) --> bool", pybind11::arg("mesh"));
		cl.def_static("sqrt3", (bool (*)(class easy3d::SurfaceMesh *)) &easy3d::SurfaceMeshSubdivision::sqrt3, "The sqrt3 subdivision. \n\nC++: easy3d::SurfaceMeshSubdivision::sqrt3(class easy3d::SurfaceMesh *) --> bool", pybind11::arg("mesh"));
	}

}
