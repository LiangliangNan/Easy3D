#include <easy3d/renderer/texture.h>
#include <memory>
#include <sstream> // __str__
#include <string>
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

void bind_easy3d_renderer_texture(pybind11::module_& m)
{
	{ // easy3d::Texture file:easy3d/renderer/texture.h line:44
		pybind11::class_<easy3d::Texture, std::shared_ptr<easy3d::Texture>> cl(m, "Texture", "OpenGL texture.\n\n \n\n \n currently only TEXTURE_2D is supported");

		pybind11::enum_<easy3d::Texture::WrapMode>(cl, "WrapMode", pybind11::arithmetic(), "")
			.value("CLAMP_TO_EDGE", easy3d::Texture::CLAMP_TO_EDGE)
			.value("REPEAT", easy3d::Texture::REPEAT)
			.export_values();


		pybind11::enum_<easy3d::Texture::FilterMode>(cl, "FilterMode", pybind11::arithmetic(), "")
			.value("NEAREST", easy3d::Texture::NEAREST)
			.value("LINEAR", easy3d::Texture::LINEAR)
			.export_values();

		cl.def_static("create", [](const std::string & a0) -> easy3d::Texture * { return easy3d::Texture::create(a0); }, "", pybind11::return_value_policy::automatic, pybind11::arg("image_file"));
		cl.def_static("create", [](const std::string & a0, enum easy3d::Texture::WrapMode const & a1) -> easy3d::Texture * { return easy3d::Texture::create(a0, a1); }, "", pybind11::return_value_policy::automatic, pybind11::arg("image_file"), pybind11::arg("wrap"));
		cl.def_static("create", (class easy3d::Texture * (*)(const std::string &, enum easy3d::Texture::WrapMode, enum easy3d::Texture::FilterMode)) &easy3d::Texture::create, "Creates a texture from an image file.\n \n\n\n \n The created texture.\n\nC++: easy3d::Texture::create(const std::string &, enum easy3d::Texture::WrapMode, enum easy3d::Texture::FilterMode) --> class easy3d::Texture *", pybind11::return_value_policy::automatic, pybind11::arg("image_file"), pybind11::arg("wrap"), pybind11::arg("filter"));
		cl.def_static("create", [](const class std::vector<unsigned char> & a0, int const & a1, int const & a2, int const & a3) -> easy3d::Texture * { return easy3d::Texture::create(a0, a1, a2, a3); }, "", pybind11::return_value_policy::automatic, pybind11::arg("rgb_data"), pybind11::arg("width"), pybind11::arg("height"), pybind11::arg("comp"));
		cl.def_static("create", [](const class std::vector<unsigned char> & a0, int const & a1, int const & a2, int const & a3, enum easy3d::Texture::WrapMode const & a4) -> easy3d::Texture * { return easy3d::Texture::create(a0, a1, a2, a3, a4); }, "", pybind11::return_value_policy::automatic, pybind11::arg("rgb_data"), pybind11::arg("width"), pybind11::arg("height"), pybind11::arg("comp"), pybind11::arg("wrap"));
		cl.def_static("create", (class easy3d::Texture * (*)(const class std::vector<unsigned char> &, int, int, int, enum easy3d::Texture::WrapMode, enum easy3d::Texture::FilterMode)) &easy3d::Texture::create, "Creates a texture from the given image data.\n \n\n The image data.\n \n\n The width (i.e., number of columns) of the image.\n \n\n The rows (i.e., number of rows) of the image.\n \n\n The number of components for each pixel (e.g., 3 for RGB)\n \n\n The created texture.\n\nC++: easy3d::Texture::create(const class std::vector<unsigned char> &, int, int, int, enum easy3d::Texture::WrapMode, enum easy3d::Texture::FilterMode) --> class easy3d::Texture *", pybind11::return_value_policy::automatic, pybind11::arg("rgb_data"), pybind11::arg("width"), pybind11::arg("height"), pybind11::arg("comp"), pybind11::arg("wrap"), pybind11::arg("filter"));
		cl.def("id", (unsigned int (easy3d::Texture::*)() const) &easy3d::Texture::id, "C++: easy3d::Texture::id() const --> unsigned int");
		cl.def("name", (const std::string & (easy3d::Texture::*)() const) &easy3d::Texture::name, "C++: easy3d::Texture::name() const --> const std::string &", pybind11::return_value_policy::automatic);
		cl.def("bind", [](easy3d::Texture const &o) -> void { return o.bind(); }, "");
		cl.def("bind", (void (easy3d::Texture::*)(int) const) &easy3d::Texture::bind, "C++: easy3d::Texture::bind(int) const --> void", pybind11::arg("unit"));
		cl.def("release", (void (easy3d::Texture::*)() const) &easy3d::Texture::release, "C++: easy3d::Texture::release() const --> void");
		cl.def("width", (int (easy3d::Texture::*)() const) &easy3d::Texture::width, "C++: easy3d::Texture::width() const --> int");
		cl.def("height", (int (easy3d::Texture::*)() const) &easy3d::Texture::height, "C++: easy3d::Texture::height() const --> int");
		cl.def("channels", (int (easy3d::Texture::*)() const) &easy3d::Texture::channels, "C++: easy3d::Texture::channels() const --> int");
		cl.def("wrap_mode", (enum easy3d::Texture::WrapMode (easy3d::Texture::*)() const) &easy3d::Texture::wrap_mode, "C++: easy3d::Texture::wrap_mode() const --> enum easy3d::Texture::WrapMode");
		cl.def("filter_mode", (enum easy3d::Texture::FilterMode (easy3d::Texture::*)() const) &easy3d::Texture::filter_mode, "C++: easy3d::Texture::filter_mode() const --> enum easy3d::Texture::FilterMode");
	}
	// easy3d::discretize_image(class std::vector<unsigned char> &, int, int, int, int) file:easy3d/renderer/texture.h line:122
	M("easy3d").def("discretize_image", (void (*)(class std::vector<unsigned char> &, int, int, int, int)) &easy3d::discretize_image, "Discretize a gradually varying-color (from left to right) image into a set of uniform colored vertical\n        stripes.\n \n\n The input data to be discretized.\n \n\n The width of the image (i.e., number of pixels in a row).\n \n\n The height of the image (i.e., number of pixels in a column).\n \n\n The number or color component per pixel.\n \n\n The number of stripes. The image will remain unmodified if num_stripes >= image width.\n\nC++: easy3d::discretize_image(class std::vector<unsigned char> &, int, int, int, int) --> void", pybind11::arg("data"), pybind11::arg("width"), pybind11::arg("height"), pybind11::arg("channels"), pybind11::arg("num_stripes"));

}
