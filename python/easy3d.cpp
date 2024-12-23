#include <pybind11/pybind11.h>

namespace py = pybind11;

void init_algo(py::module_& m);
void init_core(py::module_& m);
void init_fileio(py::module_& m);
void init_util(py::module_& m);
void init_viewer(py::module_& m);

PYBIND11_MODULE(easy3d, m) {
    m.doc() = "Bindings for Easy3D algo functions";

    init_algo(m);
    init_core(m);
    init_fileio(m);
    init_util(m);
    init_viewer(m);
}