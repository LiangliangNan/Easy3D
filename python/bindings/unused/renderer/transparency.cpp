#include <easy3d/renderer/average_color_blending.h>
#include <easy3d/renderer/dual_depth_peeling.h>
#include <easy3d/renderer/transparency.h>
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

// easy3d::Transparency file:easy3d/renderer/transparency.h line:43
struct PyCallBack_easy3d_Transparency : public easy3d::Transparency {
	using easy3d::Transparency::Transparency;

	void draw(const class std::vector<class easy3d::TrianglesDrawable *> & a0) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Transparency *>(this), "draw");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		pybind11::pybind11_fail("Tried to call pure virtual function \"Transparency::draw\"");
	}
};

// easy3d::DualDepthPeeling file:easy3d/renderer/dual_depth_peeling.h line:81
struct PyCallBack_easy3d_DualDepthPeeling : public easy3d::DualDepthPeeling {
	using easy3d::DualDepthPeeling::DualDepthPeeling;

	void draw(const class std::vector<class easy3d::TrianglesDrawable *> & a0) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::DualDepthPeeling *>(this), "draw");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return DualDepthPeeling::draw(a0);
	}
};

// easy3d::AverageColorBlending file:easy3d/renderer/average_color_blending.h line:48
struct PyCallBack_easy3d_AverageColorBlending : public easy3d::AverageColorBlending {
	using easy3d::AverageColorBlending::AverageColorBlending;

	void draw(const class std::vector<class easy3d::TrianglesDrawable *> & a0) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::AverageColorBlending *>(this), "draw");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return AverageColorBlending::draw(a0);
	}
};

void bind_easy3d_renderer_transparency(pybind11::module_& m)
{
	{ // easy3d::Transparency file:easy3d/renderer/transparency.h line:43
		pybind11::class_<easy3d::Transparency, std::shared_ptr<easy3d::Transparency>, PyCallBack_easy3d_Transparency> cl(m, "Transparency", "Base class for rendering with transparency\n \n\n\n \n AverageColorBlending, DualDepthPeeling");
		cl.def( pybind11::init( [](){ return new PyCallBack_easy3d_Transparency(); } ) );
		cl.def("draw", (void (easy3d::Transparency::*)(const class std::vector<class easy3d::TrianglesDrawable *> &)) &easy3d::Transparency::draw, "Renders the scene (a set of surfaces) with transparency effect\n\nC++: easy3d::Transparency::draw(const class std::vector<class easy3d::TrianglesDrawable *> &) --> void", pybind11::arg("surfaces"));
		cl.def("assign", (class easy3d::Transparency & (easy3d::Transparency::*)(const class easy3d::Transparency &)) &easy3d::Transparency::operator=, "C++: easy3d::Transparency::operator=(const class easy3d::Transparency &) --> class easy3d::Transparency &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}
	{ // easy3d::DualDepthPeeling file:easy3d/renderer/dual_depth_peeling.h line:81
		pybind11::class_<easy3d::DualDepthPeeling, std::shared_ptr<easy3d::DualDepthPeeling>, PyCallBack_easy3d_DualDepthPeeling, easy3d::Transparency> cl(m, "DualDepthPeeling", "");
		cl.def("set_max_peels", (void (easy3d::DualDepthPeeling::*)(int)) &easy3d::DualDepthPeeling::set_max_peels, "Sets the maximum number of peeling layers.\n \n\n It has to be a positive value. Default value is 4. A special value of 0 means no maximum limit.\n\nC++: easy3d::DualDepthPeeling::set_max_peels(int) --> void", pybind11::arg("n"));
		cl.def("max_peels", (int (easy3d::DualDepthPeeling::*)() const) &easy3d::DualDepthPeeling::max_peels, "Returns the maximum number of peeling layers.\n\nC++: easy3d::DualDepthPeeling::max_peels() const --> int");
		cl.def("draw", (void (easy3d::DualDepthPeeling::*)(const class std::vector<class easy3d::TrianglesDrawable *> &)) &easy3d::DualDepthPeeling::draw, "Renders the scene (a set of surfaces) with transparency effect\n\nC++: easy3d::DualDepthPeeling::draw(const class std::vector<class easy3d::TrianglesDrawable *> &) --> void", pybind11::arg("surfaces"));
	}
	{ // easy3d::AverageColorBlending file:easy3d/renderer/average_color_blending.h line:48
		pybind11::class_<easy3d::AverageColorBlending, std::shared_ptr<easy3d::AverageColorBlending>, PyCallBack_easy3d_AverageColorBlending, easy3d::Transparency> cl(m, "AverageColorBlending", "Transparency effect using average color blending.\n\n \n Optimization tip: rendering with multi-effects (e.g., shadowing, SSAO) can benefit from using a shared\n geometry pass.\n\n \n\n     ");
		cl.def("draw", (void (easy3d::AverageColorBlending::*)(const class std::vector<class easy3d::TrianglesDrawable *> &)) &easy3d::AverageColorBlending::draw, "Renders the scene (a set of surfaces) with transparency effect\n\nC++: easy3d::AverageColorBlending::draw(const class std::vector<class easy3d::TrianglesDrawable *> &) --> void", pybind11::arg("surfaces"));
	}
}
