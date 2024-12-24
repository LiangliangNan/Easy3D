#include <easy3d/algo/surface_mesh_enumerator.h>
#include <easy3d/core/surface_mesh.h>

#include <memory>

#include <pybind11/pybind11.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_algo_surface_mesh_enumerator(pybind11::module_& m)
{
	{ // easy3d::SurfaceMeshEnumerator file:easy3d/algo/surface_mesh_enumerator.h line:24
		pybind11::class_<easy3d::SurfaceMeshEnumerator, std::shared_ptr<easy3d::SurfaceMeshEnumerator>> cl(m, "SurfaceMeshEnumerator", "Enumerates connected components for a surface mesh.\n \n\n\n     ");
		cl.def( pybind11::init( [](){ return new easy3d::SurfaceMeshEnumerator(); } ) );
		cl.def_static("enumerate_connected_components", (int (*)(class easy3d::SurfaceMesh *, class easy3d::SurfaceMesh::VertexProperty<int>)) &easy3d::SurfaceMeshEnumerator::enumerate_connected_components, "Enumerates the connected components of a surface mesh from its vertices.\n \n\n The input mesh.\n \n\n The vertex property storing the result.\n \n\n The number of connected components.\n\nC++: easy3d::SurfaceMeshEnumerator::enumerate_connected_components(class easy3d::SurfaceMesh *, class easy3d::SurfaceMesh::VertexProperty<int>) --> int", pybind11::arg("mesh"), pybind11::arg("id"));
		cl.def_static("enumerate_connected_components", (int (*)(class easy3d::SurfaceMesh *, class easy3d::SurfaceMesh::FaceProperty<int>)) &easy3d::SurfaceMeshEnumerator::enumerate_connected_components, "Enumerates the connected components of a surface mesh from its faces.\n \n\n The input mesh.\n \n\n The face property storing the result.\n \n\n The number of connected components.\n\nC++: easy3d::SurfaceMeshEnumerator::enumerate_connected_components(class easy3d::SurfaceMesh *, class easy3d::SurfaceMesh::FaceProperty<int>) --> int", pybind11::arg("mesh"), pybind11::arg("id"));
		cl.def_static("propagate_connected_component", (void (*)(class easy3d::SurfaceMesh *, class easy3d::SurfaceMesh::VertexProperty<int>, struct easy3d::SurfaceMesh::Vertex, int)) &easy3d::SurfaceMeshEnumerator::propagate_connected_component, "Propagates a connected component of a surface mesh from the seed vertex.\n \n\n The input mesh.\n \n\n The vertex property storing the result.\n \n\n The seed vertex from which the connected component will be propagated.\n \n\n The index of the connected component to be propagated.\n \n\n The vertex property id must be initialize with \"-1\".\n\nC++: easy3d::SurfaceMeshEnumerator::propagate_connected_component(class easy3d::SurfaceMesh *, class easy3d::SurfaceMesh::VertexProperty<int>, struct easy3d::SurfaceMesh::Vertex, int) --> void", pybind11::arg("mesh"), pybind11::arg("id"), pybind11::arg("seed"), pybind11::arg("cur_id"));
		cl.def_static("propagate_connected_component", (void (*)(class easy3d::SurfaceMesh *, class easy3d::SurfaceMesh::FaceProperty<int>, struct easy3d::SurfaceMesh::Face, int)) &easy3d::SurfaceMeshEnumerator::propagate_connected_component, "Propagates a connected component of a surface mesh from the seed face.\n \n\n The input mesh.\n \n\n The face property storing the result.\n \n\n The seed face from which the connected component will be propagated.\n \n\n The index of the connected component to be propagated.\n \n\n The face property id must be initialize with \"-1\".\n\nC++: easy3d::SurfaceMeshEnumerator::propagate_connected_component(class easy3d::SurfaceMesh *, class easy3d::SurfaceMesh::FaceProperty<int>, struct easy3d::SurfaceMesh::Face, int) --> void", pybind11::arg("mesh"), pybind11::arg("id"), pybind11::arg("seed"), pybind11::arg("cur_id"));
		cl.def_static("enumerate_planar_components", [](class easy3d::SurfaceMesh * a0, class easy3d::SurfaceMesh::FaceProperty<int> const & a1) -> int { return easy3d::SurfaceMeshEnumerator::enumerate_planar_components(a0, a1); }, "", pybind11::arg("mesh"), pybind11::arg("id"));
		cl.def_static("enumerate_planar_components", (int (*)(class easy3d::SurfaceMesh *, class easy3d::SurfaceMesh::FaceProperty<int>, float)) &easy3d::SurfaceMeshEnumerator::enumerate_planar_components, "Enumerates planar patches.\n \n\n The input mesh.\n \n\n The vertex property storing the result.\n \n\n Two faces sharing a common edge are considered coplanar if the dihedral angle is\n      smaller than  (in degrees).\n \n\n The number of connected components.\n\nC++: easy3d::SurfaceMeshEnumerator::enumerate_planar_components(class easy3d::SurfaceMesh *, class easy3d::SurfaceMesh::FaceProperty<int>, float) --> int", pybind11::arg("mesh"), pybind11::arg("id"), pybind11::arg("angle_threshold"));
		cl.def_static("propagate_planar_component", (void (*)(class easy3d::SurfaceMesh *, class easy3d::SurfaceMesh::FaceProperty<int>, struct easy3d::SurfaceMesh::Face, int, float)) &easy3d::SurfaceMeshEnumerator::propagate_planar_component, "Propagates a planar patch from the seed face.\n \n\n The input mesh.\n \n\n The face property storing the result.\n \n\n The seed face from which the planar patch will be propagated.\n \n\n The index of the connected component to be propagated.\n \n\n Two faces sharing a common edge are considered coplanar if the dihedral angle is\n      smaller than  (in degrees).\n \n\n The face normal property (with name  must exist and be up to date.\n \n\n The face property id must be initialize with \"-1\".\n\nC++: easy3d::SurfaceMeshEnumerator::propagate_planar_component(class easy3d::SurfaceMesh *, class easy3d::SurfaceMesh::FaceProperty<int>, struct easy3d::SurfaceMesh::Face, int, float) --> void", pybind11::arg("mesh"), pybind11::arg("id"), pybind11::arg("seed"), pybind11::arg("cur_id"), pybind11::arg("angle_threshold"));
	}

}
