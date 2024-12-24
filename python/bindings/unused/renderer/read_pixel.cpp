#include <easy3d/renderer/read_pixel.h>
#include <memory>
#include <string>
#include <string_view>
#include <vector>

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

void bind_easy3d_renderer_read_pixel(pybind11::module_& m)
{
	// easy3d::opengl::read_depth(float &, int, int) file:easy3d/renderer/read_pixel.h line:50
	M("easy3d::opengl").def("read_depth", (void (*)(float &, int, int)) &easy3d::opengl::read_depth, "C++: easy3d::opengl::read_depth(float &, int, int) --> void", pybind11::arg("depth"), pybind11::arg("x"), pybind11::arg("y"));

	// easy3d::opengl::read_depth_ms(float &, int, int) file:easy3d/renderer/read_pixel.h line:51
	M("easy3d::opengl").def("read_depth_ms", (void (*)(float &, int, int)) &easy3d::opengl::read_depth_ms, "C++: easy3d::opengl::read_depth_ms(float &, int, int) --> void", pybind11::arg("depth"), pybind11::arg("x"), pybind11::arg("y"));

	// easy3d::opengl::read_color(int, class std::vector<unsigned char> &, unsigned int, bool) file:easy3d/renderer/read_pixel.h line:55
	M("easy3d::opengl").def("read_color", [](int const & a0, class std::vector<unsigned char> & a1, unsigned int const & a2) -> void { return easy3d::opengl::read_color(a0, a1, a2); }, "", pybind11::arg("index"), pybind11::arg("buffer"), pybind11::arg("format"));
	M("easy3d::opengl").def("read_color", (void (*)(int, class std::vector<unsigned char> &, unsigned int, bool)) &easy3d::opengl::read_color, "C++: easy3d::opengl::read_color(int, class std::vector<unsigned char> &, unsigned int, bool) --> void", pybind11::arg("index"), pybind11::arg("buffer"), pybind11::arg("format"), pybind11::arg("flip_vertically"));

	// easy3d::opengl::read_color_ms(int, class std::vector<unsigned char> &, unsigned int, bool) file:easy3d/renderer/read_pixel.h line:56
	M("easy3d::opengl").def("read_color_ms", [](int const & a0, class std::vector<unsigned char> & a1, unsigned int const & a2) -> void { return easy3d::opengl::read_color_ms(a0, a1, a2); }, "", pybind11::arg("index"), pybind11::arg("buffer"), pybind11::arg("format"));
	M("easy3d::opengl").def("read_color_ms", (void (*)(int, class std::vector<unsigned char> &, unsigned int, bool)) &easy3d::opengl::read_color_ms, "C++: easy3d::opengl::read_color_ms(int, class std::vector<unsigned char> &, unsigned int, bool) --> void", pybind11::arg("index"), pybind11::arg("buffer"), pybind11::arg("format"), pybind11::arg("flip_vertically"));

	// easy3d::opengl::read_depth(class std::vector<float> &, bool) file:easy3d/renderer/read_pixel.h line:59
	M("easy3d::opengl").def("read_depth", [](class std::vector<float> & a0) -> void { return easy3d::opengl::read_depth(a0); }, "", pybind11::arg("buffer"));
	M("easy3d::opengl").def("read_depth", (void (*)(class std::vector<float> &, bool)) &easy3d::opengl::read_depth, "C++: easy3d::opengl::read_depth(class std::vector<float> &, bool) --> void", pybind11::arg("buffer"), pybind11::arg("flip_vertically"));

	// easy3d::opengl::read_depth_ms(class std::vector<float> &, bool) file:easy3d/renderer/read_pixel.h line:60
	M("easy3d::opengl").def("read_depth_ms", [](class std::vector<float> & a0) -> void { return easy3d::opengl::read_depth_ms(a0); }, "", pybind11::arg("buffer"));
	M("easy3d::opengl").def("read_depth_ms", (void (*)(class std::vector<float> &, bool)) &easy3d::opengl::read_depth_ms, "C++: easy3d::opengl::read_depth_ms(class std::vector<float> &, bool) --> void", pybind11::arg("buffer"), pybind11::arg("flip_vertically"));

	// easy3d::opengl::snapshot_color(const std::string &) file:easy3d/renderer/read_pixel.h line:67
	M("easy3d::opengl").def("snapshot_color", (void (*)(const std::string &)) &easy3d::opengl::snapshot_color, "///////////////////////////////////////////////////////////////////////\n\nC++: easy3d::opengl::snapshot_color(const std::string &) --> void", pybind11::arg("file_name"));

	// easy3d::opengl::snapshot_color_ms(int, const std::string &) file:easy3d/renderer/read_pixel.h line:68
	M("easy3d::opengl").def("snapshot_color_ms", (void (*)(int, const std::string &)) &easy3d::opengl::snapshot_color_ms, "C++: easy3d::opengl::snapshot_color_ms(int, const std::string &) --> void", pybind11::arg("index"), pybind11::arg("file_name"));

	// easy3d::opengl::snapshot_depth(const std::string &) file:easy3d/renderer/read_pixel.h line:72
	M("easy3d::opengl").def("snapshot_depth", (void (*)(const std::string &)) &easy3d::opengl::snapshot_depth, "C++: easy3d::opengl::snapshot_depth(const std::string &) --> void", pybind11::arg("file_name"));

	// easy3d::opengl::snapshot_depth_ms(const std::string &) file:easy3d/renderer/read_pixel.h line:73
	M("easy3d::opengl").def("snapshot_depth_ms", (void (*)(const std::string &)) &easy3d::opengl::snapshot_depth_ms, "C++: easy3d::opengl::snapshot_depth_ms(const std::string &) --> void", pybind11::arg("file_name"));

}
