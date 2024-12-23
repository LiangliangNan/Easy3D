#include <pybind11/pybind11.h>

#include <easy3d/util/initializer.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/stop_watch.h>

namespace py = pybind11;


// Bind the resource namespace
void bind_resource(py::module &m) {
    py::module resource_module = m.def_submodule("resource", "Resources used in Easy3D.");

    // Bind the initialize function
    resource_module.def("initialize", &easy3d::resource::initialize,
                        py::arg("resource_dir") = Easy3D_RESOURCE_DIR,
                        R"pbdoc(
                            Initializes the resource directory (contains color maps, shaders, textures, fonts, etc.).
                            The default is Easy3D_RESOURCE_DIR, which comes with the Easy3D distribution.

                            Parameters:
                            resource_dir (str): Path to the resource directory.
                        )pbdoc");

    // Bind the directory function
    resource_module.def("directory", &easy3d::resource::directory, R"pbdoc(
        Returns the resource directory containing color maps, shaders, textures, fonts, etc.

        Returns:
        str: The resource directory path.
    )pbdoc");

    // Bind bunny_vertices
    resource_module.def("bunny_vertices", []() -> const std::vector<easy3d::vec3> & {
                            return easy3d::resource::bunny_vertices;
                        },
                        R"pbdoc(
                            The vertices of the bunny model.

                            Returns:
                            List[vec3]: A list of vec3 objects representing the bunny's vertices.
                        )pbdoc");

    // Bind bunny_indices
    resource_module.def("bunny_indices", []() -> const std::vector<unsigned int> & {
                            return easy3d::resource::bunny_indices;
                        },
                        R"pbdoc(
                            The triangle faces of the bunny model. Each consecutive 3 values denote the vertex indices of a triangle.

                            Returns:
                            List[int]: A list of unsigned integers representing the bunny's triangle indices.
                        )pbdoc");
}

// Bind the StopWatch class
void bind_stop_watch(py::module &m) {
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
                double: Elapsed time in seconds.
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

void init_util(py::module_& m) {
    m.doc() = "Bindings for Easy3D util functions";

    m.def("initialize", &easy3d::initialize,
          py::arg("info_to_stdout") = false,
          py::arg("use_log_file") = true,
          py::arg("use_setting_file") = false,
          py::arg("resource_dir") = Easy3D_RESOURCE_DIR,
          R"pbdoc(
            Initialize Easy3D resources.

            This function initializes logging, settings, and resources for Easy3D.

            Parameters:
            info_to_stdout(bool): Set to True to log messages at the INFO level to standard output.
            use_log_file (bool): Set to True to enable logging to a .log file.
            use_setting_file (bool): Set to True to create a settings .ini file.
            resource_dir (str): Path to the resource directory (default: Easy3D_RESOURCE_DIR).
        )pbdoc");

    // Bind StopWatch class
    bind_stop_watch(m);

    // Bind resource namespace
    bind_resource(m);
}