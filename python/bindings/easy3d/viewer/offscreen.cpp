// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/viewer.h>
#include <easy3d/core/model.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
namespace py = pybind11;
namespace {
struct PyOffScreen : easy3d::OffScreen {
    using easy3d::OffScreen::OffScreen;
    easy3d::Model* add_model(const std::string& file,bool create,bool fit_scene) override {
        PYBIND11_OVERRIDE(easy3d::Model*,easy3d::OffScreen,add_model,file,create,fit_scene);
    }
};
}
void bind_easy3d_viewer_offscreen(py::module_& m) {
    using namespace easy3d;
    py::class_<OffScreen,std::shared_ptr<OffScreen>,PyOffScreen>(m,"OffScreen","Backend-neutral offscreen rendering")
        .def(py::init<int,int>(),py::arg("width")=800,py::arg("height")=600)
        .def("camera",[](OffScreen& self) { return self.camera(); },py::return_value_policy::reference_internal)
        .def("width",[](const OffScreen& self) { return self.width(); }).def("height",[](const OffScreen& self) { return self.height(); }).def("resize",&OffScreen::resize)
        .def("set_background_color",[](OffScreen& self, const vec4& color) { self.set_background_color(color); })
        .def("background_color",[](const OffScreen& self) -> const vec4& { return self.background_color(); },py::return_value_policy::reference_internal)
        .def("render",&OffScreen::render,py::arg("file_name"),py::arg("scaling")=1.f,py::arg("samples")=4,py::arg("back_ground")=0,py::arg("expand")=true)
        .def("add_model",[](OffScreen& self, const std::string& file, bool create) {
            auto* added = self.add_model(file, create);
            for (const auto& model : self.models()) if (model.get() == added) return model;
            return std::shared_ptr<Model>();
        },py::arg("file_name"),py::arg("create_drawables")=true,py::return_value_policy::reference_internal)
        .def("add_model",[](OffScreen& self, std::shared_ptr<Model> model, bool create) { return self.add_model(model, create) ? model : nullptr; },py::arg("model"),py::arg("create_drawables")=true,py::return_value_policy::reference_internal)
        .def("delete_model",[](OffScreen& self, Model* model) { return self.delete_model(model); })
        .def("models",[](const OffScreen& self) -> const std::vector<std::shared_ptr<Model>>& { return self.models(); },py::return_value_policy::reference_internal)
        .def("current_model",[](const OffScreen& self) { return self.current_model(); },py::return_value_policy::reference_internal)
        .def("add_drawable",[](OffScreen& self, std::shared_ptr<Drawable> drawable) { return self.add_drawable(drawable) ? drawable : nullptr; },py::arg("drawable"),py::return_value_policy::reference_internal)
        .def("delete_drawable",[](OffScreen& self, Drawable* drawable) { return self.delete_drawable(drawable); })
        .def("drawables",[](const OffScreen& self) -> const std::vector<std::shared_ptr<Drawable>>& { return self.drawables(); },py::return_value_policy::reference_internal)
        .def("clear_scene",[](OffScreen& self) { self.clear_scene(); }).def("fit_screen",[](OffScreen& self, const Model* model) { self.fit_screen(model); },py::arg("model")=nullptr);
}
