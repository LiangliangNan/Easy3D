// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/viewer.h>
#include <easy3d/core/model.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/backend.h>
#include <easy3d/renderer/drawable.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;
namespace {
struct PyOffScreen : easy3d::OffScreen {
    using easy3d::OffScreen::OffScreen;
    easy3d::Model* add_model(const std::string& file,bool create) override {
        PYBIND11_OVERRIDE(easy3d::Model*,easy3d::OffScreen,add_model,file,create);
    }
};
}
void bind_easy3d_viewer_offscreen(py::module_& m) {
    using namespace easy3d;
    py::enum_<Backend>(m,"Backend")
        .value("Automatic",Backend::Automatic).value("Vulkan",Backend::Vulkan).value("OpenGL",Backend::OpenGL);
    py::class_<OffScreen,std::shared_ptr<OffScreen>,PyOffScreen>(m,"OffScreen","Backend-neutral offscreen rendering")
        .def(py::init<int,int>(),py::arg("width")=800,py::arg("height")=600)
        .def("camera",static_cast<Camera*(OffScreen::*)()>(&OffScreen::camera),py::return_value_policy::reference_internal)
        .def("width",&OffScreen::width).def("height",&OffScreen::height).def("resize",&OffScreen::resize)
        .def("set_background_color",&OffScreen::set_background_color)
        .def("background_color",&OffScreen::background_color,py::return_value_policy::reference_internal)
        .def("render",&OffScreen::render,py::arg("file_name"),py::arg("scaling")=1.f,py::arg("samples")=4,py::arg("back_ground")=1,py::arg("expand")=true)
        .def("add_model",static_cast<Model*(OffScreen::*)(const std::string&,bool)>(&OffScreen::add_model),py::arg("file_name"),py::arg("create_default_drawables")=true,py::return_value_policy::reference_internal)
        .def("add_model",static_cast<Model*(OffScreen::*)(std::shared_ptr<Model>,bool)>(&OffScreen::add_model),py::arg("model"),py::arg("create_default_drawables")=true,py::return_value_policy::reference_internal)
        .def("delete_model",&OffScreen::delete_model)
        .def("models",&OffScreen::models,py::return_value_policy::reference_internal)
        .def("current_model",&OffScreen::current_model,py::return_value_policy::reference_internal)
        .def("add_drawable",static_cast<Drawable*(OffScreen::*)(std::shared_ptr<Drawable>)>(&OffScreen::add_drawable),py::arg("drawable"),py::return_value_policy::reference_internal)
        .def("delete_drawable",&OffScreen::delete_drawable)
        .def("drawables",&OffScreen::drawables,py::return_value_policy::reference_internal)
        .def("clear_scene",&OffScreen::clear_scene).def("fit_screen",&OffScreen::fit_screen,py::arg("model")=nullptr);
}
