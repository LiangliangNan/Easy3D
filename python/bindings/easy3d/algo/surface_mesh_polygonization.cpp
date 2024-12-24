#include <easy3d/algo/surface_mesh_polygonization.h>
#include <easy3d/core/surface_mesh.h>

#include <memory>

#include <pybind11/pybind11.h>


#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_algo_surface_mesh_polygonization(pybind11::module_& m)
{
	{ // easy3d::SurfaceMeshPolygonization file:easy3d/algo/surface_mesh_polygonization.h line:40
		pybind11::class_<easy3d::SurfaceMeshPolygonization, std::shared_ptr<easy3d::SurfaceMeshPolygonization>> cl(m, "SurfaceMeshPolygonization", "Merge connected coplanar faces into a general polygon face.\n \n\n\n \n Support faces with holes (i.e., polygons with multiple contours)");
		cl.def( pybind11::init( [](){ return new easy3d::SurfaceMeshPolygonization(); } ) );
		cl.def("apply", [](easy3d::SurfaceMeshPolygonization &o, class easy3d::SurfaceMesh * a0) -> void { return o.apply(a0); }, "", pybind11::arg("mesh"));
		cl.def("apply", (void (easy3d::SurfaceMeshPolygonization::*)(class easy3d::SurfaceMesh *, float)) &easy3d::SurfaceMeshPolygonization::apply, "Merges connected coplanar faces into a general polygon face.\n \n\n The result is a general polygonal mesh.\n \n\n The input surface mesh. Upon return, the mesh will be modified.\n \n\n Two faces sharing a common edge are considered coplanar if the dihedral angle is\n      smaller than  (in degrees).\n \n\n The current implementation doesn't support polygon faces with holes.\n\nC++: easy3d::SurfaceMeshPolygonization::apply(class easy3d::SurfaceMesh *, float) --> void", pybind11::arg("mesh"), pybind11::arg("angle_threshold"));
		cl.def("merge_colinear_edges", [](easy3d::SurfaceMeshPolygonization &o, class easy3d::SurfaceMesh * a0) -> void { return o.merge_colinear_edges(a0); }, "", pybind11::arg("mesh"));
		cl.def("merge_colinear_edges", (void (easy3d::SurfaceMeshPolygonization::*)(class easy3d::SurfaceMesh *, float)) &easy3d::SurfaceMeshPolygonization::merge_colinear_edges, "Removes 2-degree vertices.\n \n\n For every 2-degree vertex, if the angle between its two incident edges is smaller than a threshold,\n merge the two incident edges by removing this vertex.\n \n\n The input surface mesh. Upon return, the mesh will be modified.\n \n\n Two edges sharing the same vertex are considered colinear if their angle is smaller\n      than  (in degrees).\n\nC++: easy3d::SurfaceMeshPolygonization::merge_colinear_edges(class easy3d::SurfaceMesh *, float) --> void", pybind11::arg("mesh"), pybind11::arg("angle_threshold"));
	}

}
