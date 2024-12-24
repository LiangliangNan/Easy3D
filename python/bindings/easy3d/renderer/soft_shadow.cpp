#include <easy3d/core/mat.h>
#include <easy3d/core/model.h>
#include <easy3d/core/quat.h>
#include <easy3d/core/vec.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/frame.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/shadow.h>
#include <easy3d/renderer/soft_shadow.h>
#include <easy3d/renderer/text_renderer.h>
#include <easy3d/renderer/texture.h>
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

// easy3d::SoftShadow file:easy3d/renderer/soft_shadow.h line:69
struct PyCallBack_easy3d_SoftShadow : public easy3d::SoftShadow {
	using easy3d::SoftShadow::SoftShadow;

	void ensure_fbo() override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::SoftShadow *>(this), "ensure_fbo");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return SoftShadow::ensure_fbo();
	}
	void shadow_map_pass(const class std::vector<class easy3d::TrianglesDrawable *> & a0) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::SoftShadow *>(this), "shadow_map_pass");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return SoftShadow::shadow_map_pass(a0);
	}
	void render_pass(const class std::vector<class easy3d::TrianglesDrawable *> & a0) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::SoftShadow *>(this), "render_pass");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return SoftShadow::render_pass(a0);
	}
	void update_virtual_background() override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::SoftShadow *>(this), "update_virtual_background");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return Shadow::update_virtual_background();
	}
};

