#include <easy3d/core/random.h>
#include <easy3d/core/vec.h>

#include <memory>
#include <pybind11/pybind11.h>


#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif


void bind_easy3d_core_random(pybind11::module_& m)
{
	// easy3d::random_float() file:easy3d/core/random.h line:39
	m.def("random_float", (float (*)()) &easy3d::random_float, "Random real in [0, 1]. \n\nC++: easy3d::random_float() --> float");

	// easy3d::random_float(float, float) file:easy3d/core/random.h line:44
	m.def("random_float", (float (*)(float, float)) &easy3d::random_float, "Random real number in the range [min, max]. \n\nC++: easy3d::random_float(float, float) --> float", pybind11::arg("min"), pybind11::arg("max"));

	// easy3d::random_color(bool) file:easy3d/core/random.h line:49
	m.def("random_color", []() -> easy3d::Vec<3, float> { return easy3d::random_color(); }, "");
	m.def("random_color", (class easy3d::Vec<3, float> (*)(bool)) &easy3d::random_color, "Generates a random color. The parameter  controls if too dark colors are allowed. \n\nC++: easy3d::random_color(bool) --> class easy3d::Vec<3, float>", pybind11::arg("allow_dark"));
}
