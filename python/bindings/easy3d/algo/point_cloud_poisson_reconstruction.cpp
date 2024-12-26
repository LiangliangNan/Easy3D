#include <easy3d/algo/point_cloud_poisson_reconstruction.h>
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

void bind_easy3d_algo_point_cloud_poisson_reconstruction(pybind11::module_& m)
{
    { // easy3d::PoissonReconstruction file:easy3d/algo/point_cloud_poisson_reconstruction.h line:41
        pybind11::class_<easy3d::PoissonReconstruction, std::shared_ptr<easy3d::PoissonReconstruction>> cl(m, "PoissonReconstruction", "Poisson surface reconstruction.\n \n");
        cl.def( pybind11::init( [](){ return new easy3d::PoissonReconstruction(); } ) );
        cl.def("set_depth", (void (easy3d::PoissonReconstruction::*)(int)) &easy3d::PoissonReconstruction::set_depth, "Set reconstruction depth.\n This integer is the maximum depth of the tree that will be used for surface reconstruction. Running at depth\n d corresponds to solving on a voxel grid whose resolution is no larger than 2^d x 2^d x 2^d. Note that since\n the reconstructor adapts the octree to the sampling density, the specified reconstruction depth is only an\n upper bound. The default value for this parameter is 8.\n\nC++: easy3d::PoissonReconstruction::set_depth(int) --> void", pybind11::arg("d"));
        cl.def("set_samples_per_node", (void (easy3d::PoissonReconstruction::*)(float)) &easy3d::PoissonReconstruction::set_samples_per_node, "Set the minimum number of samples.\n This floating point value specifies the minimum number of sample points that should fall within an octree\n node as the octree construction is adapted to sampling density. For noise-free samples, small values in the\n range [1.0 - 5.0] can be used. For more noisy samples, larger values in the range [15.0 - 20.0] may be needed\n to provide a smoother, noise-reduced, reconstruction. The default value is 1.0.\n\nC++: easy3d::PoissonReconstruction::set_samples_per_node(float) --> void", pybind11::arg("s"));

        // both will work
        //cl.def("apply", [](easy3d::PoissonReconstruction const &o, const class easy3d::PointCloud * a0, const std::string &density_attr_name) -> easy3d::SurfaceMesh * { return o.apply(a0, density_attr_name); }, "", pybind11::return_value_policy::automatic, pybind11::arg("cloud"), pybind11::arg("density_attr_name") = "v:density");
        cl.def("apply", (class easy3d::SurfaceMesh * (easy3d::PoissonReconstruction::*)(const class easy3d::PointCloud *, const std::string &) const) &easy3d::PoissonReconstruction::apply, "reconstruction\n\nC++: easy3d::PoissonReconstruction::apply(const class easy3d::PointCloud *, const std::string &) const --> class easy3d::SurfaceMesh *", pybind11::return_value_policy::automatic, pybind11::arg("cloud"), pybind11::arg("density_attr_name") = "v:density");

        cl.def_static("trim", (class easy3d::SurfaceMesh * (*)(class easy3d::SurfaceMesh *, float, float, bool, const std::string &)) &easy3d::PoissonReconstruction::trim, "Trim the reconstructed surface model based on the density attribute.\n\nC++: easy3d::PoissonReconstruction::trim(class easy3d::SurfaceMesh *, const std::string &, float, float, bool) --> class easy3d::SurfaceMesh *", pybind11::return_value_policy::automatic, pybind11::arg("mesh"), pybind11::arg("trim_value"), pybind11::arg("area_ratio"), pybind11::arg("triangulate"), pybind11::arg("density_attr_name") = "v:density");

        cl.def("set_full_depth", (void (easy3d::PoissonReconstruction::*)(int)) &easy3d::PoissonReconstruction::set_full_depth, "Other parameters for Poisson surface reconstruction algorithm.\n These parameters are usually not needed\n\nC++: easy3d::PoissonReconstruction::set_full_depth(int) --> void", pybind11::arg("v"));
        cl.def("set_cg_depth", (void (easy3d::PoissonReconstruction::*)(int)) &easy3d::PoissonReconstruction::set_cg_depth, "C++: easy3d::PoissonReconstruction::set_cg_depth(int) --> void", pybind11::arg("v"));
        cl.def("set_scale", (void (easy3d::PoissonReconstruction::*)(float)) &easy3d::PoissonReconstruction::set_scale, "C++: easy3d::PoissonReconstruction::set_scale(float) --> void", pybind11::arg("v"));
        cl.def("set_point_weight", (void (easy3d::PoissonReconstruction::*)(float)) &easy3d::PoissonReconstruction::set_point_weight, "C++: easy3d::PoissonReconstruction::set_point_weight(float) --> void", pybind11::arg("v"));
        cl.def("set_gs_iter", (void (easy3d::PoissonReconstruction::*)(int)) &easy3d::PoissonReconstruction::set_gs_iter, "C++: easy3d::PoissonReconstruction::set_gs_iter(int) --> void", pybind11::arg("v"));
        cl.def("set_verbose", (void (easy3d::PoissonReconstruction::*)(bool)) &easy3d::PoissonReconstruction::set_verbose, "C++: easy3d::PoissonReconstruction::set_verbose(bool) --> void", pybind11::arg("v"));
    }
}
