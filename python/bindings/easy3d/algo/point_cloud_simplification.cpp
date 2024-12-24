#include <easy3d/algo/point_cloud_simplification.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/kdtree/kdtree_search.h>

#include <memory>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_algo_point_cloud_simplification(pybind11::module_& m)
{
	{ // easy3d::PointCloudSimplification file:easy3d/algo/point_cloud_simplification.h line:42
		pybind11::class_<easy3d::PointCloudSimplification, std::shared_ptr<easy3d::PointCloudSimplification>> cl(m, "PointCloudSimplification", "PointCloudSimplification provides various point cloud simplification algorithms.\n \n");
		cl.def( pybind11::init( [](){ return new easy3d::PointCloudSimplification(); } ) );
		cl.def_static("average_space", [](class easy3d::PointCloud * a0) -> float { return easy3d::PointCloudSimplification::average_space(a0); }, "", pybind11::arg("cloud"));
		cl.def_static("average_space", [](class easy3d::PointCloud * a0, class easy3d::KdTreeSearch * a1) -> float { return easy3d::PointCloudSimplification::average_space(a0, a1); }, "", pybind11::arg("cloud"), pybind11::arg("kdtree"));
		cl.def_static("average_space", [](class easy3d::PointCloud * a0, class easy3d::KdTreeSearch * a1, int const & a2) -> float { return easy3d::PointCloudSimplification::average_space(a0, a1, a2); }, "", pybind11::arg("cloud"), pybind11::arg("kdtree"), pybind11::arg("k"));
		cl.def_static("average_space", [](class easy3d::PointCloud * a0, class easy3d::KdTreeSearch * a1, int const & a2, bool const & a3) -> float { return easy3d::PointCloudSimplification::average_space(a0, a1, a2, a3); }, "", pybind11::arg("cloud"), pybind11::arg("kdtree"), pybind11::arg("k"), pybind11::arg("accurate"));
		cl.def_static("average_space", (float (*)(class easy3d::PointCloud *, class easy3d::KdTreeSearch *, int, bool, int)) &easy3d::PointCloudSimplification::average_space, "Query the average spacing of a point cloud.\n \n\n The point cloud.\n \n\n The number of nearest points used.\n \n\n True to use every point to get an accurate calculation; false to obtain aa approximate\n                 measure, which uses only a subset (i.e., less than samples) of the points.\n \n\n  Use less than this number of points for the calculation.\n \n\n   A kdtree defined on this point cloud. If null, a new kdtree will be built and used.\n \n\n The average space of the point cloud.\n\nC++: easy3d::PointCloudSimplification::average_space(class easy3d::PointCloud *, class easy3d::KdTreeSearch *, int, bool, int) --> float", pybind11::arg("cloud"), pybind11::arg("kdtree"), pybind11::arg("k"), pybind11::arg("accurate"), pybind11::arg("samples"));
		cl.def_static("grid_simplification", (class std::vector<struct easy3d::PointCloud::Vertex> (*)(class easy3d::PointCloud *, float)) &easy3d::PointCloudSimplification::grid_simplification, "Simplification of a point cloud using a regular grid covering the bounding box of the points. Simplification\n is done by keeping a representative point (chosen arbitrarily) for each cell of the grid. This is non-uniform\n simplification since the representative point is chosen arbitrarily.\n \n\n The point cloud.\n \n\n The size of the cells of the grid.\n \n\n The indices of points to be deleted.\n\nC++: easy3d::PointCloudSimplification::grid_simplification(class easy3d::PointCloud *, float) --> class std::vector<struct easy3d::PointCloud::Vertex>", pybind11::arg("cloud"), pybind11::arg("cell_size"));
		cl.def_static("uniform_simplification", [](class easy3d::PointCloud * a0, float const & a1) -> std::vector<struct easy3d::PointCloud::Vertex> { return easy3d::PointCloudSimplification::uniform_simplification(a0, a1); }, "", pybind11::arg("cloud"), pybind11::arg("epsilon"));
		cl.def_static("uniform_simplification", (class std::vector<struct easy3d::PointCloud::Vertex> (*)(class easy3d::PointCloud *, float, class easy3d::KdTreeSearch *)) &easy3d::PointCloudSimplification::uniform_simplification, "Uniformly downsample a point cloud based on a distance criterion. This function can also be used for\n        removing duplicate points of a point cloud.\n \n\n The point cloud.\n \n\n The minimum allowed distance between points. Two points with a distance smaller than this\n                 value are considered identical. After simplification, the distance of any point pair is\n                 larger than this value.\n \n\n   A kdtree defined on this point cloud. If null, a new kdtree will be built and used.\n \n\n The indices of points to be deleted.\n\nC++: easy3d::PointCloudSimplification::uniform_simplification(class easy3d::PointCloud *, float, class easy3d::KdTreeSearch *) --> class std::vector<struct easy3d::PointCloud::Vertex>", pybind11::arg("cloud"), pybind11::arg("epsilon"), pybind11::arg("kdtree"));
		cl.def_static("uniform_simplification", (class std::vector<struct easy3d::PointCloud::Vertex> (*)(class easy3d::PointCloud *, unsigned int)) &easy3d::PointCloudSimplification::uniform_simplification, "Uniformly downsample a point cloud given the expected point number.\n \n\n The point cloud.\n \n\n   The expected point number, which must be less than or equal to the original point number.\n \n\n The indices of points to be deleted.\n\nC++: easy3d::PointCloudSimplification::uniform_simplification(class easy3d::PointCloud *, unsigned int) --> class std::vector<struct easy3d::PointCloud::Vertex>", pybind11::arg("cloud"), pybind11::arg("num"));
	}

}
