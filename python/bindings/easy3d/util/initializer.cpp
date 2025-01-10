#include <easy3d/util/initializer.h>

#include <memory>

#include <pybind11/pybind11.h>


#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif


void bind_easy3d_util_initializer(pybind11::module_& m)
{
	// easy3d::initialize(bool, bool, bool, const std::string &) file:easy3d/util/initializer.h line:52

    // Bind the initialize function using a lambda
    m.def("initialize",
          [&](bool info_to_stdout, bool use_log_file, bool use_setting_file) {
              // Call the actual Easy3D initialize function
              easy3d::initialize(info_to_stdout, use_log_file, use_setting_file);
          },
          pybind11::arg("info_to_stdout") = false,
          pybind11::arg("use_log_file") = true,
          pybind11::arg("use_setting_file") = false,
          R"(
          Initialization of Easy3D.

          This function initializes logging, setting, and resources.
          Parameters:
              info_to_stdout (bool): True to log messages at the INFO level to standard output.
                  WARNING and ERROR (including FATAL) levels are always logged to standard output.
              use_log_file (bool): True to create a ".log" file (which will be created next to the executable program).
              use_setting_file (bool): True to create an ".ini" file (which will be created next to the executable program).
                  This setting file stores the default rendering parameters. Users can modify this file to change the default
                  rendering parameters, then the changes will be effective for the future.
          )"
    );
}