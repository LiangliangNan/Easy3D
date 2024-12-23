#include <pybind11/pybind11.h>

#include <easy3d/core/point_cloud.h>
#include <easy3d/fileio/point_cloud_io.h>

namespace py = pybind11;

void bind_point_cloud_io(py::module_ &m) {
    py::class_<easy3d::PointCloudIO>(m, "PointCloudIO")
        .def_static("load",
                    &easy3d::PointCloudIO::load,
                    py::arg("file_name"),
                    "Reads a point cloud from file")
        .def_static("save",
                    &easy3d::PointCloudIO::save,
                    py::arg("file_name"),
                    py::arg("cloud"),
                    "Saves a point cloud to a file");

    py::module_ io = m.def_submodule("io", "IO operations for point clouds");

    io.def("load_bin",
           &easy3d::io::load_bin,
           py::arg("file_name"),
           py::arg("cloud"),
           "Reads point cloud from a bin format file");
    io.def("save_bin",
           &easy3d::io::save_bin,
           py::arg("file_name"),
           py::arg("cloud"),
           "Saves a point cloud to a bin format file");
    io.def("load_xyz",
           &easy3d::io::load_xyz,
           py::arg("file_name"),
           py::arg("cloud"),
           "Reads point cloud from an xyz format file");
    io.def("save_xyz",
           &easy3d::io::save_xyz,
           py::arg("file_name"),
           py::arg("cloud"),
           "Saves a point cloud to an xyz format file");
    io.def("load_bxyz",
           &easy3d::io::load_bxyz,
           py::arg("file_name"),
           py::arg("cloud"),
           "Reads point cloud from a binary xyz format file");
    io.def("save_bxyz",
           &easy3d::io::save_bxyz,
           py::arg("file_name"),
           py::arg("cloud"),
           "Saves a point cloud to a binary xyz format file");
    io.def("load_ply",
           &easy3d::io::load_ply,
           py::arg("file_name"),
           py::arg("cloud"),
           "Reads point cloud from a ply format file");
    io.def("save_ply",
           &easy3d::io::save_ply,
           py::arg("file_name"),
           py::arg("cloud"),
           py::arg("binary") = true,
           "Saves a point cloud to a ply format file");
    io.def("load_las",
           &easy3d::io::load_las,
           py::arg("file_name"),
           py::arg("cloud"),
           "Reads point cloud from a las/laz format file");
    io.def("save_las",
           &easy3d::io::save_las,
           py::arg("file_name"),
           py::arg("cloud"),
           "Saves a point cloud to a las/laz format file");
}


void init_fileio(py::module_& m){
    m.doc() = "Bindings for Easy3D fileio functions";

    bind_point_cloud_io(m);
}