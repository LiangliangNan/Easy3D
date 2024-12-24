#include <easy3d/renderer/opengl_error.h>
#include <memory>
#include <string>
#include <string_view>

#include <functional>
#include <pybind11/pybind11.h>
#include <string>
#include <pybind11/stl.h>


#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_renderer_opengl_error(pybind11::module_& m)
{
	// easy3d::opengl::check_gl_error(const std::string &, const std::string &, int) file:easy3d/renderer/opengl_error.h line:81
	M("easy3d::opengl").def("check_gl_error", (bool (*)(const std::string &, const std::string &, int)) &easy3d::opengl::check_gl_error, "C++: easy3d::opengl::check_gl_error(const std::string &, const std::string &, int) --> bool", pybind11::arg("file"), pybind11::arg("function"), pybind11::arg("line"));

	// easy3d::opengl::check_frame_buffer_error(const std::string &, const std::string &, int) file:easy3d/renderer/opengl_error.h line:82
	M("easy3d::opengl").def("check_frame_buffer_error", (bool (*)(const std::string &, const std::string &, int)) &easy3d::opengl::check_frame_buffer_error, "C++: easy3d::opengl::check_frame_buffer_error(const std::string &, const std::string &, int) --> bool", pybind11::arg("file"), pybind11::arg("function"), pybind11::arg("line"));

	// easy3d::opengl::gl_error(std::string &) file:easy3d/renderer/opengl_error.h line:86
	M("easy3d::opengl").def("gl_error", (bool (*)(std::string &)) &easy3d::opengl::gl_error, "C++: easy3d::opengl::gl_error(std::string &) --> bool", pybind11::arg("log"));

	// easy3d::opengl::frame_buffer_error(std::string &) file:easy3d/renderer/opengl_error.h line:87
	M("easy3d::opengl").def("frame_buffer_error", (bool (*)(std::string &)) &easy3d::opengl::frame_buffer_error, "C++: easy3d::opengl::frame_buffer_error(std::string &) --> bool", pybind11::arg("log"));

	// easy3d::opengl::setup_gl_debug_callback() file:easy3d/renderer/opengl_error.h line:89
	M("easy3d::opengl").def("setup_gl_debug_callback", (void (*)()) &easy3d::opengl::setup_gl_debug_callback, "C++: easy3d::opengl::setup_gl_debug_callback() --> void");

}
