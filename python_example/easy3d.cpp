#include "easy3d.h"

#include <pybind11/pybind11.h>

namespace py = pybind11;
constexpr auto byref = py::return_value_policy::reference_internal;

PYBIND11_MODULE(Easy3DLib, m) {
    m.doc() = "This is a simple example showing how to use Easy3D in Python";

    py::class_<Easy3DViewer>(m, "Easy3DViewer")
            .def(py::init<>())
            .def("run", &Easy3DViewer::run, py::call_guard<py::gil_scoped_release>())
//            .def_readonly("v_data", &Easy3DViewer::v_data, byref)
//            .def_readonly("v_gamma", &Easy3DViewer::v_gamma, byref)
            ;
}