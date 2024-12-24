#include <easy3d/core/mat.h>
#include <easy3d/core/vec.h>
#include <easy3d/core/model.h>
#include <easy3d/renderer/ambient_occlusion.h>
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

// easy3d::AmbientOcclusion file:easy3d/renderer/ambient_occlusion.h line:49
struct PyCallBack_easy3d_AmbientOcclusion : public easy3d::AmbientOcclusion {
	using easy3d::AmbientOcclusion::AmbientOcclusion;

	unsigned int generate(const class std::vector<class easy3d::Model *> & a0) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::AmbientOcclusion *>(this), "generate");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
			if (pybind11::detail::cast_is_temporary_value_reference<unsigned int>::value) {
				static pybind11::detail::override_caster_t<unsigned int> caster;
				return pybind11::detail::cast_ref<unsigned int>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<unsigned int>(std::move(o));
		}
		return AmbientOcclusion::generate(a0);
	}
};

void bind_easy3d_renderer_ambient_occlusion(pybind11::module_& m)
{
	{ // easy3d::AmbientOcclusion file:easy3d/renderer/ambient_occlusion.h line:49
		pybind11::class_<easy3d::AmbientOcclusion, std::shared_ptr<easy3d::AmbientOcclusion>, PyCallBack_easy3d_AmbientOcclusion> cl(m, "AmbientOcclusion", "Traditional Screen Space Ambient Occlusion (SSAO) technique.\n \n\n\n \n Optimization tip: rendering with multi-effects (e.g., shadowing, SSAO) can benefit from using a shared\n geometry pass.");
		cl.def("set_radius", (void (easy3d::AmbientOcclusion::*)(float)) &easy3d::AmbientOcclusion::set_radius, "Sets the sample radius (in pixels). Typical value is in range [0, 4].\n\nC++: easy3d::AmbientOcclusion::set_radius(float) --> void", pybind11::arg("r"));
		cl.def("radius", (float (easy3d::AmbientOcclusion::*)() const) &easy3d::AmbientOcclusion::radius, "Returns the sample radius.\n\nC++: easy3d::AmbientOcclusion::radius() const --> float");
		cl.def("set_bias", (void (easy3d::AmbientOcclusion::*)(float)) &easy3d::AmbientOcclusion::set_bias, "Sets the bias. Default value is 0.005.\n\nC++: easy3d::AmbientOcclusion::set_bias(float) --> void", pybind11::arg("b"));
		cl.def("bias", (float (easy3d::AmbientOcclusion::*)() const) &easy3d::AmbientOcclusion::bias, "Returns the bias.\n\nC++: easy3d::AmbientOcclusion::bias() const --> float");
		cl.def("generate", (unsigned int (easy3d::AmbientOcclusion::*)(const class std::vector<class easy3d::Model *> &)) &easy3d::AmbientOcclusion::generate, "Generates the SSAO texture\n \n\n The SSAO texture ID\n\nC++: easy3d::AmbientOcclusion::generate(const class std::vector<class easy3d::Model *> &) --> unsigned int", pybind11::arg("models"));
		cl.def("ssao_texture", (unsigned int (easy3d::AmbientOcclusion::*)() const) &easy3d::AmbientOcclusion::ssao_texture, "Returns the generated SSAO texture ID\n\nC++: easy3d::AmbientOcclusion::ssao_texture() const --> unsigned int");
	}

}
