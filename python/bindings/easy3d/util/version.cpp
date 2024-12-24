#include <easy3d/util/version.h>

#include <memory>

#include <pybind11/pybind11.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif


void bind_easy3d_util_version(pybind11::module_& m)
{
	// easy3d::version_major() file:easy3d/util/version.h line:37
	m.def("version_major", (int (*)()) &easy3d::version_major, "returns the major version number of Easy3D.\n\nC++: easy3d::version_major() --> int");

	// easy3d::version_minor() file:easy3d/util/version.h line:40
	m.def("version_minor", (int (*)()) &easy3d::version_minor, "returns the major version number of Easy3D.\n\nC++: easy3d::version_minor() --> int");

	// easy3d::version_patch() file:easy3d/util/version.h line:43
	m.def("version_patch", (int (*)()) &easy3d::version_patch, "returns the minor version number of Easy3D.\n\nC++: easy3d::version_patch() --> int");

	// easy3d::version_string() file:easy3d/util/version.h line:46
	m.def("version_string", (std::string (*)()) &easy3d::version_string, "returns the version string of Easy3D, in the format [MAJOR].[MINOR].[PATCH], e.g., “2.5.3”.\n\nC++: easy3d::version_string() --> std::string");

	// easy3d::version_number() file:easy3d/util/version.h line:49
	m.def("version_number", (std::size_t (*)()) &easy3d::version_number, "returns the version number of Easy3D, in the format 10[MAJOR]0[MINOR]0[PATCH], e.g., 1020503.\n\nC++: easy3d::version_number() --> std::size_t");

	// easy3d::release_date() file:easy3d/util/version.h line:52
	m.def("release_date", (std::size_t (*)()) &easy3d::release_date, "returns the release date of Easy3D, in the format YYYYMMDD, e.g., 20240305.\n\nC++: easy3d::release_date() --> std::size_t");
}
