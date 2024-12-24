#include <easy3d/algo/surface_mesh_curvature.h>
#include <easy3d/core/surface_mesh.h>

#include <memory>

#include <pybind11/pybind11.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_algo_surface_mesh_curvature(pybind11::module_& m)
{
	{ // easy3d::SurfaceMeshCurvature file:easy3d/algo/surface_mesh_curvature.h line:29
		pybind11::class_<easy3d::SurfaceMeshCurvature, std::shared_ptr<easy3d::SurfaceMeshCurvature>> cl(m, "SurfaceMeshCurvature", "Compute per-vertex curvatures, i.e., principle (min, max), mean, Gaussian.\n\n \n\n \n Curvature values for boundary vertices are interpolated from their interior neighbors.\n Curvature values can be smoothed. For more details, please refer to the following papers:\n    - Discrete Differential-Geometry Operators for Triangulated 2-Manifolds. Meyer et al. 2003.\n    - Restricted Delaunay triangulations and normal cycle. Cohen-Steiner and Morvan. 2003.");
		cl.def( pybind11::init<class easy3d::SurfaceMesh *>(), pybind11::arg("mesh") );

		cl.def("analyze", [](easy3d::SurfaceMeshCurvature &o) -> void { return o.analyze(); }, "");
		cl.def("analyze", (void (easy3d::SurfaceMeshCurvature::*)(unsigned int)) &easy3d::SurfaceMeshCurvature::analyze, "Computes principle curvature information for each vertex, optionally followed by some smoothing iterations\n of the curvature values. Upon finish, the principle curvatures are stored as vertex properties \"v:curv-min\"\n and \"v:curv-max\", respectively.\n\nC++: easy3d::SurfaceMeshCurvature::analyze(unsigned int) --> void", pybind11::arg("post_smoothing_steps"));
		cl.def("analyze_tensor", [](easy3d::SurfaceMeshCurvature &o) -> void { return o.analyze_tensor(); }, "");
		cl.def("analyze_tensor", [](easy3d::SurfaceMeshCurvature &o, unsigned int const & a0) -> void { return o.analyze_tensor(a0); }, "", pybind11::arg("post_smoothing_steps"));
		cl.def("analyze_tensor", (void (easy3d::SurfaceMeshCurvature::*)(unsigned int, bool)) &easy3d::SurfaceMeshCurvature::analyze_tensor, "Computes principle curvature information for each vertex, optionally followed by some smoothing iterations\n of the curvature values. Upon finish, the principle curvatures are stored as vertex properties \"v:curv-min\"\n and \"v:curv-max\", respectively.\n\nC++: easy3d::SurfaceMeshCurvature::analyze_tensor(unsigned int, bool) --> void", pybind11::arg("post_smoothing_steps"), pybind11::arg("two_ring_neighborhood"));
		cl.def("compute_mean_curvature", (void (easy3d::SurfaceMeshCurvature::*)()) &easy3d::SurfaceMeshCurvature::compute_mean_curvature, "Computes the mean curvature.\n \n\n This function must be called after calling to the \"analyze\" function.\n\nC++: easy3d::SurfaceMeshCurvature::compute_mean_curvature() --> void");
		cl.def("compute_gauss_curvature", (void (easy3d::SurfaceMeshCurvature::*)()) &easy3d::SurfaceMeshCurvature::compute_gauss_curvature, "Computes the Gaussian curvature.\n \n\n This function must be called after calling to the \"analyze\" function.\n\nC++: easy3d::SurfaceMeshCurvature::compute_gauss_curvature() --> void");
		cl.def("compute_max_abs_curvature", (void (easy3d::SurfaceMeshCurvature::*)()) &easy3d::SurfaceMeshCurvature::compute_max_abs_curvature, "Computes the max. abs. curvature.\n \n\n This function must be called after calling to the \"analyze\" function.\n\nC++: easy3d::SurfaceMeshCurvature::compute_max_abs_curvature() --> void");
		cl.def("mean_curvature", (float (easy3d::SurfaceMeshCurvature::*)(struct easy3d::SurfaceMesh::Vertex) const) &easy3d::SurfaceMeshCurvature::mean_curvature, "return mean curvature\n\nC++: easy3d::SurfaceMeshCurvature::mean_curvature(struct easy3d::SurfaceMesh::Vertex) const --> float", pybind11::arg("v"));
		cl.def("gauss_curvature", (float (easy3d::SurfaceMeshCurvature::*)(struct easy3d::SurfaceMesh::Vertex) const) &easy3d::SurfaceMeshCurvature::gauss_curvature, "return Gaussian curvature\n\nC++: easy3d::SurfaceMeshCurvature::gauss_curvature(struct easy3d::SurfaceMesh::Vertex) const --> float", pybind11::arg("v"));
		cl.def("min_curvature", (float (easy3d::SurfaceMeshCurvature::*)(struct easy3d::SurfaceMesh::Vertex) const) &easy3d::SurfaceMeshCurvature::min_curvature, "return minimum (signed) curvature\n\nC++: easy3d::SurfaceMeshCurvature::min_curvature(struct easy3d::SurfaceMesh::Vertex) const --> float", pybind11::arg("v"));
		cl.def("max_curvature", (float (easy3d::SurfaceMeshCurvature::*)(struct easy3d::SurfaceMesh::Vertex) const) &easy3d::SurfaceMeshCurvature::max_curvature, "return maximum (signed) curvature\n\nC++: easy3d::SurfaceMeshCurvature::max_curvature(struct easy3d::SurfaceMesh::Vertex) const --> float", pybind11::arg("v"));
		cl.def("max_abs_curvature", (float (easy3d::SurfaceMeshCurvature::*)(struct easy3d::SurfaceMesh::Vertex) const) &easy3d::SurfaceMeshCurvature::max_abs_curvature, "return maximum absolute curvature\n\nC++: easy3d::SurfaceMeshCurvature::max_abs_curvature(struct easy3d::SurfaceMesh::Vertex) const --> float", pybind11::arg("v"));
	}

}
