#include <easy3d/algo/gaussian_noise.h>
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

void bind_easy3d_algo_gaussian_noise(pybind11::module_& m)
{
    { // easy3d::GaussianNoise file:easy3d/algo/gaussian_noise.h line:38
        pybind11::class_<easy3d::GaussianNoise, std::shared_ptr<easy3d::GaussianNoise>> cl(m, "GaussianNoise", "Add Gaussian noise to 3D models.\n \n");
        cl.def( pybind11::init( [](){ return new easy3d::GaussianNoise(); } ) );
        cl.def_static("apply", (void (*)(class easy3d::SurfaceMesh *, float)) &easy3d::GaussianNoise::apply, "Add Gaussian noise (that has a normal distribution) to the surface mesh.\n \n\n The surface mesh.\n \n\n The standard deviation of the noise distribution. So about 68 percent of the noise values are\n              within one standard deviation of the mean (mathematically, μ ± σ, where μ is the arithmetic\n              mean), about 95 percent are within two standard deviations (μ ± 2σ).\n\nC++: easy3d::GaussianNoise::apply(class easy3d::SurfaceMesh *, float) --> void", pybind11::arg("mesh"), pybind11::arg("sigma"));
        cl.def_static("apply", (void (*)(class easy3d::PointCloud *, float)) &easy3d::GaussianNoise::apply, "Add Gaussian noise (that has a normal distribution) to a point cloud.\n \n\n The point cloud.\n \n\n The standard deviation of the noise distribution. So about 68 percent of the noise values are\n              within one standard deviation of the mean (mathematically, μ ± σ, where μ is the arithmetic\n              mean), about 95 percent are within two standard deviations (μ ± 2σ).\n\nC++: easy3d::GaussianNoise::apply(class easy3d::PointCloud *, float) --> void", pybind11::arg("cloud"), pybind11::arg("sigma"));
    }

}
