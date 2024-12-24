#include <easy3d/core/line.h>
#include <easy3d/core/mat.h>
#include <easy3d/core/quat.h>
#include <easy3d/core/vec.h>
#include <easy3d/gui/picker.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/frame.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/state.h>
#include <easy3d/renderer/texture.h>
#include <memory>
#include <sstream> // __str__
#include <string>
#include <string_view>
#include <utility>
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

void bind_easy3d_gui_picker(pybind11::module_& m)
{
	{ // easy3d::Picker file:easy3d/gui/picker.h line:43
		pybind11::class_<easy3d::Picker, std::shared_ptr<easy3d::Picker>> cl(m, "Picker", "Base class for picking mechanism.\n \n\n\n \n ModelPicker, PointCloudPicker, SurfaceMeshPicker");
		cl.def( pybind11::init<const class easy3d::Camera *>(), pybind11::arg("cam") );

		cl.def( pybind11::init( [](easy3d::Picker const &o){ return new easy3d::Picker(o); } ) );
		cl.def("camera", (const class easy3d::Camera * (easy3d::Picker::*)() const) &easy3d::Picker::camera, "Returns the pointer of the camera.\n\nC++: easy3d::Picker::camera() const --> const class easy3d::Camera *", pybind11::return_value_policy::automatic);
		cl.def("picking_line", (class easy3d::GenericLine<3, float> (easy3d::Picker::*)(int, int) const) &easy3d::Picker::picking_line, "Construct a picking line.\n \n\n The cursor x-coordinate, relative to the left edge of the content area.\n \n\n The cursor y-coordinate, relative to the top edge of the content area.\n \n\n The screen point is expressed in the screen coordinate system with an origin in the upper left\n            corner. So it doesn't necessarily correspond to a pixel on High DPI devices, e.g. a Mac with\n            a Retina display.\n\nC++: easy3d::Picker::picking_line(int, int) const --> class easy3d::GenericLine<3, float>", pybind11::arg("x"), pybind11::arg("y"));
		cl.def("picking_dir", (class easy3d::Vec<3, float> (easy3d::Picker::*)(int, int) const) &easy3d::Picker::picking_dir, "The picking direction, pointing inside the screen.\n \n\n The cursor x-coordinate, relative to the left edge of the content area.\n \n\n The cursor y-coordinate, relative to the top edge of the content area.\n \n\n The screen point is expressed in the screen coordinate system with an origin in the upper left\n            corner. So it doesn't necessarily correspond to a pixel on High DPI devices, e.g. a Mac with\n            a Retina display.\n\nC++: easy3d::Picker::picking_dir(int, int) const --> class easy3d::Vec<3, float>", pybind11::arg("x"), pybind11::arg("y"));
		cl.def("project", (class easy3d::Vec<3, float> (easy3d::Picker::*)(const class easy3d::Vec<3, float> &) const) &easy3d::Picker::project, "Project a 3D point in the world coordinate system onto the 2D screen coordinate system.\n \n\n A 3D point in the world coordinate system.\n \n\n The x and y components of the returned value denote the projected screen point expressed in the\n         screen coordinate system, with (0, 0) being the upper left corner of the content area. The z\n         component ranges between 0.0 (near plane) and 1.0 (excluded, far plane).\n \n\n The screen point is expressed in the screen coordinate system with an origin in the upper left\n            corner. So it doesn't necessarily correspond to a pixel on High DPI devices, e.g. a Mac with\n            a Retina display.\n\nC++: easy3d::Picker::project(const class easy3d::Vec<3, float> &) const --> class easy3d::Vec<3, float>", pybind11::arg("p"));
		cl.def("unproject", (class easy3d::Vec<3, float> (easy3d::Picker::*)(int, int, float) const) &easy3d::Picker::unproject, "Compute the world coordinates of a point defined in the screen coordinate system.\n \n\n The cursor x-coordinate, relative to the left edge of the content area.\n \n\n The cursor y-coordinate, relative to the top edge of the content area.\n \n\n The depth value of the screen point, ranging between 0.0 and 1.0 (excluded).\n \n\n The world unprojected coordinates of the screen point.\n \n\n The screen point is expressed in the screen coordinate system with an origin in the upper left\n            corner. So it doesn't necessarily correspond to a pixel on High DPI devices, e.g. a Mac with\n            a Retina display.\n\nC++: easy3d::Picker::unproject(int, int, float) const --> class easy3d::Vec<3, float>", pybind11::arg("x"), pybind11::arg("y"), pybind11::arg("depth"));
		cl.def("screen_to_opengl", (void (easy3d::Picker::*)(int, int, int &, int &, int, int) const) &easy3d::Picker::screen_to_opengl, "Convert a point expressed in the screen coordinate system (with an origin in the upper left corner)\n into the OpenGL coordinate system (with an origin in the lower left corner). The high DPI scaling is also\n taken into consideration, so the result always corresponds to its image pixel.\n \n\n The x-coordinate, relative to the left edge of the content area.\n \n\n The y-coordinate, relative to the top edge of the content area.\n \n\n Returns the x component of the point in the OpenGL coordinate system.\n \n\n Returns the y component of the point in the OpenGL coordinate system.\n \n\n The width of the OpenGL viewport (may not be identical to the width of the screen in pixels).\n \n\n The width of the OpenGL viewport (may not be identical to the height of the screen in pixels).\n\nC++: easy3d::Picker::screen_to_opengl(int, int, int &, int &, int, int) const --> void", pybind11::arg("x"), pybind11::arg("y"), pybind11::arg("gl_x"), pybind11::arg("gl_y"), pybind11::arg("width"), pybind11::arg("height"));
		cl.def("assign", (class easy3d::Picker & (easy3d::Picker::*)(const class easy3d::Picker &)) &easy3d::Picker::operator=, "C++: easy3d::Picker::operator=(const class easy3d::Picker &) --> class easy3d::Picker &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}
	{ // easy3d::State file:easy3d/renderer/state.h line:45
		pybind11::class_<easy3d::State, std::shared_ptr<easy3d::State>> cl(m, "State", "Class representing the rendering state of a drawable\n\n \n\n     ");
		cl.def( pybind11::init( [](){ return new easy3d::State(); } ) );
		cl.def( pybind11::init( [](easy3d::State const &o){ return new easy3d::State(o); } ) );

		pybind11::enum_<easy3d::State::Method>(cl, "Method", pybind11::arithmetic(), "A complete description of a color scheme contains the coloring method, and (if not uniform color) the\n location and name of a property.\n Available coloring methods:\n   - UNIFORM_COLOR: uniformly colored;\n   - COLOR_PROPERTY: using a color property;\n   - SCALAR_FIELD: using a scalar field;\n   - TEXTURED: using texture(s).\n Possible properties for coloring a drawable:\n   - color properties defined on vertices, faces, or edges. A color property is specified by its name,\n     e.g., \"v:color\", \"f:color\", \"e:color\";\n   - scalar fields defined on vertices, faces, or edges. A scalar field is specified by its name, e.g.,\n     \"v:curvature\", \"v:height\", \"f:segment_id\";\n   - texture coordinates defined on vertices or halfedges. A texture coordinates property is specified by its\n     name, e.g., \"v:texcoord\", \"h:texcoord\".")
			.value("UNIFORM_COLOR", easy3d::State::UNIFORM_COLOR)
			.value("COLOR_PROPERTY", easy3d::State::COLOR_PROPERTY)
			.value("SCALAR_FIELD", easy3d::State::SCALAR_FIELD)
			.value("TEXTURED", easy3d::State::TEXTURED)
			.export_values();


		pybind11::enum_<easy3d::State::Location>(cl, "Location", pybind11::arithmetic(), "The location of a coloring property. It is denoted by the type of the geometric primitives on which the\n property is defined.")
			.value("VERTEX", easy3d::State::VERTEX)
			.value("FACE", easy3d::State::FACE)
			.value("EDGE", easy3d::State::EDGE)
			.value("HALFEDGE", easy3d::State::HALFEDGE)
			.export_values();

		cl.def("assign", (class easy3d::State & (easy3d::State::*)(const class easy3d::State &)) &easy3d::State::operator=, "assign  to  performs a deep copy of all member variables.\n\nC++: easy3d::State::operator=(const class easy3d::State &) --> class easy3d::State &", pybind11::return_value_policy::automatic, pybind11::arg("rhs"));
		cl.def("is_visible", (bool (easy3d::State::*)() const) &easy3d::State::is_visible, "C++: easy3d::State::is_visible() const --> bool");
		cl.def("set_visible", (void (easy3d::State::*)(bool)) &easy3d::State::set_visible, "C++: easy3d::State::set_visible(bool) --> void", pybind11::arg("v"));
		cl.def("is_selected", (bool (easy3d::State::*)() const) &easy3d::State::is_selected, "C++: easy3d::State::is_selected() const --> bool");
		cl.def("set_selected", (void (easy3d::State::*)(bool)) &easy3d::State::set_selected, "C++: easy3d::State::set_selected(bool) --> void", pybind11::arg("b"));
		cl.def("set_uniform_coloring", (void (easy3d::State::*)(const class easy3d::Vec<4, float> &)) &easy3d::State::set_uniform_coloring, "Constructs a uniform coloring scheme.\n \n\n The color.\n\nC++: easy3d::State::set_uniform_coloring(const class easy3d::Vec<4, float> &) --> void", pybind11::arg("color"));
		cl.def("set_property_coloring", [](easy3d::State &o, enum easy3d::State::Location const & a0) -> void { return o.set_property_coloring(a0); }, "", pybind11::arg("color_location"));
		cl.def("set_property_coloring", (void (easy3d::State::*)(enum easy3d::State::Location, const std::string &)) &easy3d::State::set_property_coloring, "Constructs a scheme for rendering a drawable with per-element color.\n \n\n The location of the color property.\n \n\n The name of the color property.\n\nC++: easy3d::State::set_property_coloring(enum easy3d::State::Location, const std::string &) --> void", pybind11::arg("color_location"), pybind11::arg("color_name"));
		cl.def("set_texture_coloring", [](easy3d::State &o, enum easy3d::State::Location const & a0, const std::string & a1) -> void { return o.set_texture_coloring(a0, a1); }, "", pybind11::arg("texcoord_location"), pybind11::arg("texcoord_name"));
		cl.def("set_texture_coloring", [](easy3d::State &o, enum easy3d::State::Location const & a0, const std::string & a1, const class easy3d::Texture * a2) -> void { return o.set_texture_coloring(a0, a1, a2); }, "", pybind11::arg("texcoord_location"), pybind11::arg("texcoord_name"), pybind11::arg("texture"));
		cl.def("set_texture_coloring", [](easy3d::State &o, enum easy3d::State::Location const & a0, const std::string & a1, const class easy3d::Texture * a2, float const & a3) -> void { return o.set_texture_coloring(a0, a1, a2, a3); }, "", pybind11::arg("texcoord_location"), pybind11::arg("texcoord_name"), pybind11::arg("texture"), pybind11::arg("repeat"));
		cl.def("set_texture_coloring", (void (easy3d::State::*)(enum easy3d::State::Location, const std::string &, const class easy3d::Texture *, float, float)) &easy3d::State::set_texture_coloring, "Constructs a scheme for textured rendering.\n \n\n The location of texture coordinate.\n \n\n The name of the texture coordinate property.\n \n\n The pointer to the texture.\n \n\n The repeat factor of the texture. Default value is 1.0.\n \n\n The fractional repeat factor of the texture. Default value is 0.0.\n\nC++: easy3d::State::set_texture_coloring(enum easy3d::State::Location, const std::string &, const class easy3d::Texture *, float, float) --> void", pybind11::arg("texcoord_location"), pybind11::arg("texcoord_name"), pybind11::arg("texture"), pybind11::arg("repeat"), pybind11::arg("repeat_fraction"));
		cl.def("set_scalar_coloring", [](easy3d::State &o, enum easy3d::State::Location const & a0, const std::string & a1) -> void { return o.set_scalar_coloring(a0, a1); }, "", pybind11::arg("scalar_location"), pybind11::arg("scalar_name"));
		cl.def("set_scalar_coloring", [](easy3d::State &o, enum easy3d::State::Location const & a0, const std::string & a1, const class easy3d::Texture * a2) -> void { return o.set_scalar_coloring(a0, a1, a2); }, "", pybind11::arg("scalar_location"), pybind11::arg("scalar_name"), pybind11::arg("texture"));
		cl.def("set_scalar_coloring", [](easy3d::State &o, enum easy3d::State::Location const & a0, const std::string & a1, const class easy3d::Texture * a2, float const & a3) -> void { return o.set_scalar_coloring(a0, a1, a2, a3); }, "", pybind11::arg("scalar_location"), pybind11::arg("scalar_name"), pybind11::arg("texture"), pybind11::arg("clamp_lower"));
		cl.def("set_scalar_coloring", (void (easy3d::State::*)(enum easy3d::State::Location, const std::string &, const class easy3d::Texture *, float, float)) &easy3d::State::set_scalar_coloring, "Constructs a scheme for rendering scalar fields.\n \n\n The location of the scalar field.\n \n\n The name of the scalar field.\n \n\n The pointer to the texture.\n \n\n The percentage of values to be clamped at the lower side of the range. Default is 5%.\n \n\n The percentage of values to be clamped at the upper side of the range. Default is 5%.\n\nC++: easy3d::State::set_scalar_coloring(enum easy3d::State::Location, const std::string &, const class easy3d::Texture *, float, float) --> void", pybind11::arg("scalar_location"), pybind11::arg("scalar_name"), pybind11::arg("texture"), pybind11::arg("clamp_lower"), pybind11::arg("clamp_upper"));
		cl.def("set_coloring", (void (easy3d::State::*)(enum easy3d::State::Method, enum easy3d::State::Location, const std::string &)) &easy3d::State::set_coloring, "Sets the coloring. A generic version of the set_[method]_coloring() method.\n \n\n The coloring method.\n \n\n The the location of the coloring property.\n \n\n The name of the coloring property.\n \n\n The texture for the coloring.\n\nC++: easy3d::State::set_coloring(enum easy3d::State::Method, enum easy3d::State::Location, const std::string &) --> void", pybind11::arg("method"), pybind11::arg("location"), pybind11::arg("name"));
		cl.def("set_coloring_method", (void (easy3d::State::*)(enum easy3d::State::Method)) &easy3d::State::set_coloring_method, "Sets the coloring method.\n \n\n It has the same effect as if set_coloring() is called without changing the location (2nd argument)\n      and name (3rd argument) of the coloring property. \n\n set_coloring()\n \n\n The coloring method.\n\nC++: easy3d::State::set_coloring_method(enum easy3d::State::Method) --> void", pybind11::arg("method"));
		cl.def("coloring_method", (enum easy3d::State::Method (easy3d::State::*)() const) &easy3d::State::coloring_method, "Returns the coloring method being used for rendering. \n\nC++: easy3d::State::coloring_method() const --> enum easy3d::State::Method");
		cl.def("color", (const class easy3d::Vec<4, float> & (easy3d::State::*)() const) &easy3d::State::color, "The color, which is effective only when the coloring method was set to UNIFORM_COLOR.\n Call set_uniform_coloring() to change this color.\n\nC++: easy3d::State::color() const --> const class easy3d::Vec<4, float> &", pybind11::return_value_policy::automatic);
		cl.def("set_color", (void (easy3d::State::*)(const class easy3d::Vec<4, float> &)) &easy3d::State::set_color, "C++: easy3d::State::set_color(const class easy3d::Vec<4, float> &) --> void", pybind11::arg("c"));
		cl.def("property_location", (enum easy3d::State::Location (easy3d::State::*)() const) &easy3d::State::property_location, "The location of the color property. \n\nC++: easy3d::State::property_location() const --> enum easy3d::State::Location");
		cl.def("property_name", (const std::string & (easy3d::State::*)() const) &easy3d::State::property_name, "The name of the color attribute. \n\nC++: easy3d::State::property_name() const --> const std::string &", pybind11::return_value_policy::automatic);
		cl.def("lighting", (bool (easy3d::State::*)() const) &easy3d::State::lighting, "returns whether lighting is enabled.\n\nC++: easy3d::State::lighting() const --> bool");
		cl.def("set_lighting", (void (easy3d::State::*)(bool)) &easy3d::State::set_lighting, "enables/disables lighting.\n\nC++: easy3d::State::set_lighting(bool) --> void", pybind11::arg("l"));
		cl.def("lighting_two_sides", (bool (easy3d::State::*)() const) &easy3d::State::lighting_two_sides, "returns whether double-sided lighting is enabled.\n\nC++: easy3d::State::lighting_two_sides() const --> bool");
		cl.def("set_lighting_two_sides", (void (easy3d::State::*)(bool)) &easy3d::State::set_lighting_two_sides, "enables/disables double-sided lighting.\n\nC++: easy3d::State::set_lighting_two_sides(bool) --> void", pybind11::arg("b"));
		cl.def("distinct_back_color", (bool (easy3d::State::*)() const) &easy3d::State::distinct_back_color, "returns whether a different color is used for rendering the backside of a drawable.\n \n\n effective only when two-sides lighting is enabled\n\nC++: easy3d::State::distinct_back_color() const --> bool");
		cl.def("set_distinct_back_color", (void (easy3d::State::*)(bool)) &easy3d::State::set_distinct_back_color, "enables/disables different backside color.\n \n\n effective only when two-sides lighting is enabled\n\nC++: easy3d::State::set_distinct_back_color(bool) --> void", pybind11::arg("b"));
		cl.def("back_color", (const class easy3d::Vec<4, float> & (easy3d::State::*)() const) &easy3d::State::back_color, "returns the backside color.\n \n\n effective only when two-sides lighting and distinct back color are enabled.\n \n\n lighting_two_sides(), distinct_back_color()\n\nC++: easy3d::State::back_color() const --> const class easy3d::Vec<4, float> &", pybind11::return_value_policy::automatic);
		cl.def("set_back_color", (void (easy3d::State::*)(const class easy3d::Vec<4, float> &)) &easy3d::State::set_back_color, "sets the backside color.\n \n\n effective only when two-sides lighting and distinct back color are enabled.\n \n\n set_lighting_two_sides(), set_distinct_back_color()\n\nC++: easy3d::State::set_back_color(const class easy3d::Vec<4, float> &) --> void", pybind11::arg("c"));
		cl.def("texture", (const class easy3d::Texture * (easy3d::State::*)() const) &easy3d::State::texture, "Memory management of textures is the user's responsibility. \n\nC++: easy3d::State::texture() const --> const class easy3d::Texture *", pybind11::return_value_policy::automatic);
		cl.def("set_texture", (void (easy3d::State::*)(class easy3d::Texture *)) &easy3d::State::set_texture, "C++: easy3d::State::set_texture(class easy3d::Texture *) --> void", pybind11::arg("tex"));
		cl.def("texture_repeat", (float (easy3d::State::*)() const) &easy3d::State::texture_repeat, "How many times do you want to repeat the texture? \n\nC++: easy3d::State::texture_repeat() const --> float");
		cl.def("set_texture_repeat", (void (easy3d::State::*)(float)) &easy3d::State::set_texture_repeat, "C++: easy3d::State::set_texture_repeat(float) --> void", pybind11::arg("r"));
		cl.def("texture_fractional_repeat", (float (easy3d::State::*)() const) &easy3d::State::texture_fractional_repeat, "Controls the texture repeat at a finer level: 100 fractional repeat == 1 repeat. \n\nC++: easy3d::State::texture_fractional_repeat() const --> float");
		cl.def("set_texture_fractional_repeat", (void (easy3d::State::*)(float)) &easy3d::State::set_texture_fractional_repeat, "C++: easy3d::State::set_texture_fractional_repeat(float) --> void", pybind11::arg("fr"));
		cl.def("is_ssao_enabled", (bool (easy3d::State::*)() const) &easy3d::State::is_ssao_enabled, "C++: easy3d::State::is_ssao_enabled() const --> bool");
		cl.def("enable_ssao", (void (easy3d::State::*)(bool)) &easy3d::State::enable_ssao, "C++: easy3d::State::enable_ssao(bool) --> void", pybind11::arg("b"));
		cl.def("set_ssao_texture", (void (easy3d::State::*)(unsigned int)) &easy3d::State::set_ssao_texture, "C++: easy3d::State::set_ssao_texture(unsigned int) --> void", pybind11::arg("tex"));
		cl.def("clamp_range", (bool (easy3d::State::*)() const) &easy3d::State::clamp_range, "Clamp the value range of a scalar field. \n\nC++: easy3d::State::clamp_range() const --> bool");
		cl.def("set_clamp_range", (void (easy3d::State::*)(bool)) &easy3d::State::set_clamp_range, "C++: easy3d::State::set_clamp_range(bool) --> void", pybind11::arg("b"));
		cl.def("clamp_lower", (float (easy3d::State::*)() const) &easy3d::State::clamp_lower, "Clamp the lower side of the value range of a scalar field. Value is in [0, 1] (i.e., 0% to 100%).\n\nC++: easy3d::State::clamp_lower() const --> float");
		cl.def("set_clamp_lower", (void (easy3d::State::*)(float)) &easy3d::State::set_clamp_lower, "C++: easy3d::State::set_clamp_lower(float) --> void", pybind11::arg("v"));
		cl.def("clamp_upper", (float (easy3d::State::*)() const) &easy3d::State::clamp_upper, "Clamp the upper side of the value range of a scalar field. Value is in [0, 1] (i.e., 0% to 100%).\n\nC++: easy3d::State::clamp_upper() const --> float");
		cl.def("set_clamp_upper", (void (easy3d::State::*)(float)) &easy3d::State::set_clamp_upper, "C++: easy3d::State::set_clamp_upper(float) --> void", pybind11::arg("v"));
		cl.def("material", (struct easy3d::State::Material & (easy3d::State::*)()) &easy3d::State::material, "C++: easy3d::State::material() --> struct easy3d::State::Material &", pybind11::return_value_policy::automatic);
		cl.def("set_material", (void (easy3d::State::*)(const struct easy3d::State::Material &)) &easy3d::State::set_material, "C++: easy3d::State::set_material(const struct easy3d::State::Material &) --> void", pybind11::arg("m"));
		cl.def("plane_clip_discard_primitive", (bool (easy3d::State::*)() const) &easy3d::State::plane_clip_discard_primitive, "Controls the behavior for vertex clipping.\n \n\n If plane_clip_discard_primitive() is  (default value, standard plane clip), the clip\n      distances will be linearly interpolated across the primitive (e.g., line, triangle) and the portion\n      of the primitive with interpolated distances less than 0.0 will be clipped.\n      If plane_clip_discard_primitive() is  a primitive will be completely discarded if one of its\n      vertices has a negative clip distance.\n\nC++: easy3d::State::plane_clip_discard_primitive() const --> bool");
		cl.def("set_plane_clip_discard_primitive", (void (easy3d::State::*)(bool)) &easy3d::State::set_plane_clip_discard_primitive, "C++: easy3d::State::set_plane_clip_discard_primitive(bool) --> void", pybind11::arg("b"));
		cl.def("highlight", (bool (easy3d::State::*)() const) &easy3d::State::highlight, "Highlight a subset of primitives of this drawable. Primitives with indices within the range\n [highlight_id_low_, highlight_id_high_] will be highlighted.\n \n\n Specifies the min and max indices of the primitives to be highlighted. Providing [-1, -1] will\n              un-highlight any previously highlighted primitives.\n \n\n For non-triangular surface meshes, all polygonal faces are internally triangulated to allow a\n            unified rendering APIs. The range must be given on the triangulated faces! If you have face range,\n            you should convert the face range to triangle range. For each face, its triangles are defined on\n            SurfaceMesh::face_property<std::pair<int, int> >(\"f:triangle_range\").\n\nC++: easy3d::State::highlight() const --> bool");
		cl.def("set_highlight", (void (easy3d::State::*)(bool)) &easy3d::State::set_highlight, "C++: easy3d::State::set_highlight(bool) --> void", pybind11::arg("b"));
		cl.def("set_highlight_range", (void (easy3d::State::*)(const struct std::pair<int, int> &)) &easy3d::State::set_highlight_range, "C++: easy3d::State::set_highlight_range(const struct std::pair<int, int> &) --> void", pybind11::arg("range"));
		cl.def("highlight_range", (const struct std::pair<int, int> & (easy3d::State::*)() const) &easy3d::State::highlight_range, "C++: easy3d::State::highlight_range() const --> const struct std::pair<int, int> &", pybind11::return_value_policy::automatic);

		{ // easy3d::State::Material file:easy3d/renderer/state.h line:79
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::State::Material, std::shared_ptr<easy3d::State::Material>> cl(enclosing_class, "Material", "Material definition.");
			cl.def( pybind11::init( [](){ return new easy3d::State::Material(); } ) );
			cl.def( pybind11::init<const class easy3d::Vec<4, float> &, const class easy3d::Vec<4, float> &, float>(), pybind11::arg("ambi"), pybind11::arg("spec"), pybind11::arg("shin") );

			cl.def_readwrite("ambient", &easy3d::State::Material::ambient);
			cl.def_readwrite("specular", &easy3d::State::Material::specular);
			cl.def_readwrite("shininess", &easy3d::State::Material::shininess);
			cl.def("assign", (struct easy3d::State::Material & (easy3d::State::Material::*)(const struct easy3d::State::Material &)) &easy3d::State::Material::operator=, "C++: easy3d::State::Material::operator=(const struct easy3d::State::Material &) --> struct easy3d::State::Material &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

	}
}
