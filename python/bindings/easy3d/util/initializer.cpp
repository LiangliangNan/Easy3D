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
	m.def("initialize", []() -> void { return easy3d::initialize(); }, "");
	m.def("initialize", [](bool const & a0) -> void { return easy3d::initialize(a0); }, "", pybind11::arg("info_to_stdout"));
	m.def("initialize", [](bool const & a0, bool const & a1) -> void { return easy3d::initialize(a0, a1); }, "", pybind11::arg("info_to_stdout"), pybind11::arg("use_log_file"));
	m.def("initialize", [](bool const & a0, bool const & a1, bool const & a2) -> void { return easy3d::initialize(a0, a1, a2); }, "", pybind11::arg("info_to_stdout"), pybind11::arg("use_log_file"), pybind11::arg("use_setting_file"));
	m.def("initialize", (void (*)(bool, bool, bool, const std::string &)) &easy3d::initialize, "Initialization of Easy3D.\n \n\n This function initializes logging, setting, and resources. Internally it calls (and is thus identical\n    to calling) logging::initialize(), setting::initialize(), and resource::initialize().\n    For more fine-grained initializations, please refer to the documentation of these functions.\n \n\n  to log messages at a the  level to standard output.\n     and  (including  levels are always logged to standard output.\n \n\n  to create a \".log\" file (which will be created next to the executable program).\n \n\n  to create an \".ini\" file (which will be created next to the executable program).\n    This setting file stores the default rendering parameters. Users can modify this file to change the default\n    rendering parameters, then the changes will be effective for the future.\n \n\n The resource directory containing color maps, shaders, textures, fonts, etc.\n     is the default value, which is the directory coming with the Easy3D distribution.\n    In most cases you should use the default resource directory (unless you want to use different resources).\n\n \n logging::initialize(), setting::initialize(), and resource::initialize().\n\nC++: easy3d::initialize(bool, bool, bool, const std::string &) --> void", pybind11::arg("info_to_stdout"), pybind11::arg("use_log_file"), pybind11::arg("use_setting_file"), pybind11::arg("resource_dir"));

}
