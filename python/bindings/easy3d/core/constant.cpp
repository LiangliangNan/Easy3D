#include <easy3d/core/constant.h>

#include <pybind11/pybind11.h>
#include <string>


#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_core_constant(pybind11::module_& m)
{
	// easy3d::min() file:easy3d/core/constant.h line:50
	m.def("min", (int (*)()) &easy3d::min<int>, "Function returning  for int type numbers.\n\nC++: easy3d::min() --> int");

	// easy3d::max() file:easy3d/core/constant.h line:52
	m.def("max", (int (*)()) &easy3d::max<int>, "Function returning  for int type numbers.\n\nC++: easy3d::max() --> int");

	// easy3d::min() file:easy3d/core/constant.h line:54
	m.def("min", (float (*)()) &easy3d::min<float>, "Function returning  for float type numbers.\n\nC++: easy3d::min() --> float");

	// easy3d::max() file:easy3d/core/constant.h line:56
	m.def("max", (float (*)()) &easy3d::max<float>, "Function returning  for float type numbers.\n\nC++: easy3d::max() --> float");

	// easy3d::min() file:easy3d/core/constant.h line:58
	m.def("min", (double (*)()) &easy3d::min<double>, "Function returning  for double type numbers.\n\nC++: easy3d::min() --> double");

	// easy3d::max() file:easy3d/core/constant.h line:60
	m.def("max", (double (*)()) &easy3d::max<double>, "Function returning  for double type numbers.\n\nC++: easy3d::max() --> double");

	// easy3d::epsilon() file:easy3d/core/constant.h line:69
	m.def("epsilon", (float (*)()) &easy3d::epsilon<float>, "Function returning  for float type numbers.\n\nC++: easy3d::epsilon() --> float");

	// easy3d::epsilon_sqr() file:easy3d/core/constant.h line:71
	m.def("epsilon_sqr", (float (*)()) &easy3d::epsilon_sqr<float>, "Function returning  epsilon for float type numbers.\n\nC++: easy3d::epsilon_sqr() --> float");

	// easy3d::epsilon() file:easy3d/core/constant.h line:73
	m.def("epsilon", (double (*)()) &easy3d::epsilon<double>, "Function returning  for double type numbers.\n\nC++: easy3d::epsilon() --> double");

	// easy3d::epsilon_sqr() file:easy3d/core/constant.h line:75
	m.def("epsilon_sqr", (double (*)()) &easy3d::epsilon_sqr<double>, "Function returning  epsilon for double type numbers.\n\nC++: easy3d::epsilon_sqr() --> double");

}