void bind_easy3d_renderer_soft_shadow(pybind11::module_& m)
{
	{ // easy3d::SoftShadow file:easy3d/renderer/soft_shadow.h line:69
		pybind11::class_<easy3d::SoftShadow, std::shared_ptr<easy3d::SoftShadow>, PyCallBack_easy3d_SoftShadow, easy3d::Shadow> cl(m, "SoftShadow", "An implementation of the Percentage-Closer Soft Shadows\n\n \n\n \n SoftShadow implements the Percentage-Closer Soft Shadows technique. See\n  - http://developer.download.nvidia.com/shaderlibrary/docs/shadow_PCSS.pdf\n  - https://http.download.nvidia.com/developer/presentations/2005/SIGGRAPH/Percentage_Closer_Soft_Shadows.pdf\n\n --- Overview ---\n This sample demonstrates two techniques for rendering soft shadows: Percentage Closer Filtering (PCS) with\n a uniform kernel size and Percentage-Closer Soft Shadows (PCSS) with a variable kernel size.\n This sample is assuming the shadow map is a regular depth buffer (non-linear depths), and the shadow-map\n is generated using a perspective projection with the center of projection at the center of the area light\n simulated by PCSS. The sample uses a gradient-based depth bias to scale the depth bias based on the uv-space\n distance from the center of the kernel. To avoid self-shadowing artifacts, an additional depth bias may also\n been applied while writing depth into the shadow map.\n\n --- Percentage Closer Filtering (PCF) ---\n The Percentage Closer Filtering technique uses a uniform size filter kernel, resulting in shadows with a\n uniformly soft edge. The PCF algorithm is very straightforward and fast, but does not achieve a realistic\n contact hardening effect. Also, while the kernel size and penumbra width is uniform, the kernel can be\n scaled to accommodate different sized lights.\n\n --- Percentage-Closer Soft Shadows (PCSS) ---\n Soft shadows are typically rendered in games by using shadow mapping and Percentage Closer Filtering with\n a uniform kernel size. The Percentage-Closer Soft Shadows (PCSS) algorithm computes a variable kernel size\n based on the distance between the relative position of the receiver point, an approximation of the blocker,\n and the area light. It produces perceptually plausible soft shadows that harden on contact.\n\n Optimization tips: rendering with multi-effects (e.g., shadowing, SSAO) can benefit from sharing the same\n geometry pass.");
		cl.def( pybind11::init<class easy3d::Camera *>(), pybind11::arg("cam") );


		pybind11::enum_<easy3d::SoftShadow::SamplePattern>(cl, "SamplePattern", pybind11::arithmetic(), "")
			.value("SP_Poisson_25_25", easy3d::SoftShadow::SP_Poisson_25_25)
			.value("SP_Poisson_32_64", easy3d::SoftShadow::SP_Poisson_32_64)
			.value("SP_Poisson_64_128", easy3d::SoftShadow::SP_Poisson_64_128)
			.value("SP_Poisson_100_100", easy3d::SoftShadow::SP_Poisson_100_100)
			.value("SP_Regular_49_225", easy3d::SoftShadow::SP_Regular_49_225)
			.export_values();

		cl.def("softness", (float (easy3d::SoftShadow::*)() const) &easy3d::SoftShadow::softness, "C++: easy3d::SoftShadow::softness() const --> float");
		cl.def("set_softness", (void (easy3d::SoftShadow::*)(float)) &easy3d::SoftShadow::set_softness, "C++: easy3d::SoftShadow::set_softness(float) --> void", pybind11::arg("s"));
		cl.def("sample_pattern", (enum easy3d::SoftShadow::SamplePattern (easy3d::SoftShadow::*)() const) &easy3d::SoftShadow::sample_pattern, "C++: easy3d::SoftShadow::sample_pattern() const --> enum easy3d::SoftShadow::SamplePattern");
		cl.def("set_sample_pattern", (void (easy3d::SoftShadow::*)(enum easy3d::SoftShadow::SamplePattern)) &easy3d::SoftShadow::set_sample_pattern, "C++: easy3d::SoftShadow::set_sample_pattern(enum easy3d::SoftShadow::SamplePattern) --> void", pybind11::arg("pattern"));
	}
	{ // easy3d::TextRenderer file:easy3d/renderer/text_renderer.h line:53
		pybind11::class_<easy3d::TextRenderer, std::shared_ptr<easy3d::TextRenderer>> cl(m, "TextRenderer", "TextRenderer enables quick and easy string rendering in OpenGL applications.\n\n \n\n \n It supports truetype (TTF) fonts and unicode strings. All glyphs are cached in OpenGL textures and\n      font rasterization is done using 'stb_truetype.h.\"\n      TextRenderer allows you to:\n          - switch between fonts;\n          - change character spacing;\n          - enable/disable kerning;\n \n\n For a large number of strings/characters, it is necessary to separate buffer creation and rendering.\n \n\n Performance evaluation.");
		cl.def( pybind11::init( [](){ return new easy3d::TextRenderer(); } ), "doc" );
		cl.def( pybind11::init( [](float const & a0){ return new easy3d::TextRenderer(a0); } ), "doc" , pybind11::arg("dpi_scale"));
		cl.def( pybind11::init( [](float const & a0, int const & a1){ return new easy3d::TextRenderer(a0, a1); } ), "doc" , pybind11::arg("dpi_scale"), pybind11::arg("texture_size"));
		cl.def( pybind11::init<float, int, bool>(), pybind11::arg("dpi_scale"), pybind11::arg("texture_size"), pybind11::arg("mipmaps") );

		cl.def( pybind11::init( [](easy3d::TextRenderer const &o){ return new easy3d::TextRenderer(o); } ) );

		pybind11::enum_<easy3d::TextRenderer::Align>(cl, "Align", pybind11::arithmetic(), "Horizontal alignment. *")
			.value("ALIGN_LEFT", easy3d::TextRenderer::ALIGN_LEFT)
			.value("ALIGN_RIGHT", easy3d::TextRenderer::ALIGN_RIGHT)
			.value("ALIGN_CENTER", easy3d::TextRenderer::ALIGN_CENTER)
			.export_values();

		cl.def("add_font", (bool (easy3d::TextRenderer::*)(const std::string &)) &easy3d::TextRenderer::add_font, "Add a font from a file. This function must be called at least once to add a font.\n \n\n The font file (.ttf, etc).\n\nC++: easy3d::TextRenderer::add_font(const std::string &) --> bool", pybind11::arg("font_file"));
		cl.def("num_fonts", (std::size_t (easy3d::TextRenderer::*)() const) &easy3d::TextRenderer::num_fonts, "The number of available fonts.\n\nC++: easy3d::TextRenderer::num_fonts() const --> std::size_t");
		cl.def("font_names", (const class std::vector<std::string> & (easy3d::TextRenderer::*)() const) &easy3d::TextRenderer::font_names, "The names of available fonts.\n\nC++: easy3d::TextRenderer::font_names() const --> const class std::vector<std::string> &", pybind11::return_value_policy::automatic);
		cl.def("draw", [](easy3d::TextRenderer const &o, const std::string & a0, float const & a1, float const & a2, float const & a3) -> float { return o.draw(a0, a1, a2, a3); }, "", pybind11::arg("text"), pybind11::arg("x"), pybind11::arg("y"), pybind11::arg("font_size"));
		cl.def("draw", [](easy3d::TextRenderer const &o, const std::string & a0, float const & a1, float const & a2, float const & a3, int const & a4) -> float { return o.draw(a0, a1, a2, a3, a4); }, "", pybind11::arg("text"), pybind11::arg("x"), pybind11::arg("y"), pybind11::arg("font_size"), pybind11::arg("font_id"));
		cl.def("draw", [](easy3d::TextRenderer const &o, const std::string & a0, float const & a1, float const & a2, float const & a3, int const & a4, const class easy3d::Vec<3, float> & a5) -> float { return o.draw(a0, a1, a2, a3, a4, a5); }, "", pybind11::arg("text"), pybind11::arg("x"), pybind11::arg("y"), pybind11::arg("font_size"), pybind11::arg("font_id"), pybind11::arg("font_color"));
		cl.def("draw", (float (easy3d::TextRenderer::*)(const std::string &, float, float, float, int, const class easy3d::Vec<3, float> &, bool) const) &easy3d::TextRenderer::draw, "Draw the text.\n \n\n The string to be drawn.\n \n\n The x-coordinate of the starting position, relative to the left edge of the content area.\n \n\n The y-coordinate of the starting position. If upper_left is true, this position is relative to the\n        top edge of the content area. Otherwise, it is relative to the bottom edge of the content area.\n \n\n The font size.\n \n\n The id of the font used to draw the text, which is generated by calling to add_font(). 0 is\n        the first font, 1 is the second, etc.\n \n\n The font color.\n \n\n True to define the origin at the upper left corner of the content area. Otherwise it is\n        defined at the bottom left corner of the content area.\n \n\n The end position of the x-coordinate. This allows you to draw another string after the previous one.\n\n \n: This function will draw the text in one line and thus line breaks (i.e., \"\\n\") will be ignored.\n\nC++: easy3d::TextRenderer::draw(const std::string &, float, float, float, int, const class easy3d::Vec<3, float> &, bool) const --> float", pybind11::arg("text"), pybind11::arg("x"), pybind11::arg("y"), pybind11::arg("font_size"), pybind11::arg("font_id"), pybind11::arg("font_color"), pybind11::arg("upper_left"));
		cl.def("set_character_spacing", (void (easy3d::TextRenderer::*)(float)) &easy3d::TextRenderer::set_character_spacing, "Set the spacing between consecutive characters. The default character spacing is 0.\n \n\n The expected character spacing.\n\nC++: easy3d::TextRenderer::set_character_spacing(float) --> void", pybind11::arg("spacing"));
		cl.def("character_spacing", (float (easy3d::TextRenderer::*)() const) &easy3d::TextRenderer::character_spacing, "Query the spacing between consecutive characters.\n \n\n The spacing between consecutive characters.\n\nC++: easy3d::TextRenderer::character_spacing() const --> float");
		cl.def("set_kerning", (void (easy3d::TextRenderer::*)(bool)) &easy3d::TextRenderer::set_kerning, "Set kerning.\n \n\n The expected kerning behavior.\n\nC++: easy3d::TextRenderer::set_kerning(bool) --> void", pybind11::arg("kerning"));
		cl.def("kerning", (bool (easy3d::TextRenderer::*)() const) &easy3d::TextRenderer::kerning, "Query the kerning behavior. Kerning is disabled by default.\n \n\n The kerning behavior.\n\nC++: easy3d::TextRenderer::kerning() const --> bool");
		cl.def("font_height", (float (easy3d::TextRenderer::*)(float) const) &easy3d::TextRenderer::font_height, "Query the height of the font.\n \n\n The font size.\n \n\n The height of the font in pixels.\n\nC++: easy3d::TextRenderer::font_height(float) const --> float", pybind11::arg("font_size"));
		cl.def("string_width", (float (easy3d::TextRenderer::*)(const std::string &, float) const) &easy3d::TextRenderer::string_width, "Query the width of a string (treated as if with no space between characters). Useful to position a string.\n For example, with the string width, its X-center can be computed as: viewer_width * 0.5 - string_width * 0.5.\n \n\n The font size.\n \n\n The width of the string in pixels.\n\nC++: easy3d::TextRenderer::string_width(const std::string &, float) const --> float", pybind11::arg("str"), pybind11::arg("font_size"));
		cl.def("string_height", (float (easy3d::TextRenderer::*)(const std::string &, float) const) &easy3d::TextRenderer::string_height, "Query the height of a string.\n \n\n The font size.\n \n\n The height of the string in pixels.\n\nC++: easy3d::TextRenderer::string_height(const std::string &, float) const --> float", pybind11::arg("str"), pybind11::arg("font_size"));
		cl.def("assign", (class easy3d::TextRenderer & (easy3d::TextRenderer::*)(const class easy3d::TextRenderer &)) &easy3d::TextRenderer::operator=, "C++: easy3d::TextRenderer::operator=(const class easy3d::TextRenderer &) --> class easy3d::TextRenderer &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}
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
