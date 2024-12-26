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
}
