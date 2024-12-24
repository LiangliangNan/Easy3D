#include <easy3d/algo/point_cloud_normals.h>
#include <easy3d/core/point_cloud.h>

#include <memory>

#include <pybind11/pybind11.h>


#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_algo_point_cloud_normals(pybind11::module_& m)
{
	{ // easy3d::PointCloudNormals file:easy3d/algo/point_cloud_normals.h line:40
		pybind11::class_<easy3d::PointCloudNormals, std::shared_ptr<easy3d::PointCloudNormals>> cl(m, "PointCloudNormals", "Estimate point cloud normals. It also allows to reorients the point cloud normals based on a minimum\n spanning tree algorithm.\n \n");
		cl.def( pybind11::init( [](){ return new easy3d::PointCloudNormals(); } ) );

		cl.def_static("estimate", &easy3d::PointCloudNormals::estimate,
						pybind11::arg("cloud"),
						pybind11::arg("k"),
						pybind11::arg("compute_curvature") = false,
						R"doc(
            Estimates the point cloud normals using PCA.

            Parameters:
                cloud (PointCloud): The input point cloud.
                k (int): The number of neighboring points to construct the covariance matrix.
                compute_curvature (bool): Whether to compute the curvature as well. Defaults to False.

            Returns:
                bool: True if successful, False otherwise.
            )doc");

		cl.def_static(
						"reorient",
						[](easy3d::PointCloud *cloud, unsigned int k) {
							if (!cloud) {
								throw std::invalid_argument("PointCloud pointer is null.");
							}
							return easy3d::PointCloudNormals::reorient(cloud, k);
						},
						pybind11::arg("cloud"),
						pybind11::arg("k"),
						R"doc(
            Reorients the point cloud normals based on the minimum spanning tree algorithm.

            Parameters:
                cloud (PointCloud): The input point cloud.
                k (int): The number of neighboring points to construct the graph.

            Returns:
                bool: True if successful, False otherwise.
            )doc");
	}
}
