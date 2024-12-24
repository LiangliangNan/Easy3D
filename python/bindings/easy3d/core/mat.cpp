#include <easy3d/core/box.h>
#include <easy3d/core/mat.h>
#include <easy3d/core/types.h>
#include <easy3d/core/vec.h>

#include <pybind11/pybind11.h>
#include <string>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_core_mat(pybind11::module_& m)
{
	{ // easy3d::Mat file:easy3d/core/mat.h line:65
		pybind11::class_<easy3d::Mat<3UL,3UL,float>, std::shared_ptr<easy3d::Mat<3UL,3UL,float>>> cl(m, "Mat_3UL_3UL_float_t", "");
		cl.def( pybind11::init( [](){ return new easy3d::Mat<3UL,3UL,float>(); } ) );
		cl.def( pybind11::init<float>(), pybind11::arg("s") );

		cl.def( pybind11::init<const float *>(), pybind11::arg("m") );

		cl.def( pybind11::init( [](easy3d::Mat<3UL,3UL,float> const &o){ return new easy3d::Mat<3UL,3UL,float>(o); } ) );
		cl.def_static("identity", (class easy3d::Mat<3, 3, float> (*)()) &easy3d::Mat<3, 3, float>::identity, "C++: easy3d::Mat<3, 3, float>::identity() --> class easy3d::Mat<3, 3, float>");
		cl.def("num_rows", (unsigned long (easy3d::Mat<3UL,3UL,float>::*)() const) &easy3d::Mat<3, 3, float>::num_rows, "C++: easy3d::Mat<3, 3, float>::num_rows() const --> unsigned long");
		cl.def("num_columns", (unsigned long (easy3d::Mat<3UL,3UL,float>::*)() const) &easy3d::Mat<3, 3, float>::num_columns, "C++: easy3d::Mat<3, 3, float>::num_columns() const --> unsigned long");
		cl.def("row", (class easy3d::Vec<3, float> (easy3d::Mat<3UL,3UL,float>::*)(unsigned long) const) &easy3d::Mat<3, 3, float>::row, "C++: easy3d::Mat<3, 3, float>::row(unsigned long) const --> class easy3d::Vec<3, float>", pybind11::arg("r"));
		cl.def("col", (class easy3d::Vec<3, float> (easy3d::Mat<3UL,3UL,float>::*)(unsigned long) const) &easy3d::Mat<3, 3, float>::col, "C++: easy3d::Mat<3, 3, float>::col(unsigned long) const --> class easy3d::Vec<3, float>", pybind11::arg("c"));
		cl.def("__call__", (float & (easy3d::Mat<3UL,3UL,float>::*)(unsigned long, unsigned long)) &easy3d::Mat<3, 3, float>::operator(), "C++: easy3d::Mat<3, 3, float>::operator()(unsigned long, unsigned long) --> float &", pybind11::return_value_policy::automatic, pybind11::arg("row"), pybind11::arg("col"));
		cl.def("load_zero", (void (easy3d::Mat<3UL,3UL,float>::*)()) &easy3d::Mat<3, 3, float>::load_zero, "C++: easy3d::Mat<3, 3, float>::load_zero() --> void");
		cl.def("load_identity", [](easy3d::Mat<3UL,3UL,float> &o) -> void { return o.load_identity(); }, "");
		cl.def("load_identity", (void (easy3d::Mat<3UL,3UL,float>::*)(float)) &easy3d::Mat<3, 3, float>::load_identity, "C++: easy3d::Mat<3, 3, float>::load_identity(float) --> void", pybind11::arg("s"));
		cl.def("swap_rows", (void (easy3d::Mat<3UL,3UL,float>::*)(unsigned long, unsigned long)) &easy3d::Mat<3, 3, float>::swap_rows, "C++: easy3d::Mat<3, 3, float>::swap_rows(unsigned long, unsigned long) --> void", pybind11::arg("a"), pybind11::arg("b"));
		cl.def("swap_cols", (void (easy3d::Mat<3UL,3UL,float>::*)(unsigned long, unsigned long)) &easy3d::Mat<3, 3, float>::swap_cols, "C++: easy3d::Mat<3, 3, float>::swap_cols(unsigned long, unsigned long) --> void", pybind11::arg("a"), pybind11::arg("b"));
		cl.def("__eq__", (bool (easy3d::Mat<3UL,3UL,float>::*)(const class easy3d::Mat<3, 3, float> &) const) &easy3d::Mat<3, 3, float>::operator==, "C++: easy3d::Mat<3, 3, float>::operator==(const class easy3d::Mat<3, 3, float> &) const --> bool", pybind11::arg("rhs"));
		cl.def("__ne__", (bool (easy3d::Mat<3UL,3UL,float>::*)(const class easy3d::Mat<3, 3, float> &) const) &easy3d::Mat<3, 3, float>::operator!=, "C++: easy3d::Mat<3, 3, float>::operator!=(const class easy3d::Mat<3, 3, float> &) const --> bool", pybind11::arg("rhs"));
		cl.def("__add__", (class easy3d::Mat<3, 3, float> (easy3d::Mat<3UL,3UL,float>::*)(const class easy3d::Mat<3, 3, float> &) const) &easy3d::Mat<3, 3, float>::operator+, "C++: easy3d::Mat<3, 3, float>::operator+(const class easy3d::Mat<3, 3, float> &) const --> class easy3d::Mat<3, 3, float>", pybind11::arg("rhs"));
		cl.def("__sub__", (class easy3d::Mat<3, 3, float> (easy3d::Mat<3UL,3UL,float>::*)(const class easy3d::Mat<3, 3, float> &) const) &easy3d::Mat<3, 3, float>::operator-, "C++: easy3d::Mat<3, 3, float>::operator-(const class easy3d::Mat<3, 3, float> &) const --> class easy3d::Mat<3, 3, float>", pybind11::arg("rhs"));
		cl.def("__neg__", (class easy3d::Mat<3, 3, float> (easy3d::Mat<3UL,3UL,float>::*)() const) &easy3d::Mat<3, 3, float>::operator-, "C++: easy3d::Mat<3, 3, float>::operator-() const --> class easy3d::Mat<3, 3, float>");
		cl.def("__mul__", (class easy3d::Vec<3, float> (easy3d::Mat<3UL,3UL,float>::*)(const class easy3d::Vec<3, float> &) const) &easy3d::Mat<3, 3, float>::operator*, "C++: easy3d::Mat<3, 3, float>::operator*(const class easy3d::Vec<3, float> &) const --> class easy3d::Vec<3, float>", pybind11::arg("rhs"));
		cl.def("__mul__", (class easy3d::Mat<3, 3, float> (easy3d::Mat<3UL,3UL,float>::*)(float) const) &easy3d::Mat<3, 3, float>::operator*, "C++: easy3d::Mat<3, 3, float>::operator*(float) const --> class easy3d::Mat<3, 3, float>", pybind11::arg("rhs"));
		cl.def("__truediv__", (class easy3d::Mat<3, 3, float> (easy3d::Mat<3UL,3UL,float>::*)(float) const) &easy3d::Mat<3, 3, float>::operator/, "C++: easy3d::Mat<3, 3, float>::operator/(float) const --> class easy3d::Mat<3, 3, float>", pybind11::arg("rhs"));
		cl.def("__imul__", (class easy3d::Mat<3, 3, float> & (easy3d::Mat<3UL,3UL,float>::*)(const class easy3d::Mat<3, 3, float> &)) &easy3d::Mat<3, 3, float>::operator*=, "C++: easy3d::Mat<3, 3, float>::operator*=(const class easy3d::Mat<3, 3, float> &) --> class easy3d::Mat<3, 3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("rhs"));
		cl.def("__iadd__", (class easy3d::Mat<3, 3, float> & (easy3d::Mat<3UL,3UL,float>::*)(const class easy3d::Mat<3, 3, float> &)) &easy3d::Mat<3, 3, float>::operator+=, "C++: easy3d::Mat<3, 3, float>::operator+=(const class easy3d::Mat<3, 3, float> &) --> class easy3d::Mat<3, 3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("rhs"));
		cl.def("__isub__", (class easy3d::Mat<3, 3, float> & (easy3d::Mat<3UL,3UL,float>::*)(const class easy3d::Mat<3, 3, float> &)) &easy3d::Mat<3, 3, float>::operator-=, "C++: easy3d::Mat<3, 3, float>::operator-=(const class easy3d::Mat<3, 3, float> &) --> class easy3d::Mat<3, 3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("rhs"));
		cl.def("__imul__", (class easy3d::Mat<3, 3, float> & (easy3d::Mat<3UL,3UL,float>::*)(float)) &easy3d::Mat<3, 3, float>::operator*=, "C++: easy3d::Mat<3, 3, float>::operator*=(float) --> class easy3d::Mat<3, 3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("rhs"));
		cl.def("__itruediv__", (class easy3d::Mat<3, 3, float> & (easy3d::Mat<3UL,3UL,float>::*)(float)) &easy3d::Mat<3, 3, float>::operator/=, "C++: easy3d::Mat<3, 3, float>::operator/=(float) --> class easy3d::Mat<3, 3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("rhs"));
		cl.def("__iadd__", (class easy3d::Mat<3, 3, float> & (easy3d::Mat<3UL,3UL,float>::*)(float)) &easy3d::Mat<3, 3, float>::operator+=, "C++: easy3d::Mat<3, 3, float>::operator+=(float) --> class easy3d::Mat<3, 3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("rhs"));
		cl.def("__isub__", (class easy3d::Mat<3, 3, float> & (easy3d::Mat<3UL,3UL,float>::*)(float)) &easy3d::Mat<3, 3, float>::operator-=, "C++: easy3d::Mat<3, 3, float>::operator-=(float) --> class easy3d::Mat<3, 3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("rhs"));
	}
	// easy3d::determinant(const class easy3d::Mat<3, 3, float> &) file:easy3d/core/mat.h line:930
	m.def("determinant", (float (*)(const class easy3d::Mat<3, 3, float> &)) &easy3d::determinant<3UL,float>, "C++: easy3d::determinant(const class easy3d::Mat<3, 3, float> &) --> float", pybind11::arg("m"));

	// easy3d::determinant(const class easy3d::Mat3<float> &) file:easy3d/core/mat.h line:949
	m.def("determinant", (float (*)(const class easy3d::Mat3<float> &)) &easy3d::determinant<float>, "C++: easy3d::determinant(const class easy3d::Mat3<float> &) --> float", pybind11::arg("m"));

	// easy3d::next_pow2(int) file:easy3d/core/types.h line:143
	m.def("next_pow2", (int (*)(int)) &easy3d::next_pow2, "Calculates the next larger power of 2. If the input is already a power of 2, it will return itself.\n \n\n The starting point for finding the next power of 2.\n \n\n value^2.\n Example:\n      next_pow2(50);  // returns 64\n      next_pow2(64);  // returns 64\n      next_pow2(401); // returns 512\n\nC++: easy3d::next_pow2(int) --> int", pybind11::arg("a"));

    { // easy3d::Mat3 file:easy3d/core/mat.h line:1628
        pybind11::class_<easy3d::Mat3<float>, std::shared_ptr<easy3d::Mat3<float>>, easy3d::Mat<3UL,3UL,float>> cl(m, "Mat3_float_t", "");
        cl.def( pybind11::init( [](){ return new easy3d::Mat3<float>(); } ) );
        cl.def( pybind11::init<float>(), pybind11::arg("s") );

        cl.def( pybind11::init<const class easy3d::Mat<3, 3, float> &>(), pybind11::arg("rhs") );

        cl.def( pybind11::init<float, float, float, float, float, float, float, float, float>(), pybind11::arg("s00"), pybind11::arg("s01"), pybind11::arg("s02"), pybind11::arg("s10"), pybind11::arg("s11"), pybind11::arg("s12"), pybind11::arg("s20"), pybind11::arg("s21"), pybind11::arg("s22") );

        cl.def( pybind11::init<const float *>(), pybind11::arg("m") );

        cl.def( pybind11::init<const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &>(), pybind11::arg("x"), pybind11::arg("y"), pybind11::arg("z") );

        cl.def( pybind11::init( [](easy3d::Mat3<float> const &o){ return new easy3d::Mat3<float>(o); } ) );
        cl.def_static("scale", (class easy3d::Mat3<float> (*)(float)) &easy3d::Mat3<float>::scale, "C++: easy3d::Mat3<float>::scale(float) --> class easy3d::Mat3<float>", pybind11::arg("s"));
        cl.def_static("scale", (class easy3d::Mat3<float> (*)(float, float, float)) &easy3d::Mat3<float>::scale, "C++: easy3d::Mat3<float>::scale(float, float, float) --> class easy3d::Mat3<float>", pybind11::arg("x"), pybind11::arg("y"), pybind11::arg("z"));
        cl.def_static("rotation", (class easy3d::Mat3<float> (*)(const class easy3d::Vec<3, float> &, float)) &easy3d::Mat3<float>::rotation, "C++: easy3d::Mat3<float>::rotation(const class easy3d::Vec<3, float> &, float) --> class easy3d::Mat3<float>", pybind11::arg("axis"), pybind11::arg("angle"));
        cl.def_static("rotation", (class easy3d::Mat3<float> (*)(const class easy3d::Vec<3, float> &)) &easy3d::Mat3<float>::rotation, "C++: easy3d::Mat3<float>::rotation(const class easy3d::Vec<3, float> &) --> class easy3d::Mat3<float>", pybind11::arg("axis_angle"));
        cl.def_static("rotation", [](float const & a0, float const & a1, float const & a2) -> easy3d::Mat3<float> { return easy3d::Mat3<float>::rotation(a0, a1, a2); }, "", pybind11::arg("x"), pybind11::arg("y"), pybind11::arg("z"));
        cl.def_static("rotation", (class easy3d::Mat3<float> (*)(float, float, float, int)) &easy3d::Mat3<float>::rotation, "C++: easy3d::Mat3<float>::rotation(float, float, float, int) --> class easy3d::Mat3<float>", pybind11::arg("x"), pybind11::arg("y"), pybind11::arg("z"), pybind11::arg("order"));
        cl.def_static("identity", (class easy3d::Mat<3, 3, float> (*)()) &easy3d::Mat<3, 3, float>::identity, "C++: easy3d::Mat<3, 3, float>::identity() --> class easy3d::Mat<3, 3, float>");
        cl.def("num_rows", (unsigned long (easy3d::Mat<3UL,3UL,float>::*)() const) &easy3d::Mat<3, 3, float>::num_rows, "C++: easy3d::Mat<3, 3, float>::num_rows() const --> unsigned long");
        cl.def("num_columns", (unsigned long (easy3d::Mat<3UL,3UL,float>::*)() const) &easy3d::Mat<3, 3, float>::num_columns, "C++: easy3d::Mat<3, 3, float>::num_columns() const --> unsigned long");
        cl.def("row", (class easy3d::Vec<3, float> (easy3d::Mat<3UL,3UL,float>::*)(unsigned long) const) &easy3d::Mat<3, 3, float>::row, "C++: easy3d::Mat<3, 3, float>::row(unsigned long) const --> class easy3d::Vec<3, float>", pybind11::arg("r"));
        cl.def("col", (class easy3d::Vec<3, float> (easy3d::Mat<3UL,3UL,float>::*)(unsigned long) const) &easy3d::Mat<3, 3, float>::col, "C++: easy3d::Mat<3, 3, float>::col(unsigned long) const --> class easy3d::Vec<3, float>", pybind11::arg("c"));
        cl.def("__call__", (float & (easy3d::Mat<3UL,3UL,float>::*)(unsigned long, unsigned long)) &easy3d::Mat<3, 3, float>::operator(), "C++: easy3d::Mat<3, 3, float>::operator()(unsigned long, unsigned long) --> float &", pybind11::return_value_policy::automatic, pybind11::arg("row"), pybind11::arg("col"));
        cl.def("load_zero", (void (easy3d::Mat<3UL,3UL,float>::*)()) &easy3d::Mat<3, 3, float>::load_zero, "C++: easy3d::Mat<3, 3, float>::load_zero() --> void");
        cl.def("load_identity", [](easy3d::Mat<3UL,3UL,float> &o) -> void { return o.load_identity(); }, "");
        cl.def("load_identity", (void (easy3d::Mat<3UL,3UL,float>::*)(float)) &easy3d::Mat<3, 3, float>::load_identity, "C++: easy3d::Mat<3, 3, float>::load_identity(float) --> void", pybind11::arg("s"));
        cl.def("swap_rows", (void (easy3d::Mat<3UL,3UL,float>::*)(unsigned long, unsigned long)) &easy3d::Mat<3, 3, float>::swap_rows, "C++: easy3d::Mat<3, 3, float>::swap_rows(unsigned long, unsigned long) --> void", pybind11::arg("a"), pybind11::arg("b"));
        cl.def("swap_cols", (void (easy3d::Mat<3UL,3UL,float>::*)(unsigned long, unsigned long)) &easy3d::Mat<3, 3, float>::swap_cols, "C++: easy3d::Mat<3, 3, float>::swap_cols(unsigned long, unsigned long) --> void", pybind11::arg("a"), pybind11::arg("b"));
        cl.def("__eq__", (bool (easy3d::Mat<3UL,3UL,float>::*)(const class easy3d::Mat<3, 3, float> &) const) &easy3d::Mat<3, 3, float>::operator==, "C++: easy3d::Mat<3, 3, float>::operator==(const class easy3d::Mat<3, 3, float> &) const --> bool", pybind11::arg("rhs"));
        cl.def("__ne__", (bool (easy3d::Mat<3UL,3UL,float>::*)(const class easy3d::Mat<3, 3, float> &) const) &easy3d::Mat<3, 3, float>::operator!=, "C++: easy3d::Mat<3, 3, float>::operator!=(const class easy3d::Mat<3, 3, float> &) const --> bool", pybind11::arg("rhs"));
        cl.def("__add__", (class easy3d::Mat<3, 3, float> (easy3d::Mat<3UL,3UL,float>::*)(const class easy3d::Mat<3, 3, float> &) const) &easy3d::Mat<3, 3, float>::operator+, "C++: easy3d::Mat<3, 3, float>::operator+(const class easy3d::Mat<3, 3, float> &) const --> class easy3d::Mat<3, 3, float>", pybind11::arg("rhs"));
        cl.def("__sub__", (class easy3d::Mat<3, 3, float> (easy3d::Mat<3UL,3UL,float>::*)(const class easy3d::Mat<3, 3, float> &) const) &easy3d::Mat<3, 3, float>::operator-, "C++: easy3d::Mat<3, 3, float>::operator-(const class easy3d::Mat<3, 3, float> &) const --> class easy3d::Mat<3, 3, float>", pybind11::arg("rhs"));
        cl.def("__neg__", (class easy3d::Mat<3, 3, float> (easy3d::Mat<3UL,3UL,float>::*)() const) &easy3d::Mat<3, 3, float>::operator-, "C++: easy3d::Mat<3, 3, float>::operator-() const --> class easy3d::Mat<3, 3, float>");
        cl.def("__mul__", (class easy3d::Vec<3, float> (easy3d::Mat<3UL,3UL,float>::*)(const class easy3d::Vec<3, float> &) const) &easy3d::Mat<3, 3, float>::operator*, "C++: easy3d::Mat<3, 3, float>::operator*(const class easy3d::Vec<3, float> &) const --> class easy3d::Vec<3, float>", pybind11::arg("rhs"));
        cl.def("__mul__", (class easy3d::Mat<3, 3, float> (easy3d::Mat<3UL,3UL,float>::*)(float) const) &easy3d::Mat<3, 3, float>::operator*, "C++: easy3d::Mat<3, 3, float>::operator*(float) const --> class easy3d::Mat<3, 3, float>", pybind11::arg("rhs"));
        cl.def("__truediv__", (class easy3d::Mat<3, 3, float> (easy3d::Mat<3UL,3UL,float>::*)(float) const) &easy3d::Mat<3, 3, float>::operator/, "C++: easy3d::Mat<3, 3, float>::operator/(float) const --> class easy3d::Mat<3, 3, float>", pybind11::arg("rhs"));
        cl.def("__imul__", (class easy3d::Mat<3, 3, float> & (easy3d::Mat<3UL,3UL,float>::*)(const class easy3d::Mat<3, 3, float> &)) &easy3d::Mat<3, 3, float>::operator*=, "C++: easy3d::Mat<3, 3, float>::operator*=(const class easy3d::Mat<3, 3, float> &) --> class easy3d::Mat<3, 3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("rhs"));
        cl.def("__iadd__", (class easy3d::Mat<3, 3, float> & (easy3d::Mat<3UL,3UL,float>::*)(const class easy3d::Mat<3, 3, float> &)) &easy3d::Mat<3, 3, float>::operator+=, "C++: easy3d::Mat<3, 3, float>::operator+=(const class easy3d::Mat<3, 3, float> &) --> class easy3d::Mat<3, 3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("rhs"));
        cl.def("__isub__", (class easy3d::Mat<3, 3, float> & (easy3d::Mat<3UL,3UL,float>::*)(const class easy3d::Mat<3, 3, float> &)) &easy3d::Mat<3, 3, float>::operator-=, "C++: easy3d::Mat<3, 3, float>::operator-=(const class easy3d::Mat<3, 3, float> &) --> class easy3d::Mat<3, 3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("rhs"));
        cl.def("__imul__", (class easy3d::Mat<3, 3, float> & (easy3d::Mat<3UL,3UL,float>::*)(float)) &easy3d::Mat<3, 3, float>::operator*=, "C++: easy3d::Mat<3, 3, float>::operator*=(float) --> class easy3d::Mat<3, 3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("rhs"));
        cl.def("__itruediv__", (class easy3d::Mat<3, 3, float> & (easy3d::Mat<3UL,3UL,float>::*)(float)) &easy3d::Mat<3, 3, float>::operator/=, "C++: easy3d::Mat<3, 3, float>::operator/=(float) --> class easy3d::Mat<3, 3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("rhs"));
        cl.def("__iadd__", (class easy3d::Mat<3, 3, float> & (easy3d::Mat<3UL,3UL,float>::*)(float)) &easy3d::Mat<3, 3, float>::operator+=, "C++: easy3d::Mat<3, 3, float>::operator+=(float) --> class easy3d::Mat<3, 3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("rhs"));
        cl.def("__isub__", (class easy3d::Mat<3, 3, float> & (easy3d::Mat<3UL,3UL,float>::*)(float)) &easy3d::Mat<3, 3, float>::operator-=, "C++: easy3d::Mat<3, 3, float>::operator-=(float) --> class easy3d::Mat<3, 3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("rhs"));
    }

}
