#include <pybind11/pybind11.h>

#include <easy3d/core/point_cloud.h>
#include <easy3d/algo/point_cloud_normals.h>

namespace py = pybind11;

void init_point_cloud_normals(py::module_ &m) {
    py::class_<easy3d::PointCloudNormals>(m, "PointCloudNormals")
            .def_static(
                    "estimate", &easy3d::PointCloudNormals::estimate,
                    py::arg("cloud"),
                    py::arg("k"),
                    py::arg("compute_curvature") = false,
                    R"doc(
            Estimates the point cloud normals using PCA.

            Parameters:
                cloud (PointCloud): The input point cloud.
                k (int): The number of neighboring points to construct the covariance matrix.
                compute_curvature (bool): Whether to compute the curvature as well. Defaults to False.

            Returns:
                bool: True if successful, False otherwise.
            )doc")
            .def_static(
                    "reorient",
                    [](easy3d::PointCloud *cloud, unsigned int k) {
                        if (!cloud) {
                            throw std::invalid_argument("PointCloud pointer is null.");
                        }
                        return easy3d::PointCloudNormals::reorient(cloud, k);
                    },
                    py::arg("cloud"),
                    py::arg("k"),
                    R"doc(
            Reorients the point cloud normals based on the minimum spanning tree algorithm.

            Parameters:
                cloud (PointCloud): The input point cloud.
                k (int): The number of neighboring points to construct the graph.

            Returns:
                bool: True if successful, False otherwise.
            )doc");
}

PYBIND11_MODULE(easy3d_algo, m) {
    m.doc() = "Bindings for Easy3D algo functions";

    init_point_cloud_normals(m);
}