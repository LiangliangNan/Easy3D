#include <easy3d/algo/surface_mesh_sampler.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>

#include <memory>

#include <pybind11/pybind11.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_algo_surface_mesh_sampler(pybind11::module_& m)
{
    { // easy3d::SurfaceMeshSampler file:easy3d/algo/surface_mesh_sampler.h line:38
        pybind11::class_<easy3d::SurfaceMeshSampler, std::shared_ptr<easy3d::SurfaceMeshSampler>> cl(m, "SurfaceMeshSampler", "Sample a surface mesh (near uniformly) into a point cloud.\n \n");
        cl.def( pybind11::init( [](){ return new easy3d::SurfaceMeshSampler(); } ) );
        cl.def_static("apply", [](const class easy3d::SurfaceMesh * a0) -> easy3d::PointCloud * { return easy3d::SurfaceMeshSampler::apply(a0); }, "", pybind11::return_value_policy::automatic, pybind11::arg("mesh"));
        cl.def_static("apply", (class easy3d::PointCloud * (*)(const class easy3d::SurfaceMesh *, int)) &easy3d::SurfaceMeshSampler::apply, "The expected point number, must be greater than the number of vertices of the surface mesh.\n\nC++: easy3d::SurfaceMeshSampler::apply(const class easy3d::SurfaceMesh *, int) --> class easy3d::PointCloud *", pybind11::return_value_policy::automatic, pybind11::arg("mesh"), pybind11::arg("num"));
    }

}
