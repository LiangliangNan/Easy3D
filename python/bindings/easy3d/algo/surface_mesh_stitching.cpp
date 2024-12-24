#include <easy3d/algo/surface_mesh_stitching.h>
#include <easy3d/core/surface_mesh.h>

#include <memory>

#include <pybind11/pybind11.h>


#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_algo_surface_mesh_stitching(pybind11::module_& m)
{
	{ // easy3d::SurfaceMeshStitching file:easy3d/algo/surface_mesh_stitching.h line:47
		pybind11::class_<easy3d::SurfaceMeshStitching, std::shared_ptr<easy3d::SurfaceMeshStitching>> cl(m, "SurfaceMeshStitching", "Stitch coincident border edges of a surface mesh.\n\n \n\n \n This class only performs stitching, without reversing the orientation of components having\n coincident but incompatible boundary cycles. It dose the same thing as Surfacer::stitch_borders()\n To stitch incompatible boundaries please use Surfacer::merge_reversible_connected_components().");
		cl.def( pybind11::init<class easy3d::SurfaceMesh *>(), pybind11::arg("mesh") );

		cl.def( pybind11::init( [](easy3d::SurfaceMeshStitching const &o){ return new easy3d::SurfaceMeshStitching(o); } ) );
		cl.def("apply", [](easy3d::SurfaceMeshStitching &o) -> void { return o.apply(); }, "");
		cl.def("apply", (void (easy3d::SurfaceMeshStitching::*)(float)) &easy3d::SurfaceMeshStitching::apply, "C++: easy3d::SurfaceMeshStitching::apply(float) --> void", pybind11::arg("dist_threshold"));
		cl.def("assign", (class easy3d::SurfaceMeshStitching & (easy3d::SurfaceMeshStitching::*)(const class easy3d::SurfaceMeshStitching &)) &easy3d::SurfaceMeshStitching::operator=, "C++: easy3d::SurfaceMeshStitching::operator=(const class easy3d::SurfaceMeshStitching &) --> class easy3d::SurfaceMeshStitching &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}

}
