#include <easy3d/algo/surface_mesh_smoothing.h>
#include <easy3d/core/surface_mesh.h>

#include <memory>

#include <pybind11/pybind11.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_algo_surface_mesh_smoothing(pybind11::module_& m)
{
	{ // easy3d::SurfaceMeshSmoothing file:easy3d/algo/surface_mesh_smoothing.h line:26
		pybind11::class_<easy3d::SurfaceMeshSmoothing, std::shared_ptr<easy3d::SurfaceMeshSmoothing>> cl(m, "SurfaceMeshSmoothing", "A class for Laplacian smoothing.\n \n\n\n See the following papers for more details:\n  - Mathieu Desbrun et al. Implicit fairing of irregular meshes using diffusion and curvature flow. SIGGRAPH, 1999.\n  - Misha Kazhdan et al. Can mean‐curvature flow be modified to be non‐singular? CGF, 2012.");
		cl.def( pybind11::init<class easy3d::SurfaceMesh *>(), pybind11::arg("mesh") );

		cl.def("explicit_smoothing", [](easy3d::SurfaceMeshSmoothing &o) -> void { return o.explicit_smoothing(); }, "");
		cl.def("explicit_smoothing", [](easy3d::SurfaceMeshSmoothing &o, unsigned int const & a0) -> void { return o.explicit_smoothing(a0); }, "", pybind11::arg("iters"));
		cl.def("explicit_smoothing", (void (easy3d::SurfaceMeshSmoothing::*)(unsigned int, bool)) &easy3d::SurfaceMeshSmoothing::explicit_smoothing, "Perform  iterations of explicit Laplacian smoothing.\n Decide whether to use uniform Laplacian or cotan Laplacian (default: cotan).\n\nC++: easy3d::SurfaceMeshSmoothing::explicit_smoothing(unsigned int, bool) --> void", pybind11::arg("iters"), pybind11::arg("use_uniform_laplace"));
		cl.def("implicit_smoothing", [](easy3d::SurfaceMeshSmoothing &o) -> void { return o.implicit_smoothing(); }, "");
		cl.def("implicit_smoothing", [](easy3d::SurfaceMeshSmoothing &o, float const & a0) -> void { return o.implicit_smoothing(a0); }, "", pybind11::arg("timestep"));
		cl.def("implicit_smoothing", [](easy3d::SurfaceMeshSmoothing &o, float const & a0, bool const & a1) -> void { return o.implicit_smoothing(a0, a1); }, "", pybind11::arg("timestep"), pybind11::arg("use_uniform_laplace"));
		cl.def("implicit_smoothing", (void (easy3d::SurfaceMeshSmoothing::*)(float, bool, bool)) &easy3d::SurfaceMeshSmoothing::implicit_smoothing, "Perform implicit Laplacian smoothing with \n Decide whether to use uniform Laplacian or cotan Laplacian (default: cotan).\n Decide whether to re-center and re-scale model after smoothing (default: true).\n\nC++: easy3d::SurfaceMeshSmoothing::implicit_smoothing(float, bool, bool) --> void", pybind11::arg("timestep"), pybind11::arg("use_uniform_laplace"), pybind11::arg("rescale"));
		cl.def("initialize", [](easy3d::SurfaceMeshSmoothing &o) -> void { return o.initialize(); }, "");
		cl.def("initialize", (void (easy3d::SurfaceMeshSmoothing::*)(bool)) &easy3d::SurfaceMeshSmoothing::initialize, "Initialize edge and vertex weights.\n\nC++: easy3d::SurfaceMeshSmoothing::initialize(bool) --> void", pybind11::arg("use_uniform_laplace"));
	}

}
