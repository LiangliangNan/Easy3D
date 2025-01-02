#include <easy3d/util/resource.h>

#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_util_resource(pybind11::module_& m)
{
	// easy3d::resource::initialize(const std::string &) file:easy3d/util/resource.h line:45
	m.def("initialize", []() -> void { return easy3d::resource::initialize(); }, "");
	m.def("initialize", (void (*)(const std::string &)) &easy3d::resource::initialize, "Initializes the resource directory (that contains color maps, shaders, textures, fonts, etc.).\n \n\n  is the default value, which is the directory coming with the Easy3D\n     distribution. In most cases you should use the default resource directory (unless you want to use\n     different resources).\n\nC++: easy3d::resource::initialize(const std::string &) --> void", pybind11::arg("resource_dir"));

	// easy3d::resource::directory() file:easy3d/util/resource.h line:48
	m.def("resource_directory", (std::string (*)()) &easy3d::resource::directory, "Returns the resource directory (containing color maps, shaders, textures, fonts, etc.)\n\nC++: easy3d::resource::directory() --> std::string");

    // The vertices of a bunny model.
    // easy3d::resource::bunny_vertices file:easy3d/util/resource.h line:53
    m.attr("bunny_vertices") = &easy3d::resource::bunny_vertices;

    // The triangle faces of a bunny model. Each consecutive 3 values denote the vertex indices of a triangle.
    // easy3d::resource::bunny_indices file:easy3d/util/resource.h line:56
    m.attr("bunny_indices") = &easy3d::resource::bunny_indices;
}
