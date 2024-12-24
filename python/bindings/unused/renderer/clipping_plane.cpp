#include <easy3d/core/box.h>
#include <easy3d/core/line.h>
#include <easy3d/core/mat.h>
#include <easy3d/core/model.h>
#include <easy3d/core/plane.h>
#include <easy3d/core/quat.h>
#include <easy3d/core/vec.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/renderer/constraint.h>
#include <easy3d/renderer/drawable.h>
#include <easy3d/renderer/frame.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/manipulated_frame.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/state.h>
#include <easy3d/renderer/texture.h>
#include <easy3d/renderer/vertex_array_object.h>
#include <functional>
#include <ios>
#include <memory>
#include <ostream>
#include <sstream> // __str__
#include <streambuf>
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

// easy3d::ClippingPlane file:easy3d/renderer/clipping_plane.h line:44
struct PyCallBack_easy3d_ClippingPlane : public easy3d::ClippingPlane {
	using easy3d::ClippingPlane::ClippingPlane;

	class easy3d::Manipulator * manipulator() override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::ClippingPlane *>(this), "manipulator");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<class easy3d::Manipulator *>::value) {
				static pybind11::detail::override_caster_t<class easy3d::Manipulator *> caster;
				return pybind11::detail::cast_ref<class easy3d::Manipulator *>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<class easy3d::Manipulator *>(std::move(o));
		}
		return ClippingPlane::manipulator();
	}
};

// easy3d::Drawable file:easy3d/renderer/drawable.h line:56
struct PyCallBack_easy3d_Drawable : public easy3d::Drawable {
	using easy3d::Drawable::Drawable;

	enum easy3d::Drawable::Type type() const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Drawable *>(this), "type");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<enum easy3d::Drawable::Type>::value) {
				static pybind11::detail::override_caster_t<enum easy3d::Drawable::Type> caster;
				return pybind11::detail::cast_ref<enum easy3d::Drawable::Type>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<enum easy3d::Drawable::Type>(std::move(o));
		}
		pybind11::pybind11_fail("Tried to call pure virtual function \"Drawable::type\"");
	}
	void draw(const class easy3d::Camera * a0) const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Drawable *>(this), "draw");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		pybind11::pybind11_fail("Tried to call pure virtual function \"Drawable::draw\"");
	}
	void update_buffers_internal() override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Drawable *>(this), "update_buffers_internal");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return Drawable::update_buffers_internal();
	}
};

