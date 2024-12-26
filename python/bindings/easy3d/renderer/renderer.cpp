#include <easy3d/core/box.h>
#include <easy3d/core/graph.h>
#include <easy3d/core/model.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/vec.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/shader_program.h>
#include <memory>
#include <ostream>
#include <sstream> // __str__
#include <string>
#include <string_view>
#include <typeinfo>
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

void bind_easy3d_renderer_renderer(pybind11::module_& m)
{
	{ // easy3d::Renderer file:easy3d/renderer/renderer.h line:62
		pybind11::class_<easy3d::Renderer, std::shared_ptr<easy3d::Renderer>> cl(m, "Renderer", "A Renderer manages the drawables (and thus the rendering) of a model.\n \n\n Renderer also allow to create default drawables for most rendering purposes.\n \n\n\n \n Drawable, PointsDrawable, LinesDrawable, TrianglesDrawable\n\n The following code shows how to use renderer:\n \n\n\n\n\n \n\n\n\n     ");
		cl.def( pybind11::init( [](class easy3d::Model * a0){ return new easy3d::Renderer(a0); } ), "doc" , pybind11::arg("model"));
		cl.def( pybind11::init<class easy3d::Model *, bool>(), pybind11::arg("model"), pybind11::arg("create_drawables") );

		cl.def( pybind11::init( [](easy3d::Renderer const &o){ return new easy3d::Renderer(o); } ) );
		cl.def("model", (class easy3d::Model * (easy3d::Renderer::*)()) &easy3d::Renderer::model, "The model to which this renderer is attached.\n\nC++: easy3d::Renderer::model() --> class easy3d::Model *", pybind11::return_value_policy::automatic);
		cl.def("is_visible", (bool (easy3d::Renderer::*)() const) &easy3d::Renderer::is_visible, "Returns whether the model is currently visible.\n\nC++: easy3d::Renderer::is_visible() const --> bool");
		cl.def("set_visible", (void (easy3d::Renderer::*)(bool)) &easy3d::Renderer::set_visible, "Shows/Hides the model.\n\nC++: easy3d::Renderer::set_visible(bool) --> void", pybind11::arg("b"));
		cl.def("is_selected", (bool (easy3d::Renderer::*)() const) &easy3d::Renderer::is_selected, "Returns whether the model has been selected.\n\nC++: easy3d::Renderer::is_selected() const --> bool");
		cl.def("set_selected", (void (easy3d::Renderer::*)(bool)) &easy3d::Renderer::set_selected, "Select/Deselect the model. The state of all its drawables will change accordingly.\n\nC++: easy3d::Renderer::set_selected(bool) --> void", pybind11::arg("b"));
		cl.def("update", (void (easy3d::Renderer::*)()) &easy3d::Renderer::update, "Invalidates the rendering buffers of the model and thus updates the rendering (delayed in rendering).\n \n\n This method triggers an update of the rendering buffers of all the drawables of the model to which\n      this renderer is attached. The effect is equivalent to calling Drawable::update() functions for all\n      the drawables of this model.\n todo: for better performance, it is wise to update only the affected drawables and buffers.\n \n\n  Drawable::update()\n\nC++: easy3d::Renderer::update() --> void");
		cl.def("get_points_drawable", [](easy3d::Renderer const &o, const std::string & a0) -> easy3d::PointsDrawable * { return o.get_points_drawable(a0); }, "", pybind11::return_value_policy::reference_internal, pybind11::arg("name"));
		cl.def("get_points_drawable", (class easy3d::PointsDrawable * (easy3d::Renderer::*)(const std::string &, bool) const) &easy3d::Renderer::get_points_drawable, "Get the points drawable with a given name.\n Return nullptr if the drawable does not exist.\n\nC++: easy3d::Renderer::get_points_drawable(const std::string &, bool) const --> class easy3d::PointsDrawable *", pybind11::return_value_policy::reference_internal, pybind11::arg("name"), pybind11::arg("warning_not_found"));
		cl.def("get_lines_drawable", [](easy3d::Renderer const &o, const std::string & a0) -> easy3d::LinesDrawable * { return o.get_lines_drawable(a0); }, "", pybind11::return_value_policy::reference_internal, pybind11::arg("name"));
		cl.def("get_lines_drawable", (class easy3d::LinesDrawable * (easy3d::Renderer::*)(const std::string &, bool) const) &easy3d::Renderer::get_lines_drawable, "Get the lines drawable with a given name.\n Return nullptr if the drawable does not exist.\n\nC++: easy3d::Renderer::get_lines_drawable(const std::string &, bool) const --> class easy3d::LinesDrawable *", pybind11::return_value_policy::reference_internal, pybind11::arg("name"), pybind11::arg("warning_not_found"));
		cl.def("get_triangles_drawable", [](easy3d::Renderer const &o, const std::string & a0) -> easy3d::TrianglesDrawable * { return o.get_triangles_drawable(a0); }, "", pybind11::return_value_policy::reference_internal, pybind11::arg("name"));
		cl.def("get_triangles_drawable", (class easy3d::TrianglesDrawable * (easy3d::Renderer::*)(const std::string &, bool) const) &easy3d::Renderer::get_triangles_drawable, "Get the triangles drawable with a given name.\n Return nullptr if the drawable does not exist.\n\nC++: easy3d::Renderer::get_triangles_drawable(const std::string &, bool) const --> class easy3d::TrianglesDrawable *", pybind11::return_value_policy::reference_internal, pybind11::arg("name"), pybind11::arg("warning_not_found"));
		cl.def("add_points_drawable", (class easy3d::PointsDrawable * (easy3d::Renderer::*)(const std::string &)) &easy3d::Renderer::add_points_drawable, "Create a new points drawable and add it to this renderer.\n \n\n The name of the points drawable to be created.\n \n\n The created points drawable. If a points drawable with 'name' already exists, the creation will be\n         ignored and the existing drawable is returned.\n\nC++: easy3d::Renderer::add_points_drawable(const std::string &) --> class easy3d::PointsDrawable *", pybind11::return_value_policy::automatic, pybind11::arg("name"));
		cl.def("add_lines_drawable", (class easy3d::LinesDrawable * (easy3d::Renderer::*)(const std::string &)) &easy3d::Renderer::add_lines_drawable, "Create a new lines drawable and add it to this renderer.\n \n\n The name of the lines drawable to be created.\n \n\n The created lines drawable. If a lines drawable with 'name' already exists, the creation will be\n         ignored and the existing drawable is returned.\n\nC++: easy3d::Renderer::add_lines_drawable(const std::string &) --> class easy3d::LinesDrawable *", pybind11::return_value_policy::automatic, pybind11::arg("name"));
		cl.def("add_triangles_drawable", (class easy3d::TrianglesDrawable * (easy3d::Renderer::*)(const std::string &)) &easy3d::Renderer::add_triangles_drawable, "Create a new triangles drawable and add it to this renderer.\n \n\n The name of the triangles drawable to be created.\n \n\n The created triangles drawable. If a triangles drawable with 'name' already exists, the creation will\n         be ignored and the existing drawable is returned.\n\nC++: easy3d::Renderer::add_triangles_drawable(const std::string &) --> class easy3d::TrianglesDrawable *", pybind11::return_value_policy::automatic, pybind11::arg("name"));
		cl.def("points_drawables", (const class std::vector<class easy3d::PointsDrawable *> & (easy3d::Renderer::*)() const) &easy3d::Renderer::points_drawables, "All available points drawables managed by this renderer.\n\nC++: easy3d::Renderer::points_drawables() const --> const class std::vector<class easy3d::PointsDrawable *> &", pybind11::return_value_policy::reference_internal);
		cl.def("lines_drawables", (const class std::vector<class easy3d::LinesDrawable *> & (easy3d::Renderer::*)() const) &easy3d::Renderer::lines_drawables, "All available lines drawables managed by this renderer.\n\nC++: easy3d::Renderer::lines_drawables() const --> const class std::vector<class easy3d::LinesDrawable *> &", pybind11::return_value_policy::reference_internal);
		cl.def("triangles_drawables", (const class std::vector<class easy3d::TrianglesDrawable *> & (easy3d::Renderer::*)() const) &easy3d::Renderer::triangles_drawables, "All available triangles drawables managed by this renderer.\n\nC++: easy3d::Renderer::triangles_drawables() const --> const class std::vector<class easy3d::TrianglesDrawable *> &", pybind11::return_value_policy::reference_internal);
		cl.def("assign", (class easy3d::Renderer & (easy3d::Renderer::*)(const class easy3d::Renderer &)) &easy3d::Renderer::operator=, "C++: easy3d::Renderer::operator=(const class easy3d::Renderer &) --> class easy3d::Renderer &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}
//	{ // easy3d::ShaderProgram file:easy3d/renderer/shader_program.h line:77
//		pybind11::class_<easy3d::ShaderProgram, std::shared_ptr<easy3d::ShaderProgram>> cl(m, "ShaderProgram", "OpenGL Shader Compilation\n\n \n\n To use the shader program class, you need to do the following:\n - Call load_shader_from_code(ShaderProgram::VERTEX, vert_file) to create vertex shader and\n        load_shader_from_code(ShaderProgram::FRAGMENT, frag_file) to create fragment shader\n        (you may also need to create other types of shaders depending on your needs).\n - Call set_attrib_name(ShaderProgram::POSITION, \"position\") for vertex attribute \"position\".\n        You may also need to set other attributes like normal, color, etc. To know what vertex\n        attributes need to be set, check your shader code or call print_active_attributes().\n - Call link_program() to link the program.\n\n For rendering\n - Call bind();\n - Call set_uniform() to set all the necessary uniforms. You may also need to call set_block(),\n        set_block_uniform(), and/or set_block_uniform_array_element(). To know what uniforms need to\n        be set, check your shader code or call print_active_uniforms(), print_active_uniform_blocks().\n - Call the drawable's draw() function\n - Call release();\n\n To retrieve the model view projection matrix, call camera's  modelViewProjectionMatrix() ");
//		cl.def( pybind11::init( [](){ return new easy3d::ShaderProgram(); } ), "doc" );
//		cl.def( pybind11::init<const std::string &>(), pybind11::arg("name") );
//
//
//		pybind11::enum_<easy3d::ShaderProgram::AttribType>(cl, "AttribType", pybind11::arithmetic(), "")
//			.value("POSITION", easy3d::ShaderProgram::POSITION)
//			.value("COLOR", easy3d::ShaderProgram::COLOR)
//			.value("NORMAL", easy3d::ShaderProgram::NORMAL)
//			.value("TEXCOORD", easy3d::ShaderProgram::TEXCOORD)
//			.export_values();
//
//
//		pybind11::enum_<easy3d::ShaderProgram::ShaderType>(cl, "ShaderType", pybind11::arithmetic(), "")
//			.value("VERTEX", easy3d::ShaderProgram::VERTEX)
//			.value("FRAGMENT", easy3d::ShaderProgram::FRAGMENT)
//			.value("GEOMETRY", easy3d::ShaderProgram::GEOMETRY)
//			.value("TESS_CONTROL", easy3d::ShaderProgram::TESS_CONTROL)
//			.value("TESS_EVALUATION", easy3d::ShaderProgram::TESS_EVALUATION)
//			.value("COMPUTE", easy3d::ShaderProgram::COMPUTE)
//			.value("NUM_SHADER_TYPES", easy3d::ShaderProgram::NUM_SHADER_TYPES)
//			.export_values();
//
//		cl.def_static("is_supported", (bool (*)()) &easy3d::ShaderProgram::is_supported, "C++: easy3d::ShaderProgram::is_supported() --> bool");
//		cl.def("set_name", (void (easy3d::ShaderProgram::*)(const std::string &)) &easy3d::ShaderProgram::set_name, "C++: easy3d::ShaderProgram::set_name(const std::string &) --> void", pybind11::arg("name"));
//		cl.def("name", (const std::string & (easy3d::ShaderProgram::*)() const) &easy3d::ShaderProgram::name, "C++: easy3d::ShaderProgram::name() const --> const std::string &", pybind11::return_value_policy::automatic);
//		cl.def("set_verbose", (void (easy3d::ShaderProgram::*)(bool)) &easy3d::ShaderProgram::set_verbose, "Sets true to log any issues found.\n\nC++: easy3d::ShaderProgram::set_verbose(bool) --> void", pybind11::arg("v"));
//		cl.def("get_program", (unsigned int (easy3d::ShaderProgram::*)() const) &easy3d::ShaderProgram::get_program, "Returns the program index.\n\nC++: easy3d::ShaderProgram::get_program() const --> unsigned int");
//		cl.def("clear", (void (easy3d::ShaderProgram::*)()) &easy3d::ShaderProgram::clear, "Removes (deletes) all shaders\n\nC++: easy3d::ShaderProgram::clear() --> void");
//		cl.def("load_shader_from_file", [](easy3d::ShaderProgram &o, enum easy3d::ShaderProgram::ShaderType const & a0, const std::string & a1) -> bool { return o.load_shader_from_file(a0, a1); }, "", pybind11::arg("st"), pybind11::arg("file_name"));
//		cl.def("load_shader_from_file", (bool (easy3d::ShaderProgram::*)(enum easy3d::ShaderProgram::ShaderType, const std::string &, const std::string &)) &easy3d::ShaderProgram::load_shader_from_file, "Loads the text in the file to the source of the specified shader\n \n\n one of the enum values of ShaderType\n \n\n the full path of the file where the source is to be found\n \n\n the include identifier string (e.g., \"#include\")\n\nC++: easy3d::ShaderProgram::load_shader_from_file(enum easy3d::ShaderProgram::ShaderType, const std::string &, const std::string &) --> bool", pybind11::arg("st"), pybind11::arg("file_name"), pybind11::arg("inc_id"));
//		cl.def("load_shader_from_code", (bool (easy3d::ShaderProgram::*)(enum easy3d::ShaderProgram::ShaderType, const std::string &)) &easy3d::ShaderProgram::load_shader_from_code, "Loads the code to the source of the specified shader\n \n\n one of the enum values of ShaderType\n \n\n the string of the shader source code\n\nC++: easy3d::ShaderProgram::load_shader_from_code(enum easy3d::ShaderProgram::ShaderType, const std::string &) --> bool", pybind11::arg("st"), pybind11::arg("code"));
//		cl.def("set_attrib_name", (void (easy3d::ShaderProgram::*)(enum easy3d::ShaderProgram::AttribType, const std::string &) const) &easy3d::ShaderProgram::set_attrib_name, "Defines semantics for the input vertex attributes. This is required for other libraries to know how to\n send data to the shader.\n \n\n The semantic of the attribute\n \n\n The name of the vertex attribute\n \n\n To specify a location for an attribute you must do it prior to linking the program, or, if the\n      program is already linked, to link it again afterwards (call method link_program)\n\nC++: easy3d::ShaderProgram::set_attrib_name(enum easy3d::ShaderProgram::AttribType, const std::string &) const --> void", pybind11::arg("at"), pybind11::arg("name"));
//		cl.def("set_attrib_names", (void (easy3d::ShaderProgram::*)(const class std::vector<struct std::pair<enum easy3d::ShaderProgram::AttribType, std::string> > &) const) &easy3d::ShaderProgram::set_attrib_names, "C++: easy3d::ShaderProgram::set_attrib_names(const class std::vector<struct std::pair<enum easy3d::ShaderProgram::AttribType, std::string> > &) const --> void", pybind11::arg("attributes"));
//		cl.def("link_program", (bool (easy3d::ShaderProgram::*)()) &easy3d::ShaderProgram::link_program, "Prepares program for usage. Links it and collects information about uniform variables and uniform blocks.\n\nC++: easy3d::ShaderProgram::link_program() --> bool");
//		cl.def("set_program_output", (void (easy3d::ShaderProgram::*)(int, const std::string &) const) &easy3d::ShaderProgram::set_program_output, "Bind a user-defined varying out variable to a fragment shader color number\n Note: linking is required for this operation to take effect\n (call method prepare_program afterwards)\n \n\n The fragment color number\n \n\n The name of the fragment's shader variable\n\nC++: easy3d::ShaderProgram::set_program_output(int, const std::string &) const --> void", pybind11::arg("index"), pybind11::arg("name"));
//		cl.def("program_output", (int (easy3d::ShaderProgram::*)(const std::string &) const) &easy3d::ShaderProgram::program_output, "Returns the fragment shader color number bound to a user-defined varying out variable\n Note: linking is required for this operation to take effect\n (call method prepare_program afterwards)\n \n\n name of the fragment's shader variable\n \n\n the fragment color number\n\nC++: easy3d::ShaderProgram::program_output(const std::string &) const --> int", pybind11::arg("name"));
//		cl.def("bind", (void (easy3d::ShaderProgram::*)() const) &easy3d::ShaderProgram::bind, "Starts using the program.\n\nC++: easy3d::ShaderProgram::bind() const --> void");
//		cl.def("set_uniform", (class easy3d::ShaderProgram * (easy3d::ShaderProgram::*)(const std::string &, const void *)) &easy3d::ShaderProgram::set_uniform, "C++: easy3d::ShaderProgram::set_uniform(const std::string &, const void *) --> class easy3d::ShaderProgram *", pybind11::return_value_policy::automatic, pybind11::arg("name"), pybind11::arg("value"));
//		cl.def("set_uniform", (class easy3d::ShaderProgram * (easy3d::ShaderProgram::*)(const std::string &, int)) &easy3d::ShaderProgram::set_uniform, "C++: easy3d::ShaderProgram::set_uniform(const std::string &, int) --> class easy3d::ShaderProgram *", pybind11::return_value_policy::automatic, pybind11::arg("name"), pybind11::arg("value"));
//		cl.def("set_uniform", (class easy3d::ShaderProgram * (easy3d::ShaderProgram::*)(const std::string &, unsigned int)) &easy3d::ShaderProgram::set_uniform, "C++: easy3d::ShaderProgram::set_uniform(const std::string &, unsigned int) --> class easy3d::ShaderProgram *", pybind11::return_value_policy::automatic, pybind11::arg("name"), pybind11::arg("value"));
//		cl.def("set_uniform", (class easy3d::ShaderProgram * (easy3d::ShaderProgram::*)(const std::string &, float)) &easy3d::ShaderProgram::set_uniform, "C++: easy3d::ShaderProgram::set_uniform(const std::string &, float) --> class easy3d::ShaderProgram *", pybind11::return_value_policy::automatic, pybind11::arg("name"), pybind11::arg("value"));
//		cl.def("set_block", (class easy3d::ShaderProgram * (easy3d::ShaderProgram::*)(const std::string &, const void *)) &easy3d::ShaderProgram::set_block, "Sets a uniform block as a whole\n\nC++: easy3d::ShaderProgram::set_block(const std::string &, const void *) --> class easy3d::ShaderProgram *", pybind11::return_value_policy::automatic, pybind11::arg("name"), pybind11::arg("value"));
//		cl.def("set_block_uniform", (class easy3d::ShaderProgram * (easy3d::ShaderProgram::*)(const std::string &, const std::string &, const void *)) &easy3d::ShaderProgram::set_block_uniform, "Sets a uniform inside a named block\n Warning: Be careful when using uniform blocks. Please refer to OpenGL Specification Version 4.5\n      (Core Profile) - May 28, 2015 (https://www.opengl.org/registry/doc/glspec45.core.pdf#page=159)\n		In \"Standard Uniform Block Layout\", it says \"If the member is a three-component vector with components\n		consuming N basic machine units, the base alignment is 4N\" (see rule 3 of section 7.6.2.2 in page.159).\n		That means the array stride (the bytes between array elements) is always rounded up to the size of a\n		vec4 (i.e., 16-bytes).\n		So we have to avoid using vec3/mat3 (use vec4/mat4/mat43 instead) in a uniform block. Otherwise,\n		you have to manually pad your structures/arrays out.\n\nC++: easy3d::ShaderProgram::set_block_uniform(const std::string &, const std::string &, const void *) --> class easy3d::ShaderProgram *", pybind11::return_value_policy::automatic, pybind11::arg("blockName"), pybind11::arg("uniformName"), pybind11::arg("value"));
//		cl.def("set_block_uniform_array_element", (class easy3d::ShaderProgram * (easy3d::ShaderProgram::*)(const std::string &, const std::string &, int, const void *)) &easy3d::ShaderProgram::set_block_uniform_array_element, "Sets an element of an array of uniforms inside a block\n\nC++: easy3d::ShaderProgram::set_block_uniform_array_element(const std::string &, const std::string &, int, const void *) --> class easy3d::ShaderProgram *", pybind11::return_value_policy::automatic, pybind11::arg("blockName"), pybind11::arg("uniformName"), pybind11::arg("arrayIndex"), pybind11::arg("value"));
//		cl.def("bind_texture", [](easy3d::ShaderProgram &o, const std::string & a0, unsigned int const & a1, int const & a2) -> easy3d::ShaderProgram * { return o.bind_texture(a0, a1, a2); }, "", pybind11::return_value_policy::automatic, pybind11::arg("name"), pybind11::arg("tex_id"), pybind11::arg("unit"));
//		cl.def("bind_texture", (class easy3d::ShaderProgram * (easy3d::ShaderProgram::*)(const std::string &, unsigned int, int, unsigned int)) &easy3d::ShaderProgram::bind_texture, "C++: easy3d::ShaderProgram::bind_texture(const std::string &, unsigned int, int, unsigned int) --> class easy3d::ShaderProgram *", pybind11::return_value_policy::automatic, pybind11::arg("name"), pybind11::arg("tex_id"), pybind11::arg("unit"), pybind11::arg("tex_target"));
//		cl.def("release_texture", [](easy3d::ShaderProgram &o) -> easy3d::ShaderProgram * { return o.release_texture(); }, "", pybind11::return_value_policy::automatic);
//		cl.def("release_texture", (class easy3d::ShaderProgram * (easy3d::ShaderProgram::*)(unsigned int)) &easy3d::ShaderProgram::release_texture, "C++: easy3d::ShaderProgram::release_texture(unsigned int) --> class easy3d::ShaderProgram *", pybind11::return_value_policy::automatic, pybind11::arg("tex_target"));
//		cl.def("release", (void (easy3d::ShaderProgram::*)() const) &easy3d::ShaderProgram::release, "Ends using the program.\n\nC++: easy3d::ShaderProgram::release() const --> void");
//		cl.def("get_attribute_location", (int (easy3d::ShaderProgram::*)(const std::string &) const) &easy3d::ShaderProgram::get_attribute_location, "C++: easy3d::ShaderProgram::get_attribute_location(const std::string &) const --> int", pybind11::arg("name"));
//		cl.def("is_uniform_used", (bool (easy3d::ShaderProgram::*)(const std::string &)) &easy3d::ShaderProgram::is_uniform_used, "C++: easy3d::ShaderProgram::is_uniform_used(const std::string &) --> bool", pybind11::arg("name"));
//		cl.def("is_attribute_used", (bool (easy3d::ShaderProgram::*)(const std::string &)) &easy3d::ShaderProgram::is_attribute_used, "C++: easy3d::ShaderProgram::is_attribute_used(const std::string &) --> bool", pybind11::arg("name"));
//		cl.def("is_program_valid", (bool (easy3d::ShaderProgram::*)() const) &easy3d::ShaderProgram::is_program_valid, "C++: easy3d::ShaderProgram::is_program_valid() const --> bool");
//		cl.def("is_program_linked", (bool (easy3d::ShaderProgram::*)() const) &easy3d::ShaderProgram::is_program_linked, "Returns true if program linked, false otherwise.\n\nC++: easy3d::ShaderProgram::is_program_linked() const --> bool");
//		cl.def("is_bound", (bool (easy3d::ShaderProgram::*)() const) &easy3d::ShaderProgram::is_bound, "Returns true if the program is being used (i.e., between bind() and  release()).\n\nC++: easy3d::ShaderProgram::is_bound() const --> bool");
//		cl.def("shader_info_log", (bool (easy3d::ShaderProgram::*)(std::string &, unsigned int) const) &easy3d::ShaderProgram::shader_info_log, "C++: easy3d::ShaderProgram::shader_info_log(std::string &, unsigned int) const --> bool", pybind11::arg("log"), pybind11::arg("shader"));
//		cl.def("program_info_log", (bool (easy3d::ShaderProgram::*)(std::string &) const) &easy3d::ShaderProgram::program_info_log, "C++: easy3d::ShaderProgram::program_info_log(std::string &) const --> bool", pybind11::arg("log"));
//		cl.def("print_active_attributes", (void (easy3d::ShaderProgram::*)()) &easy3d::ShaderProgram::print_active_attributes, "C++: easy3d::ShaderProgram::print_active_attributes() --> void");
//		cl.def("print_active_uniforms", (void (easy3d::ShaderProgram::*)()) &easy3d::ShaderProgram::print_active_uniforms, "C++: easy3d::ShaderProgram::print_active_uniforms() --> void");
//		cl.def("print_active_uniform_blocks", (void (easy3d::ShaderProgram::*)()) &easy3d::ShaderProgram::print_active_uniform_blocks, "C++: easy3d::ShaderProgram::print_active_uniform_blocks() --> void");
//		cl.def("load_binary", (bool (easy3d::ShaderProgram::*)(const std::string &)) &easy3d::ShaderProgram::load_binary, "C++: easy3d::ShaderProgram::load_binary(const std::string &) --> bool", pybind11::arg("file_name"));
//		cl.def("save_binary", (bool (easy3d::ShaderProgram::*)(const std::string &)) &easy3d::ShaderProgram::save_binary, "C++: easy3d::ShaderProgram::save_binary(const std::string &) --> bool", pybind11::arg("file_name"));
//		cl.def_static("load_shader_source", [](const std::string & a0) -> std::string { return easy3d::ShaderProgram::load_shader_source(a0); }, "", pybind11::arg("file_name"));
//		cl.def_static("load_shader_source", (std::string (*)(const std::string &, const std::string &)) &easy3d::ShaderProgram::load_shader_source, "Auxiliary function that loads the shader source code from a file.\n \n\n the full path of the file where the source is to be found\n \n\n the include identifier string (e.g., \"#include\")\n\nC++: easy3d::ShaderProgram::load_shader_source(const std::string &, const std::string &) --> std::string", pybind11::arg("file_name"), pybind11::arg("inc_id"));
//	}
}
