#include <pybind11/pybind11.h>

#include <easy3d/viewer/viewer.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/core/model.h>


namespace py = pybind11;


void init_viewer(py::module& m) {
    py::class_<easy3d::Viewer>(m, "Viewer")
            // Binding constructor
            .def(py::init<const std::string&, int, int, int, bool, bool, int, int, int, int>(),
                 py::arg("title") = "Easy3D Viewer",
                 py::arg("samples") = 4,
                 py::arg("gl_major") = 3,
                 py::arg("gl_minor") = 2,
                 py::arg("full_screen") = false,
                 py::arg("resizable") = true,
                 py::arg("depth_bits") = 24,
                 py::arg("stencil_bits") = 8,
                 py::arg("width") = 800,
                 py::arg("height") = 600)
                    // Binding methods
            .def("run", &easy3d::Viewer::run, py::arg("see_all") = true, "Run the viewer.")
            .def("exit", &easy3d::Viewer::exit, "Terminate the viewer.")
            .def("set_title", &easy3d::Viewer::set_title, py::arg("title"), "Set the window title of the viewer.")
            .def("title", &easy3d::Viewer::title, py::return_value_policy::reference_internal, "Query the window title of the viewer.")
            .def("resize", &easy3d::Viewer::resize, py::arg("width"), py::arg("height"), "Set/Change the viewer size.")
            .def("width", &easy3d::Viewer::width, "Returns the width of the viewer/window.")
            .def("height", &easy3d::Viewer::height, "Returns the height of the viewer/window.")
            .def("dpi_scaling", &easy3d::Viewer::dpi_scaling, "Query the scaling factor for high DPI devices.")
            .def("set_background_color", &easy3d::Viewer::set_background_color, py::arg("color"), "Set the background color of the viewer.")
            .def("background_color", &easy3d::Viewer::background_color, py::return_value_policy::reference_internal, "Query the background color of the viewer.")
            .def("camera", py::overload_cast<>(&easy3d::Viewer::camera, py::const_), py::return_value_policy::reference_internal, "Returns the camera used by the viewer.")
            .def("open", &easy3d::Viewer::open, "Open a model from a file into the viewer.")
            .def("save", &easy3d::Viewer::save, "Save the active model to a file.")
            .def("add_model", py::overload_cast<const std::string&, bool>(&easy3d::Viewer::add_model),
                 py::arg("file_name"), py::arg("create_default_drawables") = true, py::return_value_policy::reference_internal,
                 "Add a model from a file to the viewer.")
            .def("add_model", py::overload_cast<easy3d::Model*, bool>(&easy3d::Viewer::add_model),
                 py::arg("model"), py::arg("create_default_drawables") = true, py::return_value_policy::reference_internal,
                 "Add an existing to the viewer.")
            .def("clear_scene", &easy3d::Viewer::clear_scene, "Delete all visual contents of the viewer.")
            .def("update", &easy3d::Viewer::update, "Update the display (i.e., repaint).")
            .def("fit_screen", &easy3d::Viewer::fit_screen, py::arg("model") = nullptr, "Center the scene or active model on the screen.")
            .def("snapshot", py::overload_cast<const std::string&, float, int, int, bool>(&easy3d::Viewer::snapshot, py::const_),
                 py::arg("file_name"), py::arg("scaling") = 1.0f, py::arg("samples") = 4, py::arg("background") = 1, py::arg("expand") = true,
                 "Take a snapshot of the screen and save it to an image file.");
}


PYBIND11_MODULE(easy3d_viewer, m) {
    m.doc() = "Bindings for Easy3D viewer functions";

    init_viewer(m);
}