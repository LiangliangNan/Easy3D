#include <pybind11/pybind11.h>

#include <easy3d/core/point_cloud.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/manipulator.h>


namespace py = pybind11;

void bind_model(py::module& m) {
    py::class_<easy3d::Model>(m, "Model")
            .def("set_name", &easy3d::Model::set_name)
            .def("name", &easy3d::Model::name)
            .def("bounding_box", &easy3d::Model::bounding_box, py::arg("recompute") = false)
            .def("invalidate_bounding_box", &easy3d::Model::invalidate_bounding_box)
            .def("points", py::overload_cast<>(&easy3d::Model::points, py::const_))
            .def("empty", &easy3d::Model::empty)
            .def("property_stats", &easy3d::Model::property_stats)
            .def("set_renderer", &easy3d::Model::set_renderer)
            .def("renderer", py::overload_cast<>(&easy3d::Model::renderer, py::const_))
            .def("set_manipulator", &easy3d::Model::set_manipulator)
            .def("manipulator", py::overload_cast<>(&easy3d::Model::manipulator, py::const_));
}

void bind_point_cloud(py::module& m) {
    py::class_<easy3d::PointCloud, easy3d::Model>(m, "PointCloud")
            .def(py::init<>())
            .def(py::init<const easy3d::PointCloud&>())
            .def("add_vertex", &easy3d::PointCloud::add_vertex)
            .def("vertices_size", &easy3d::PointCloud::vertices_size)
            .def("n_vertices", &easy3d::PointCloud::n_vertices)
            .def("clear", &easy3d::PointCloud::clear)
            .def("resize", &easy3d::PointCloud::resize)
            .def("has_garbage", &easy3d::PointCloud::has_garbage)
            .def("collect_garbage", &easy3d::PointCloud::collect_garbage)
            .def("delete_vertex", &easy3d::PointCloud::delete_vertex)
            .def("is_deleted", &easy3d::PointCloud::is_deleted)
            .def("is_valid", &easy3d::PointCloud::is_valid)
            .def("add_vertex_property", [](easy3d::PointCloud& self, const std::string& name) {
                return self.add_vertex_property<float>(name);
            })
            .def("get_vertex_property", [](easy3d::PointCloud& self, const std::string& name) {
                return self.get_vertex_property<float>(name);
            })
            .def("add_model_property", [](easy3d::PointCloud& self, const std::string& name) {
                return self.add_model_property<float>(name);
            })
            .def("get_model_property", [](easy3d::PointCloud& self, const std::string& name) {
                return self.get_model_property<float>(name);
            });
}

PYBIND11_MODULE(easy3d_core, m) {
    m.doc() = "Bindings for Easy3D core functions";

    bind_model(m);
    bind_point_cloud(m);
}