void bind_easy3d_renderer_clipping_plane(pybind11::module_& m)
{
	{ // easy3d::ClippingPlane file:easy3d/renderer/clipping_plane.h line:44
		pybind11::class_<easy3d::ClippingPlane, std::shared_ptr<easy3d::ClippingPlane>, PyCallBack_easy3d_ClippingPlane> cl(m, "ClippingPlane", "An implementation of a manipulable clipping plane for visualization of model interiors and cross sections.\n \n\n It also provides functions for visualizing the clipping plane itself.\n\n \n\n     ");
		cl.def( pybind11::init( [](){ return new easy3d::ClippingPlane(); }, [](){ return new PyCallBack_easy3d_ClippingPlane(); } ) );
		cl.def_static("instance", (class easy3d::ClippingPlane * (*)()) &easy3d::ClippingPlane::instance, "returns the instance of the clipping plane.\n The entire program will have only one clipping plane.\n\nC++: easy3d::ClippingPlane::instance() --> class easy3d::ClippingPlane *", pybind11::return_value_policy::automatic);
		cl.def("is_enabled", (bool (easy3d::ClippingPlane::*)() const) &easy3d::ClippingPlane::is_enabled, "returns whether the clipping plane has been enabled.\n\nC++: easy3d::ClippingPlane::is_enabled() const --> bool");
		cl.def("set_enabled", (void (easy3d::ClippingPlane::*)(bool)) &easy3d::ClippingPlane::set_enabled, "enables/disables the clipping plane.\n\nC++: easy3d::ClippingPlane::set_enabled(bool) --> void", pybind11::arg("b"));
		cl.def("fit_scene", (void (easy3d::ClippingPlane::*)(const class easy3d::Vec<3, float> &, float)) &easy3d::ClippingPlane::fit_scene, "puts the clipping plane at the center of the scene.\n\nC++: easy3d::ClippingPlane::fit_scene(const class easy3d::Vec<3, float> &, float) --> void", pybind11::arg("center"), pybind11::arg("radius"));
		cl.def("center", (class easy3d::Vec<3, float> (easy3d::ClippingPlane::*)() const) &easy3d::ClippingPlane::center, "the center of the clipping plane. \n normal()\n\nC++: easy3d::ClippingPlane::center() const --> class easy3d::Vec<3, float>");
		cl.def("normal", (class easy3d::Vec<3, float> (easy3d::ClippingPlane::*)() const) &easy3d::ClippingPlane::normal, "the normal direction of the clipping plane. \n center().\n\nC++: easy3d::ClippingPlane::normal() const --> class easy3d::Vec<3, float>");
		cl.def("plane0", (class easy3d::GenericPlane<float> (easy3d::ClippingPlane::*)() const) &easy3d::ClippingPlane::plane0, "the clipping plane\n\nC++: easy3d::ClippingPlane::plane0() const --> class easy3d::GenericPlane<float>");
		cl.def("plane1", (class easy3d::GenericPlane<float> (easy3d::ClippingPlane::*)() const) &easy3d::ClippingPlane::plane1, "the other clipping plane. plane0() and plane1() together define a cross section.\n\nC++: easy3d::ClippingPlane::plane1() const --> class easy3d::GenericPlane<float>");
		cl.def("is_visible", (bool (easy3d::ClippingPlane::*)() const) &easy3d::ClippingPlane::is_visible, "returns whether the clipping plane is visible.\n\nC++: easy3d::ClippingPlane::is_visible() const --> bool");
		cl.def("set_visible", (void (easy3d::ClippingPlane::*)(bool)) &easy3d::ClippingPlane::set_visible, "shows/hides the clipping plane.\n\nC++: easy3d::ClippingPlane::set_visible(bool) --> void", pybind11::arg("v"));
		cl.def("color", (const class easy3d::Vec<4, float> & (easy3d::ClippingPlane::*)() const) &easy3d::ClippingPlane::color, "returns the color of the clipping plane.\n\nC++: easy3d::ClippingPlane::color() const --> const class easy3d::Vec<4, float> &", pybind11::return_value_policy::automatic);
		cl.def("set_color", (void (easy3d::ClippingPlane::*)(const class easy3d::Vec<4, float> &)) &easy3d::ClippingPlane::set_color, "sets the color of the clipping plane.\n\nC++: easy3d::ClippingPlane::set_color(const class easy3d::Vec<4, float> &) --> void", pybind11::arg("c"));
		cl.def("cross_section", (bool (easy3d::ClippingPlane::*)() const) &easy3d::ClippingPlane::cross_section, "returns whether cross section is enabled.\n\nC++: easy3d::ClippingPlane::cross_section() const --> bool");
		cl.def("set_cross_section", (void (easy3d::ClippingPlane::*)(bool)) &easy3d::ClippingPlane::set_cross_section, "enables/disables cross section.\n\nC++: easy3d::ClippingPlane::set_cross_section(bool) --> void", pybind11::arg("b"));
		cl.def("cross_section_width", (float (easy3d::ClippingPlane::*)() const) &easy3d::ClippingPlane::cross_section_width, "returns the thickness of the cross section.\n \n\n The thickness is defined relative to the scene bounding box. Default value is 0.01.\n\nC++: easy3d::ClippingPlane::cross_section_width() const --> float");
		cl.def("set_cross_section_width", (void (easy3d::ClippingPlane::*)(float)) &easy3d::ClippingPlane::set_cross_section_width, "sets the thickness of the cross section.\n\nC++: easy3d::ClippingPlane::set_cross_section_width(float) --> void", pybind11::arg("w"));
		cl.def("set_program", (void (easy3d::ClippingPlane::*)(class easy3d::ShaderProgram *) const) &easy3d::ClippingPlane::set_program, "sets up the shader program for rendering.\n \n\n The shader program.\n\nC++: easy3d::ClippingPlane::set_program(class easy3d::ShaderProgram *) const --> void", pybind11::arg("program"));
		cl.def("set_discard_primitives", (void (easy3d::ClippingPlane::*)(class easy3d::ShaderProgram *, bool) const) &easy3d::ClippingPlane::set_discard_primitives, "sets up whether a primitive should be completely discarded if one of its vertices is clipped.\n \n\n The shader program.\n \n\n Controls clipping plane behavior.\n     - true: completely discard a primitive if one of its vertices is clipped by this clipping plane.\n     - false: standard plane clipping\n\nC++: easy3d::ClippingPlane::set_discard_primitives(class easy3d::ShaderProgram *, bool) const --> void", pybind11::arg("program"), pybind11::arg("plane_clipping_discard"));
		cl.def("draw", (void (easy3d::ClippingPlane::*)(class easy3d::Camera *) const) &easy3d::ClippingPlane::draw, "draws the clipping plane itself.\n\nC++: easy3d::ClippingPlane::draw(class easy3d::Camera *) const --> void", pybind11::arg("cam"));
		cl.def("manipulator", (class easy3d::Manipulator * (easy3d::ClippingPlane::*)()) &easy3d::ClippingPlane::manipulator, "returns the manipulator attached to the clipping plane.\n\nC++: easy3d::ClippingPlane::manipulator() --> class easy3d::Manipulator *", pybind11::return_value_policy::automatic);
		cl.def("assign", (class easy3d::ClippingPlane & (easy3d::ClippingPlane::*)(const class easy3d::ClippingPlane &)) &easy3d::ClippingPlane::operator=, "C++: easy3d::ClippingPlane::operator=(const class easy3d::ClippingPlane &) --> class easy3d::ClippingPlane &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}
	{ // easy3d::Drawable file:easy3d/renderer/drawable.h line:56
		pybind11::class_<easy3d::Drawable, std::shared_ptr<easy3d::Drawable>, PyCallBack_easy3d_Drawable, easy3d::State> cl(m, "Drawable", "The base class for drawable objects. A drawable represent a set of points, line segments, or triangles.\n \n\n\n \n A Drawable is an abstraction for \"something that can be drawn\", e.g., a point cloud, the surface\n          of a mesh, the wireframe of a surface mesh, the vertices of a graph, the border of a polyhedral mesh.\n          A drawable manages its rendering status and controls the upload of the data to the GPU.\n          A drawable can live independently or be associated with a Model.\n          The rendering states of a drawable can be accessed/modified through the functions provided in its\n          parent class State.\n \n\n State, Renderer");
		cl.def( pybind11::init( [](){ return new PyCallBack_easy3d_Drawable(); } ), "doc");
		cl.def( pybind11::init( [](const std::string & a0){ return new PyCallBack_easy3d_Drawable(a0); } ), "doc");
		cl.def( pybind11::init<const std::string &, class easy3d::Model *>(), pybind11::arg("name"), pybind11::arg("model") );

		cl.def(pybind11::init<PyCallBack_easy3d_Drawable const &>());

		pybind11::enum_<easy3d::Drawable::Type>(cl, "Type", pybind11::arithmetic(), "")
			.value("DT_POINTS", easy3d::Drawable::DT_POINTS)
			.value("DT_LINES", easy3d::Drawable::DT_LINES)
			.value("DT_TRIANGLES", easy3d::Drawable::DT_TRIANGLES)
			.export_values();

		cl.def("type", (enum easy3d::Drawable::Type (easy3d::Drawable::*)() const) &easy3d::Drawable::type, "Returns the type of the drawable.\n\nC++: easy3d::Drawable::type() const --> enum easy3d::Drawable::Type");
		cl.def("name", (const std::string & (easy3d::Drawable::*)() const) &easy3d::Drawable::name, "C++: easy3d::Drawable::name() const --> const std::string &", pybind11::return_value_policy::automatic);
		cl.def("set_name", (void (easy3d::Drawable::*)(const std::string &)) &easy3d::Drawable::set_name, "C++: easy3d::Drawable::set_name(const std::string &) --> void", pybind11::arg("n"));
		cl.def("model", (class easy3d::Model * (easy3d::Drawable::*)()) &easy3d::Drawable::model, "the model to which the drawable is attached to (can be NULL).\n\nC++: easy3d::Drawable::model() --> class easy3d::Model *", pybind11::return_value_policy::automatic);
		cl.def("set_model", (void (easy3d::Drawable::*)(class easy3d::Model *)) &easy3d::Drawable::set_model, "C++: easy3d::Drawable::set_model(class easy3d::Model *) --> void", pybind11::arg("m"));
		cl.def("bounding_box", (const class easy3d::GenericBox<3, float> & (easy3d::Drawable::*)() const) &easy3d::Drawable::bounding_box, "C++: easy3d::Drawable::bounding_box() const --> const class easy3d::GenericBox<3, float> &", pybind11::return_value_policy::automatic);
		cl.def("state", (class easy3d::State & (easy3d::Drawable::*)()) &easy3d::Drawable::state, "C++: easy3d::Drawable::state() --> class easy3d::State &", pybind11::return_value_policy::automatic);
		cl.def("set_state", (void (easy3d::Drawable::*)(const class easy3d::State &)) &easy3d::Drawable::set_state, "C++: easy3d::Drawable::set_state(const class easy3d::State &) --> void", pybind11::arg("s"));
		cl.def("buffer_stats", (void (easy3d::Drawable::*)(std::ostream &) const) &easy3d::Drawable::buffer_stats, "print statistics (e.g., num vertices, memory usage) of the buffers to an output stream (e.g., std::cout).\n\nC++: easy3d::Drawable::buffer_stats(std::ostream &) const --> void", pybind11::arg("output"));
		cl.def("vertex_buffer", (unsigned int (easy3d::Drawable::*)() const) &easy3d::Drawable::vertex_buffer, "@{\n\nC++: easy3d::Drawable::vertex_buffer() const --> unsigned int");
		cl.def("color_buffer", (unsigned int (easy3d::Drawable::*)() const) &easy3d::Drawable::color_buffer, "C++: easy3d::Drawable::color_buffer() const --> unsigned int");
		cl.def("normal_buffer", (unsigned int (easy3d::Drawable::*)() const) &easy3d::Drawable::normal_buffer, "C++: easy3d::Drawable::normal_buffer() const --> unsigned int");
		cl.def("texcoord_buffer", (unsigned int (easy3d::Drawable::*)() const) &easy3d::Drawable::texcoord_buffer, "C++: easy3d::Drawable::texcoord_buffer() const --> unsigned int");
		cl.def("element_buffer", (unsigned int (easy3d::Drawable::*)() const) &easy3d::Drawable::element_buffer, "C++: easy3d::Drawable::element_buffer() const --> unsigned int");
		cl.def("update_vertex_buffer", [](easy3d::Drawable &o, const class std::vector<class easy3d::Vec<3, float> > & a0) -> void { return o.update_vertex_buffer(a0); }, "", pybind11::arg("vertices"));
		cl.def("update_vertex_buffer", (void (easy3d::Drawable::*)(const class std::vector<class easy3d::Vec<3, float> > &, bool)) &easy3d::Drawable::update_vertex_buffer, "Creates/Updates a single buffer.\n \n\n Primitives like lines and triangles can be drawn with or without the element buffer.\n  - With an element buffer: this can reduce the GPU memory consumption.\n  - Without an element buffer: easier data transfer, but uses more GPU memory. In this case, vertices need to\n    be in a correct order, like f1_v1, f1_v2, f1_v3, f2_v1, f2_v2, f2_v3... This requires the shared vertices\n    be duplicated in the vertex buffer.\n\nC++: easy3d::Drawable::update_vertex_buffer(const class std::vector<class easy3d::Vec<3, float> > &, bool) --> void", pybind11::arg("vertices"), pybind11::arg("dynamic"));
		cl.def("update_color_buffer", [](easy3d::Drawable &o, const class std::vector<class easy3d::Vec<3, float> > & a0) -> void { return o.update_color_buffer(a0); }, "", pybind11::arg("colors"));
		cl.def("update_color_buffer", (void (easy3d::Drawable::*)(const class std::vector<class easy3d::Vec<3, float> > &, bool)) &easy3d::Drawable::update_color_buffer, "C++: easy3d::Drawable::update_color_buffer(const class std::vector<class easy3d::Vec<3, float> > &, bool) --> void", pybind11::arg("colors"), pybind11::arg("dynamic"));
		cl.def("update_normal_buffer", [](easy3d::Drawable &o, const class std::vector<class easy3d::Vec<3, float> > & a0) -> void { return o.update_normal_buffer(a0); }, "", pybind11::arg("normals"));
		cl.def("update_normal_buffer", (void (easy3d::Drawable::*)(const class std::vector<class easy3d::Vec<3, float> > &, bool)) &easy3d::Drawable::update_normal_buffer, "C++: easy3d::Drawable::update_normal_buffer(const class std::vector<class easy3d::Vec<3, float> > &, bool) --> void", pybind11::arg("normals"), pybind11::arg("dynamic"));
		cl.def("update_texcoord_buffer", [](easy3d::Drawable &o, const class std::vector<class easy3d::Vec<2, float> > & a0) -> void { return o.update_texcoord_buffer(a0); }, "", pybind11::arg("texcoords"));
		cl.def("update_texcoord_buffer", (void (easy3d::Drawable::*)(const class std::vector<class easy3d::Vec<2, float> > &, bool)) &easy3d::Drawable::update_texcoord_buffer, "C++: easy3d::Drawable::update_texcoord_buffer(const class std::vector<class easy3d::Vec<2, float> > &, bool) --> void", pybind11::arg("texcoords"), pybind11::arg("dynamic"));
		cl.def("update_element_buffer", (void (easy3d::Drawable::*)(const class std::vector<unsigned int> &)) &easy3d::Drawable::update_element_buffer, "C++: easy3d::Drawable::update_element_buffer(const class std::vector<unsigned int> &) --> void", pybind11::arg("elements"));
		cl.def("update_element_buffer", (void (easy3d::Drawable::*)(const class std::vector<class std::vector<unsigned int> > &)) &easy3d::Drawable::update_element_buffer, "Updates the element buffer.\n \n\n This is an overload of the above update_element_buffer() method.\n \n\n Each entry must have 2 (for LinesDrawable) or 3 elements (for TrianglesDrawable).\n\nC++: easy3d::Drawable::update_element_buffer(const class std::vector<class std::vector<unsigned int> > &) --> void", pybind11::arg("elements"));
		cl.def("disable_element_buffer", (void (easy3d::Drawable::*)()) &easy3d::Drawable::disable_element_buffer, "Disables the use of the element buffer.\n \n\n This method should be used if existing vertex data is sufficient for rendering (may require\n         duplicating vertex data).\n \n\n This method also releases the element buffer.\n\nC++: easy3d::Drawable::disable_element_buffer() --> void");
		cl.def("num_vertices", (std::size_t (easy3d::Drawable::*)() const) &easy3d::Drawable::num_vertices, "@}\n\nC++: easy3d::Drawable::num_vertices() const --> std::size_t");
		cl.def("draw", (void (easy3d::Drawable::*)(const class easy3d::Camera *) const) &easy3d::Drawable::draw, "The draw method.\n\nC++: easy3d::Drawable::draw(const class easy3d::Camera *) const --> void", pybind11::arg("camera"));
		cl.def("gl_draw", (void (easy3d::Drawable::*)() const) &easy3d::Drawable::gl_draw, "The internal draw method of this drawable.\n NOTE: this functions should be called when your shader program is in use,\n		 i.e., between glUseProgram(id) and glUseProgram(0);\n\nC++: easy3d::Drawable::gl_draw() const --> void");
		cl.def("update", (void (easy3d::Drawable::*)()) &easy3d::Drawable::update, "Requests an update of the OpenGL buffers.\n \n\n This function sets the status to trigger an update of the OpenGL buffers. The actual update does\n      not occur immediately but is deferred to the rendering phase.\n \n\n This method works for both standard drawables (no update function required) and non-standard\n      drawable (update function required). Standard drawables include:\n            - SurfaceMesh: \"faces\", \"edges\", \"vertices\", \"borders\", and \"locks\";\n            - PointCloud: \"vertices\";\n            - Graph: \"edges\", and \"vertices\",\n            - PolyMesh: \"faces:border\" and \"faces:interior\".\n \n\n set_update_func(), Renderer::update()\n\nC++: easy3d::Drawable::update() --> void");
		cl.def("set_update_func", (void (easy3d::Drawable::*)(const class std::function<void (class easy3d::Model *, class easy3d::Drawable *)> &)) &easy3d::Drawable::set_update_func, "Setups how a drawable updates its rendering buffers.\n \n\n This function is required by only non-standard drawables for a special visualization purpose.\n      Rendering buffers of standard drawables attached to a model can be automatically updated and do not\n      require this function.\n      The drawable is considered standalone if  is \n \n\n update(), Renderer::update().\n \n\n For a subclassed drawable, you can also reimplement update_buffers_internal() for the same purpose.\n\nC++: easy3d::Drawable::set_update_func(const class std::function<void (class easy3d::Model *, class easy3d::Drawable *)> &) --> void", pybind11::arg("func"));
		cl.def("manipulator", (class easy3d::Manipulator * (easy3d::Drawable::*)()) &easy3d::Drawable::manipulator, "@{\n\n \n Gets the manipulator attached to this drawable.\n \n\n If the drawable is part of a model, it returns the model's manipulator.\n      Returns  if the drawable cannot be manipulated.\n\nC++: easy3d::Drawable::manipulator() --> class easy3d::Manipulator *", pybind11::return_value_policy::automatic);
		cl.def("set_manipulator", (void (easy3d::Drawable::*)(class easy3d::Manipulator *)) &easy3d::Drawable::set_manipulator, "Attaches a manipulator to this model.\n \n\n This is used to manipulate a drawable that is not part of a model.\n\nC++: easy3d::Drawable::set_manipulator(class easy3d::Manipulator *) --> void", pybind11::arg("manip"));
		cl.def("manipulated_matrix", (class easy3d::Mat4<float> (easy3d::Drawable::*)() const) &easy3d::Drawable::manipulated_matrix, "Returns the manipulation matrix.\n\nC++: easy3d::Drawable::manipulated_matrix() const --> class easy3d::Mat4<float>");
		cl.def("vao", (class easy3d::VertexArrayObject * (easy3d::Drawable::*)()) &easy3d::Drawable::vao, "Returns the vertex array object of this drawable.\n\nC++: easy3d::Drawable::vao() --> class easy3d::VertexArrayObject *", pybind11::return_value_policy::automatic);
		cl.def("assign", (class easy3d::Drawable & (easy3d::Drawable::*)(const class easy3d::Drawable &)) &easy3d::Drawable::operator=, "C++: easy3d::Drawable::operator=(const class easy3d::Drawable &) --> class easy3d::Drawable &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}
}
