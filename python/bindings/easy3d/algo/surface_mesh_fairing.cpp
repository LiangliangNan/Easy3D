#include <easy3d/algo/surface_mesh_fairing.h>
#include <easy3d/core/surface_mesh.h>

#include <memory>

#include <pybind11/pybind11.h>


#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_algo_surface_mesh_fairing(pybind11::module_& m)
{
	{ // easy3d::SurfaceMeshFairing file:easy3d/algo/surface_mesh_fairing.h line:26
		pybind11::class_<easy3d::SurfaceMeshFairing, std::shared_ptr<easy3d::SurfaceMeshFairing>> cl(m, "SurfaceMeshFairing", "A class for implicitly fairing a surface mesh.\n \n\n\n See the following paper for more details:\n  - Mathieu Desbrun et al. Implicit fairing of irregular meshes using diffusion and curvature flow. SIGGRAPH, 1999.");
		cl.def( pybind11::init<class easy3d::SurfaceMesh *>(), pybind11::arg("mesh") );

		cl.def("minimize_area", (void (easy3d::SurfaceMeshFairing::*)()) &easy3d::SurfaceMeshFairing::minimize_area, "minimize surface area (class SurfaceFairing::fair(1))\n\nC++: easy3d::SurfaceMeshFairing::minimize_area() --> void");
		cl.def("minimize_curvature", (void (easy3d::SurfaceMeshFairing::*)()) &easy3d::SurfaceMeshFairing::minimize_curvature, "minimize surface curvature (class SurfaceFairing::fair(2))\n\nC++: easy3d::SurfaceMeshFairing::minimize_curvature() --> void");
		cl.def("fair", [](easy3d::SurfaceMeshFairing &o) -> void { return o.fair(); }, "");
		cl.def("fair", (void (easy3d::SurfaceMeshFairing::*)(unsigned int)) &easy3d::SurfaceMeshFairing::fair, "compute surface by solving k-harmonic equation\n\nC++: easy3d::SurfaceMeshFairing::fair(unsigned int) --> void", pybind11::arg("k"));
	}

}
