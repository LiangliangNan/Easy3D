#include <easy3d/util/setting.h>

#include <memory>

#include <pybind11/pybind11.h>


#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif


void bind_easy3d_util_setting(pybind11::module_& m)
{
	// easy3d::setting::initialize(const std::string &) file:easy3d/util/setting.h line:54
	m.def("initialize", []() -> void { return easy3d::setting::initialize(); }, "");
	m.def("initialize", (void (*)(const std::string &)) &easy3d::setting::initialize, "Initializes the default setting for rendering.\n \n\n By calling this function with a valid file name, a setting file with an extension \".ini\" will be\n    created on the first startup of the program. This setting file stores the default rendering parameters.\n    Users can modify this file to change the default rendering parameters, then the changes will be\n    effective for the future.\n \n\n A string specifying the name of setting file. Three different values are accepted:\n    - \"\"(an empty string): indicating that no setting file will be created.\n    - \"default\": create a setting file with a title in the form \"AppName.ini\" next to the executable file.\n    - any non-empty string: (if it is a valid path) a setting file with the same name will be created and\n      rendering parameters will be written to this file.\n\nC++: easy3d::setting::initialize(const std::string &) --> void", pybind11::arg("setting_file"));

	// easy3d::setting::save(const std::string &) file:easy3d/util/setting.h line:57
	m.def("save", (bool (*)(const std::string &)) &easy3d::setting::save, "Save the setting (i.e., rendering parameters) to a file.\n\nC++: easy3d::setting::save(const std::string &) --> bool", pybind11::arg("filename"));

	// easy3d::setting::load(const std::string &) file:easy3d/util/setting.h line:60
	m.def("load", (bool (*)(const std::string &)) &easy3d::setting::load, "Load the setting (i.e., rendering parameters) from a file.\n\nC++: easy3d::setting::load(const std::string &) --> bool", pybind11::arg("filename"));
}
