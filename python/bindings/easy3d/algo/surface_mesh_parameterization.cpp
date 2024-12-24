#include <easy3d/algo/surface_mesh_parameterization.h>

#include <easy3d/core/surface_mesh.h>

#include <memory>

#include <pybind11/pybind11.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_algo_surface_mesh_parameterization(pybind11::module_& m)
{
	{ // easy3d::SurfaceMeshParameterization file:easy3d/algo/surface_mesh_parameterization.h line:28
		pybind11::class_<easy3d::SurfaceMeshParameterization, std::shared_ptr<easy3d::SurfaceMeshParameterization>> cl(m, "SurfaceMeshParameterization", "A class for surface parameterization.\n \n\n\n \n It implements two parameterization methods described in the following papers:\n  - Bruno Lévy et al. Least squares conformal maps for automatic texture atlas generation. SIGGRAPH, 2002.\n  - Mathieu Desbrun et al. Intrinsic parameterizations of surface meshes. CGF, 21(3):209–218, 2002.");
		cl.def( pybind11::init<class easy3d::SurfaceMesh *>(), pybind11::arg("mesh") );

		cl.def("harmonic", [](easy3d::SurfaceMeshParameterization &o) -> void { return o.harmonic(); }, "");
		cl.def("harmonic", (void (easy3d::SurfaceMeshParameterization::*)(bool)) &easy3d::SurfaceMeshParameterization::harmonic, "Compute discrete harmonic parameterization.\n\nC++: easy3d::SurfaceMeshParameterization::harmonic(bool) --> void", pybind11::arg("use_uniform_weights"));
		cl.def("lscm", (void (easy3d::SurfaceMeshParameterization::*)()) &easy3d::SurfaceMeshParameterization::lscm, "Compute parameterization based on least squares conformal mapping.\n\nC++: easy3d::SurfaceMeshParameterization::lscm() --> void");
	}

}
