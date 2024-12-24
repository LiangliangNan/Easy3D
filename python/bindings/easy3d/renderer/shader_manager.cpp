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
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/shadow.h>
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

// easy3d::Shadow file:easy3d/renderer/shadow.h line:49
struct PyCallBack_easy3d_Shadow : public easy3d::Shadow {
	using easy3d::Shadow::Shadow;

	void ensure_fbo() override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Shadow *>(this), "ensure_fbo");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return Shadow::ensure_fbo();
	}
	void shadow_map_pass(const class std::vector<class easy3d::TrianglesDrawable *> & a0) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Shadow *>(this), "shadow_map_pass");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return Shadow::shadow_map_pass(a0);
	}
	void render_pass(const class std::vector<class easy3d::TrianglesDrawable *> & a0) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Shadow *>(this), "render_pass");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return Shadow::render_pass(a0);
	}
	void update_virtual_background() override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Shadow *>(this), "update_virtual_background");
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

void bind_easy3d_renderer_shader_manager(pybind11::module_& m)
{
	{ // easy3d::ShaderManager file:easy3d/renderer/shader_manager.h line:45
		pybind11::class_<easy3d::ShaderManager, std::shared_ptr<easy3d::ShaderManager>> cl(m, "ShaderManager", "Management of shader programs.\n \n\n\n \n make sure to call terminate() to destroy existing programs before the OpenGL context is deleted.");
		cl.def( pybind11::init( [](){ return new easy3d::ShaderManager(); } ) );
		cl.def_static("get_program", (class easy3d::ShaderProgram * (*)(const std::string &)) &easy3d::ShaderManager::get_program, "C++: easy3d::ShaderManager::get_program(const std::string &) --> class easy3d::ShaderProgram *", pybind11::return_value_policy::automatic, pybind11::arg("shader_name"));
		cl.def_static("create_program_from_files", [](const std::string & a0) -> easy3d::ShaderProgram * { return easy3d::ShaderManager::create_program_from_files(a0); }, "", pybind11::return_value_policy::automatic, pybind11::arg("file_base_name"));
		cl.def_static("create_program_from_files", [](const std::string & a0, const class std::vector<struct std::pair<enum easy3d::ShaderProgram::AttribType, std::string> > & a1) -> easy3d::ShaderProgram * { return easy3d::ShaderManager::create_program_from_files(a0, a1); }, "", pybind11::return_value_policy::automatic, pybind11::arg("file_base_name"), pybind11::arg("attributes"));
		cl.def_static("create_program_from_files", [](const std::string & a0, const class std::vector<struct std::pair<enum easy3d::ShaderProgram::AttribType, std::string> > & a1, const class std::vector<std::string> & a2) -> easy3d::ShaderProgram * { return easy3d::ShaderManager::create_program_from_files(a0, a1, a2); }, "", pybind11::return_value_policy::automatic, pybind11::arg("file_base_name"), pybind11::arg("attributes"), pybind11::arg("outputs"));
		cl.def_static("create_program_from_files", (class easy3d::ShaderProgram * (*)(const std::string &, const class std::vector<struct std::pair<enum easy3d::ShaderProgram::AttribType, std::string> > &, const class std::vector<std::string> &, bool)) &easy3d::ShaderManager::create_program_from_files, "C++: easy3d::ShaderManager::create_program_from_files(const std::string &, const class std::vector<struct std::pair<enum easy3d::ShaderProgram::AttribType, std::string> > &, const class std::vector<std::string> &, bool) --> class easy3d::ShaderProgram *", pybind11::return_value_policy::automatic, pybind11::arg("file_base_name"), pybind11::arg("attributes"), pybind11::arg("outputs"), pybind11::arg("geom_shader"));
		cl.def_static("create_program_from_files", [](const std::string & a0, const std::string & a1) -> easy3d::ShaderProgram * { return easy3d::ShaderManager::create_program_from_files(a0, a1); }, "", pybind11::return_value_policy::automatic, pybind11::arg("vert_file_name"), pybind11::arg("frag_file_name"));
		cl.def_static("create_program_from_files", [](const std::string & a0, const std::string & a1, const std::string & a2) -> easy3d::ShaderProgram * { return easy3d::ShaderManager::create_program_from_files(a0, a1, a2); }, "", pybind11::return_value_policy::automatic, pybind11::arg("vert_file_name"), pybind11::arg("frag_file_name"), pybind11::arg("geom_file_name"));
		cl.def_static("create_program_from_files", [](const std::string & a0, const std::string & a1, const std::string & a2, const std::string & a3) -> easy3d::ShaderProgram * { return easy3d::ShaderManager::create_program_from_files(a0, a1, a2, a3); }, "", pybind11::return_value_policy::automatic, pybind11::arg("vert_file_name"), pybind11::arg("frag_file_name"), pybind11::arg("geom_file_name"), pybind11::arg("extra_vert_code"));
		cl.def_static("create_program_from_files", [](const std::string & a0, const std::string & a1, const std::string & a2, const std::string & a3, const std::string & a4) -> easy3d::ShaderProgram * { return easy3d::ShaderManager::create_program_from_files(a0, a1, a2, a3, a4); }, "", pybind11::return_value_policy::automatic, pybind11::arg("vert_file_name"), pybind11::arg("frag_file_name"), pybind11::arg("geom_file_name"), pybind11::arg("extra_vert_code"), pybind11::arg("extra_frag_code"));
		cl.def_static("create_program_from_files", [](const std::string & a0, const std::string & a1, const std::string & a2, const std::string & a3, const std::string & a4, const std::string & a5) -> easy3d::ShaderProgram * { return easy3d::ShaderManager::create_program_from_files(a0, a1, a2, a3, a4, a5); }, "", pybind11::return_value_policy::automatic, pybind11::arg("vert_file_name"), pybind11::arg("frag_file_name"), pybind11::arg("geom_file_name"), pybind11::arg("extra_vert_code"), pybind11::arg("extra_frag_code"), pybind11::arg("extra_geom_code"));
		cl.def_static("create_program_from_files", [](const std::string & a0, const std::string & a1, const std::string & a2, const std::string & a3, const std::string & a4, const std::string & a5, const class std::vector<struct std::pair<enum easy3d::ShaderProgram::AttribType, std::string> > & a6) -> easy3d::ShaderProgram * { return easy3d::ShaderManager::create_program_from_files(a0, a1, a2, a3, a4, a5, a6); }, "", pybind11::return_value_policy::automatic, pybind11::arg("vert_file_name"), pybind11::arg("frag_file_name"), pybind11::arg("geom_file_name"), pybind11::arg("extra_vert_code"), pybind11::arg("extra_frag_code"), pybind11::arg("extra_geom_code"), pybind11::arg("attributes"));
		cl.def_static("create_program_from_files", (class easy3d::ShaderProgram * (*)(const std::string &, const std::string &, const std::string &, const std::string &, const std::string &, const std::string &, const class std::vector<struct std::pair<enum easy3d::ShaderProgram::AttribType, std::string> > &, const class std::vector<std::string> &)) &easy3d::ShaderManager::create_program_from_files, "C++: easy3d::ShaderManager::create_program_from_files(const std::string &, const std::string &, const std::string &, const std::string &, const std::string &, const std::string &, const class std::vector<struct std::pair<enum easy3d::ShaderProgram::AttribType, std::string> > &, const class std::vector<std::string> &) --> class easy3d::ShaderProgram *", pybind11::return_value_policy::automatic, pybind11::arg("vert_file_name"), pybind11::arg("frag_file_name"), pybind11::arg("geom_file_name"), pybind11::arg("extra_vert_code"), pybind11::arg("extra_frag_code"), pybind11::arg("extra_geom_code"), pybind11::arg("attributes"), pybind11::arg("outputs"));
		cl.def_static("create_program_from_codes", [](const std::string & a0, const std::string & a1) -> easy3d::ShaderProgram * { return easy3d::ShaderManager::create_program_from_codes(a0, a1); }, "", pybind11::return_value_policy::automatic, pybind11::arg("vert_code"), pybind11::arg("frag_code"));
		cl.def_static("create_program_from_codes", [](const std::string & a0, const std::string & a1, const std::string & a2) -> easy3d::ShaderProgram * { return easy3d::ShaderManager::create_program_from_codes(a0, a1, a2); }, "", pybind11::return_value_policy::automatic, pybind11::arg("vert_code"), pybind11::arg("frag_code"), pybind11::arg("geom_code"));
		cl.def_static("create_program_from_codes", [](const std::string & a0, const std::string & a1, const std::string & a2, const class std::vector<struct std::pair<enum easy3d::ShaderProgram::AttribType, std::string> > & a3) -> easy3d::ShaderProgram * { return easy3d::ShaderManager::create_program_from_codes(a0, a1, a2, a3); }, "", pybind11::return_value_policy::automatic, pybind11::arg("vert_code"), pybind11::arg("frag_code"), pybind11::arg("geom_code"), pybind11::arg("attributes"));
		cl.def_static("create_program_from_codes", (class easy3d::ShaderProgram * (*)(const std::string &, const std::string &, const std::string &, const class std::vector<struct std::pair<enum easy3d::ShaderProgram::AttribType, std::string> > &, const class std::vector<std::string> &)) &easy3d::ShaderManager::create_program_from_codes, "C++: easy3d::ShaderManager::create_program_from_codes(const std::string &, const std::string &, const std::string &, const class std::vector<struct std::pair<enum easy3d::ShaderProgram::AttribType, std::string> > &, const class std::vector<std::string> &) --> class easy3d::ShaderProgram *", pybind11::return_value_policy::automatic, pybind11::arg("vert_code"), pybind11::arg("frag_code"), pybind11::arg("geom_code"), pybind11::arg("attributes"), pybind11::arg("outputs"));
		cl.def_static("all_programs", (class std::vector<class easy3d::ShaderProgram *> (*)()) &easy3d::ShaderManager::all_programs, "C++: easy3d::ShaderManager::all_programs() --> class std::vector<class easy3d::ShaderProgram *>");
		cl.def_static("terminate", (void (*)()) &easy3d::ShaderManager::terminate, "C++: easy3d::ShaderManager::terminate() --> void");
		cl.def_static("reload", (void (*)()) &easy3d::ShaderManager::reload, "C++: easy3d::ShaderManager::reload() --> void");
	}
	{ // easy3d::Shadow file:easy3d/renderer/shadow.h line:49
		pybind11::class_<easy3d::Shadow, std::shared_ptr<easy3d::Shadow>, PyCallBack_easy3d_Shadow> cl(m, "Shadow", "Shadow implements the standard shadow map (hard shadow) algorithm.\n \n");
		cl.def( pybind11::init<class easy3d::Camera *>(), pybind11::arg("cam") );

		cl.def("virtual_background", (bool (easy3d::Shadow::*)() const) &easy3d::Shadow::virtual_background, "Shadow allows to have a virtual background as the shadow receiver. The virtual background plane is\n perpendicular to the light direction and is placed at the far plane of the light frustum.\n \n\n This only works for directional lights.\n\nC++: easy3d::Shadow::virtual_background() const --> bool");
		cl.def("set_virtual_background", (void (easy3d::Shadow::*)(bool)) &easy3d::Shadow::set_virtual_background, "Enable/Disable the virtual background. \n virtual_background()\n\nC++: easy3d::Shadow::set_virtual_background(bool) --> void", pybind11::arg("b"));
		cl.def("virtual_background_color", (const class easy3d::Vec<4, float> & (easy3d::Shadow::*)(const class easy3d::Vec<4, float> &)) &easy3d::Shadow::virtual_background_color, "Query the virtual background color.\n\nC++: easy3d::Shadow::virtual_background_color(const class easy3d::Vec<4, float> &) --> const class easy3d::Vec<4, float> &", pybind11::return_value_policy::automatic, pybind11::arg("c"));
		cl.def("set_virtual_background_color", (void (easy3d::Shadow::*)(const class easy3d::Vec<4, float> &)) &easy3d::Shadow::set_virtual_background_color, "Set the virtual background color.\n\nC++: easy3d::Shadow::set_virtual_background_color(const class easy3d::Vec<4, float> &) --> void", pybind11::arg("c"));
		cl.def("shadow_map_size", (int (easy3d::Shadow::*)() const) &easy3d::Shadow::shadow_map_size, "Query the size of the shadow map. The shadow is assumed to be square.\n\nC++: easy3d::Shadow::shadow_map_size() const --> int");
		cl.def("set_shadow_map_size", (void (easy3d::Shadow::*)(int)) &easy3d::Shadow::set_shadow_map_size, "Set/Change the size of the shadow map. The shadow is assumed to be square. Default: 1024 by 1024.\n\nC++: easy3d::Shadow::set_shadow_map_size(int) --> void", pybind11::arg("size"));
		cl.def("light_distance", (float (easy3d::Shadow::*)() const) &easy3d::Shadow::light_distance, "The distance of the light source to the scene scene (w.r.t the scene radius).\n\nC++: easy3d::Shadow::light_distance() const --> float");
		cl.def("set_light_distance", (void (easy3d::Shadow::*)(float)) &easy3d::Shadow::set_light_distance, "Set/Change the distance of the light source to the scene scene (w.r.t the scene radius). Default value is\n 50 (large enough to mimic the directional light used in the default viewer).\n \n\n Value must be > 1.0.\n\nC++: easy3d::Shadow::set_light_distance(float) --> void", pybind11::arg("dist"));
		cl.def("darkness", (float (easy3d::Shadow::*)() const) &easy3d::Shadow::darkness, "Query the darkness of the shadow region. The values is in [0, 1.0] (0: no shadow at all; 1: completely\n dark/black). \n\n set_darkness(float darkness).\n\nC++: easy3d::Shadow::darkness() const --> float");
		cl.def("set_darkness", (void (easy3d::Shadow::*)(float)) &easy3d::Shadow::set_darkness, "Set the darkness of the shadow region. Values must be in [0, 1.0] (0: no shadow at all; 1: completely\n dark/black). The default value is 0.6.\n\nC++: easy3d::Shadow::set_darkness(float) --> void", pybind11::arg("darkness"));
		cl.def("draw", (void (easy3d::Shadow::*)(const class std::vector<class easy3d::TrianglesDrawable *> &)) &easy3d::Shadow::draw, "Rendering the surfaces.\n\nC++: easy3d::Shadow::draw(const class std::vector<class easy3d::TrianglesDrawable *> &) --> void", pybind11::arg("surfaces"));
	}
}
