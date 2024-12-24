#include <easy3d/core/mat.h>
#include <easy3d/core/vec.h>
#include <easy3d/renderer/shadow.h>
#include <easy3d/renderer/soft_shadow.h>
#include <easy3d/renderer/drawable_triangles.h>

#include <memory>
#include <sstream> // __str__
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

void bind_easy3d_renderer_shadow(pybind11::module_& m)
{
	{ // easy3d::Shadow file:easy3d/renderer/shadow.h line:49
		pybind11::class_<easy3d::Shadow, std::shared_ptr<easy3d::Shadow>, PyCallBack_easy3d_Shadow> cl(m, "Shadow", "Shadow implements the standard shadow map (hard shadow) algorithm.\n \n");
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
	{ // easy3d::SoftShadow file:easy3d/renderer/soft_shadow.h line:69
		pybind11::class_<easy3d::SoftShadow, std::shared_ptr<easy3d::SoftShadow>, PyCallBack_easy3d_SoftShadow, easy3d::Shadow> cl(m, "SoftShadow", "An implementation of the Percentage-Closer Soft Shadows\n\n \n\n \n SoftShadow implements the Percentage-Closer Soft Shadows technique. See\n  - http://developer.download.nvidia.com/shaderlibrary/docs/shadow_PCSS.pdf\n  - https://http.download.nvidia.com/developer/presentations/2005/SIGGRAPH/Percentage_Closer_Soft_Shadows.pdf\n\n --- Overview ---\n This sample demonstrates two techniques for rendering soft shadows: Percentage Closer Filtering (PCS) with\n a uniform kernel size and Percentage-Closer Soft Shadows (PCSS) with a variable kernel size.\n This sample is assuming the shadow map is a regular depth buffer (non-linear depths), and the shadow-map\n is generated using a perspective projection with the center of projection at the center of the area light\n simulated by PCSS. The sample uses a gradient-based depth bias to scale the depth bias based on the uv-space\n distance from the center of the kernel. To avoid self-shadowing artifacts, an additional depth bias may also\n been applied while writing depth into the shadow map.\n\n --- Percentage Closer Filtering (PCF) ---\n The Percentage Closer Filtering technique uses a uniform size filter kernel, resulting in shadows with a\n uniformly soft edge. The PCF algorithm is very straightforward and fast, but does not achieve a realistic\n contact hardening effect. Also, while the kernel size and penumbra width is uniform, the kernel can be\n scaled to accommodate different sized lights.\n\n --- Percentage-Closer Soft Shadows (PCSS) ---\n Soft shadows are typically rendered in games by using shadow mapping and Percentage Closer Filtering with\n a uniform kernel size. The Percentage-Closer Soft Shadows (PCSS) algorithm computes a variable kernel size\n based on the distance between the relative position of the receiver point, an approximation of the blocker,\n and the area light. It produces perceptually plausible soft shadows that harden on contact.\n\n Optimization tips: rendering with multi-effects (e.g., shadowing, SSAO) can benefit from sharing the same\n geometry pass.");

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

}
