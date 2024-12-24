#include <easy3d/algo/surface_mesh_remeshing.h>

#include <easy3d/core/surface_mesh.h>

#include <memory>

#include <pybind11/pybind11.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_algo_surface_mesh_remeshing(pybind11::module_& m)
{
	{ // easy3d::SurfaceMeshRemeshing file:easy3d/algo/surface_mesh_remeshing.h line:31
		pybind11::class_<easy3d::SurfaceMeshRemeshing, std::shared_ptr<easy3d::SurfaceMeshRemeshing>> cl(m, "SurfaceMeshRemeshing", "A class for uniform and adaptive surface remeshing.\n\n \n\n \n The algorithm implemented here performs incremental remeshing based on edge collapse, split, flip,\n and tangential relaxation. See the following papers for more details:\n  - Mario Botsch and Leif Kobbelt. A remeshing approach to multiresolution modeling. SGP, 2004.\n  - Marion Dunyach et al. Adaptive remeshing for real-time mesh deformation. EG (Short Papers) 2013.");
		cl.def( pybind11::init<class easy3d::SurfaceMesh *>(), pybind11::arg("mesh") );

		cl.def("uniform_remeshing", [](easy3d::SurfaceMeshRemeshing &o, float const & a0) -> void { return o.uniform_remeshing(a0); }, "", pybind11::arg("edge_length"));
		cl.def("uniform_remeshing", [](easy3d::SurfaceMeshRemeshing &o, float const & a0, unsigned int const & a1) -> void { return o.uniform_remeshing(a0, a1); }, "", pybind11::arg("edge_length"), pybind11::arg("iterations"));
		cl.def("uniform_remeshing", (void (easy3d::SurfaceMeshRemeshing::*)(float, unsigned int, bool)) &easy3d::SurfaceMeshRemeshing::uniform_remeshing, "Perform uniform remeshing.\n \n\n the target edge length.\n \n\n the number of iterations\n \n\n use back-projection to the input surface\n\nC++: easy3d::SurfaceMeshRemeshing::uniform_remeshing(float, unsigned int, bool) --> void", pybind11::arg("edge_length"), pybind11::arg("iterations"), pybind11::arg("use_projection"));
		cl.def("adaptive_remeshing", [](easy3d::SurfaceMeshRemeshing &o, float const & a0, float const & a1, float const & a2) -> void { return o.adaptive_remeshing(a0, a1, a2); }, "", pybind11::arg("min_edge_length"), pybind11::arg("max_edge_length"), pybind11::arg("approx_error"));
		cl.def("adaptive_remeshing", [](easy3d::SurfaceMeshRemeshing &o, float const & a0, float const & a1, float const & a2, unsigned int const & a3) -> void { return o.adaptive_remeshing(a0, a1, a2, a3); }, "", pybind11::arg("min_edge_length"), pybind11::arg("max_edge_length"), pybind11::arg("approx_error"), pybind11::arg("iterations"));
		cl.def("adaptive_remeshing", (void (easy3d::SurfaceMeshRemeshing::*)(float, float, float, unsigned int, bool)) &easy3d::SurfaceMeshRemeshing::adaptive_remeshing, "Perform adaptive remeshing.\n \n\n the minimum edge length.\n \n\n the maximum edge length.\n \n\n the maximum approximation error\n \n\n the number of iterations\n \n\n use back-projection to the input surface\n\nC++: easy3d::SurfaceMeshRemeshing::adaptive_remeshing(float, float, float, unsigned int, bool) --> void", pybind11::arg("min_edge_length"), pybind11::arg("max_edge_length"), pybind11::arg("approx_error"), pybind11::arg("iterations"), pybind11::arg("use_projection"));
	}

}
