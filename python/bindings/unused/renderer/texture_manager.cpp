#include <easy3d/renderer/texture.h>
#include <easy3d/renderer/texture_manager.h>
#include <memory>
#include <sstream> // __str__
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

void bind_easy3d_renderer_texture_manager(pybind11::module_& m)
{
	{ // easy3d::TextureManager file:easy3d/renderer/texture_manager.h line:53
		pybind11::class_<easy3d::TextureManager, std::shared_ptr<easy3d::TextureManager>> cl(m, "TextureManager", "Management of OpenGL textures.\n\n \n\n \n A texture can be shared by multiple models, so it is wise to have a single instance of a texture.\n TextureManager manages the texture creation and memories. It ensures no duplication.\n\n \n Make sure to call terminate() to destroy existing textures before the OpenGL context is deleted.\n\n \n Allow to change the wrap/filter mode (i.e., multiple textures of different parameters from the same image).");
		cl.def( pybind11::init( [](){ return new easy3d::TextureManager(); } ) );
		cl.def_static("request", [](const std::string & a0) -> easy3d::Texture * { return easy3d::TextureManager::request(a0); }, "", pybind11::return_value_policy::automatic, pybind11::arg("image_file"));
		cl.def_static("request", [](const std::string & a0, enum easy3d::Texture::WrapMode const & a1) -> easy3d::Texture * { return easy3d::TextureManager::request(a0, a1); }, "", pybind11::return_value_policy::automatic, pybind11::arg("image_file"), pybind11::arg("wrap"));
		cl.def_static("request", (class easy3d::Texture * (*)(const std::string &, enum easy3d::Texture::WrapMode, enum easy3d::Texture::FilterMode)) &easy3d::TextureManager::request, "Request a texture from the image file.\n \n\n The generated texture has a unique identifier of its full-path file name. If a texture with the same\n          identifier is requested again, no new texture will be generated and the existing texture is returned.\n \n\n The full path to the image file.\n \n\n The wrap mode.\n \n\n The filter mode.\n \n\n The texture created from the image.\n\nC++: easy3d::TextureManager::request(const std::string &, enum easy3d::Texture::WrapMode, enum easy3d::Texture::FilterMode) --> class easy3d::Texture *", pybind11::return_value_policy::automatic, pybind11::arg("image_file"), pybind11::arg("wrap"), pybind11::arg("filter"));
		cl.def_static("request", [](const std::string & a0, int const & a1) -> easy3d::Texture * { return easy3d::TextureManager::request(a0, a1); }, "", pybind11::return_value_policy::automatic, pybind11::arg("image_file"), pybind11::arg("num_stripes"));
		cl.def_static("request", [](const std::string & a0, int const & a1, enum easy3d::Texture::WrapMode const & a2) -> easy3d::Texture * { return easy3d::TextureManager::request(a0, a1, a2); }, "", pybind11::return_value_policy::automatic, pybind11::arg("image_file"), pybind11::arg("num_stripes"), pybind11::arg("wrap"));
		cl.def_static("request", (class easy3d::Texture * (*)(const std::string &, int, enum easy3d::Texture::WrapMode, enum easy3d::Texture::FilterMode)) &easy3d::TextureManager::request, "Request a texture for rendering scalar fields from a gradually varying (from left to right) color\n        image given its file name.\n \n\n This method allows to sample the original image into a specified number of vertical stripes. The\n          generated texture has a unique identifier in the format: \"file_name|num_stripes\". If a texture with\n          the same identifier is requested again, no new texture will be generated and the existing one is\n          returned.\n \n\n The full path to the image file.\n \n\n The number of stripes. If num_stripes == image_width, no sampling will occur.\n \n\n The wrap mode.\n \n\n The filter mode.\n \n\n The texture created from the image.\n\nC++: easy3d::TextureManager::request(const std::string &, int, enum easy3d::Texture::WrapMode, enum easy3d::Texture::FilterMode) --> class easy3d::Texture *", pybind11::return_value_policy::automatic, pybind11::arg("image_file"), pybind11::arg("num_stripes"), pybind11::arg("wrap"), pybind11::arg("filter"));
		cl.def_static("request", [](int const & a0, int const & a1) -> easy3d::Texture * { return easy3d::TextureManager::request(a0, a1); }, "", pybind11::return_value_policy::automatic, pybind11::arg("num_stripes"), pybind11::arg("stride"));
		cl.def_static("request", [](int const & a0, int const & a1, enum easy3d::Texture::WrapMode const & a2) -> easy3d::Texture * { return easy3d::TextureManager::request(a0, a1, a2); }, "", pybind11::return_value_policy::automatic, pybind11::arg("num_stripes"), pybind11::arg("stride"), pybind11::arg("wrap"));
		cl.def_static("request", (class easy3d::Texture * (*)(int, int, enum easy3d::Texture::WrapMode, enum easy3d::Texture::FilterMode)) &easy3d::TextureManager::request, "Request a random color texture for rendering scalar fields. The texture will have a set of uniformly\n        colored vertical stripes.\n \n\n It first generates a random-colored image, where each row has \"num_colors\" color and each color\n          repeat \"stride\" times. Then, a texture is created from this image data. The generated texture has a\n          unique identifier in the format: \"random|num_stripes|stride\", where num_stripes and stride are\n          integer numbers. If the requested number of stripes and the stride match a previously generated\n          texture, no new texture will be generated and the existing texture is returned.\n \n\n The number of stripes in the texture.\n \n\n The width of each vertical strip. So the total width of the texture is: num_stripes * stride.\n \n\n The texture created.\n\nC++: easy3d::TextureManager::request(int, int, enum easy3d::Texture::WrapMode, enum easy3d::Texture::FilterMode) --> class easy3d::Texture *", pybind11::return_value_policy::automatic, pybind11::arg("num_stripes"), pybind11::arg("stride"), pybind11::arg("wrap"), pybind11::arg("filter"));
		cl.def_static("release", (void (*)(const class easy3d::Texture *)) &easy3d::TextureManager::release, "Release a texture (deallocate its memory).\n \n\n The texture to be released.\n\nC++: easy3d::TextureManager::release(const class easy3d::Texture *) --> void", pybind11::arg("texture"));
		cl.def_static("terminate", (void (*)()) &easy3d::TextureManager::terminate, "destroy all textures.\n\nC++: easy3d::TextureManager::terminate() --> void");
	}
}
