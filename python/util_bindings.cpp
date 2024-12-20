#include <pybind11/pybind11.h>

#include <easy3d/util/initializer.h>
#include <easy3d/util/stop_watch.h>

namespace py = pybind11;


// Bind the StopWatch class
void init_stop_watch(py::module_ &m) {
    py::class_<easy3d::StopWatch>(m, "StopWatch")
            .def(py::init<>(), R"pbdoc(
                Constructor for StopWatch.
                Automatically starts the timer upon creation.
            )pbdoc")
            .def("start", &easy3d::StopWatch::start, R"pbdoc(
                Starts the timer or resumes it if already started.
            )pbdoc")
            .def("restart", &easy3d::StopWatch::restart, R"pbdoc(
                Restarts the timer. This has the same effect as calling `start()`.
            )pbdoc")
            .def("elapsed_seconds", &easy3d::StopWatch::elapsed_seconds,
                 py::arg("num_digits") = 1, R"pbdoc(
                Returns the elapsed time (in seconds) since the timer started.

                Parameters:
                num_digits (int): Number of decimal places to include in the result.

                Returns:
                float: Elapsed time in seconds.
            )pbdoc")
            .def("time_string", &easy3d::StopWatch::time_string,
                 py::arg("num_digits") = 1, R"pbdoc(
                Returns the elapsed time as a human-readable string.

                Parameters:
                num_digits (int): Number of decimal places to include in the result.

                Returns:
                str: Elapsed time string (e.g., '88ms', '2.3s', '1.7m', '0.1h').
            )pbdoc");
}



PYBIND11_MODULE(easy3d_util, m) {
    m.doc() = "Bindings for Easy3D util functions";

    m.def("initialize", &easy3d::initialize,
          py::arg("use_log_file") = false,
          py::arg("use_setting_file") = false,
          py::arg("resource_dir") = Easy3D_RESOURCE_DIR,
          R"pbdoc(
            Initialize Easy3D resources.

            This function initializes logging, settings, and resources for Easy3D.

            Parameters:
            use_log_file (bool): Set to True to enable logging to a .log file.
            use_setting_file (bool): Set to True to create a settings .ini file.
            resource_dir (str): Path to the resource directory (default: Easy3D_RESOURCE_DIR).
        )pbdoc");

    init_stop_watch(m);
}