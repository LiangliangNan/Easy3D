#include <easy3d/algo/surface_mesh_features.h>
#include <easy3d/core/surface_mesh.h>

#include <memory>

#include <pybind11/pybind11.h>


#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_algo_surface_mesh_features(pybind11::module_& m)
{
	{ // easy3d::SurfaceMeshFeatures file:easy3d/algo/surface_mesh_features.h line:22
		pybind11::class_<easy3d::SurfaceMeshFeatures, std::shared_ptr<easy3d::SurfaceMeshFeatures>> cl(m, "SurfaceMeshFeatures", "Detect and mark feature edges based on boundary or dihedral angle\n \n");
		cl.def( pybind11::init<class easy3d::SurfaceMesh *>(), pybind11::arg("mesh") );

		cl.def("clear", (void (easy3d::SurfaceMeshFeatures::*)()) &easy3d::SurfaceMeshFeatures::clear, "Clear features.\n\nC++: easy3d::SurfaceMeshFeatures::clear() --> void");
		cl.def("detect_boundary", (void (easy3d::SurfaceMeshFeatures::*)()) &easy3d::SurfaceMeshFeatures::detect_boundary, "Mark all boundary edges as features.\n\nC++: easy3d::SurfaceMeshFeatures::detect_boundary() --> void");
		cl.def("detect_angle", (void (easy3d::SurfaceMeshFeatures::*)(float)) &easy3d::SurfaceMeshFeatures::detect_angle, "Mark edges with dihedral angle larger than  as feature.\n\nC++: easy3d::SurfaceMeshFeatures::detect_angle(float) --> void", pybind11::arg("angle"));
	}

}
