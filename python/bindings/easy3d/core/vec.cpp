#include <easy3d/core/types.h>
#include <easy3d/core/constant.h>

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>

#include <string>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_core_vec(pybind11::module_& m)
{
	{ // easy3d::Vec file:easy3d/core/vec.h line:320
		pybind11::class_<easy3d::vec2, std::shared_ptr<easy3d::vec2>> cl(m, "vec2", "");
		cl.def( pybind11::init( [](){ return new easy3d::vec2(); } ) );
		cl.def( pybind11::init<float, float>(), pybind11::arg("x_in"), pybind11::arg("y_in") );

		cl.def( pybind11::init<const easy3d::vec3 &>(), pybind11::arg("v") );

		cl.def( pybind11::init<const float &>(), pybind11::arg("s") );

		cl.def( pybind11::init( [](easy3d::vec2 const &o){ return new easy3d::vec2(o); } ) );

        cl.def("to_numpy", [](const easy3d::vec2 &v) {
            return pybind11::array_t<float>({2}, {sizeof(float)}, &v.x);
        }, "Converts the vector to a NumPy array.");

        cl.def("length2", (float (easy3d::vec2::*)() const) &easy3d::vec2::length2, "C++: easy3d::vec2::length2() const --> float");
		cl.def("length", (float (easy3d::vec2::*)() const) &easy3d::vec2::length, "C++: easy3d::vec2::length() const --> float");
		cl.def("norm", (float (easy3d::vec2::*)() const) &easy3d::vec2::norm, "C++: easy3d::vec2::norm() const --> float");
		cl.def("distance2", (float (easy3d::vec2::*)(const easy3d::vec2 &) const) &easy3d::vec2::distance2, "C++: easy3d::vec2::distance2(const easy3d::vec2 &) const --> float", pybind11::arg("rhs"));
		cl.def("normalize", (easy3d::vec2 & (easy3d::vec2::*)()) &easy3d::vec2::normalize, "C++: easy3d::vec2::normalize() --> easy3d::vec2 &", pybind11::return_value_policy::automatic);
		cl.def("__iadd__", (easy3d::vec2 & (easy3d::vec2::*)(const easy3d::vec2 &)) &easy3d::vec2::operator+=, "C++: easy3d::vec2::operator+=(const easy3d::vec2 &) --> easy3d::vec2 &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
		cl.def("__isub__", (easy3d::vec2 & (easy3d::vec2::*)(const easy3d::vec2 &)) &easy3d::vec2::operator-=, "C++: easy3d::vec2::operator-=(const easy3d::vec2 &) --> easy3d::vec2 &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
		cl.def("__imul__", (easy3d::vec2 & (easy3d::vec2::*)(const easy3d::vec2 &)) &easy3d::vec2::operator*=, "C++: easy3d::vec2::operator*=(const easy3d::vec2 &) --> easy3d::vec2 &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
		cl.def("__itruediv__", (easy3d::vec2 & (easy3d::vec2::*)(const easy3d::vec2 &)) &easy3d::vec2::operator/=, "C++: easy3d::vec2::operator/=(const easy3d::vec2 &) --> easy3d::vec2 &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
		cl.def("__add__", (easy3d::vec2 (easy3d::vec2::*)(const easy3d::vec2 &) const) &easy3d::vec2::operator+, "C++: easy3d::vec2::operator+(const easy3d::vec2 &) const --> easy3d::vec2", pybind11::arg("v"));
		cl.def("__sub__", (easy3d::vec2 (easy3d::vec2::*)(const easy3d::vec2 &) const) &easy3d::vec2::operator-, "C++: easy3d::vec2::operator-(const easy3d::vec2 &) const --> easy3d::vec2", pybind11::arg("v"));
		cl.def("__neg__", (easy3d::vec2 (easy3d::vec2::*)() const) &easy3d::vec2::operator-, "C++: easy3d::vec2::operator-() const --> easy3d::vec2");
		cl.def("dimension", (unsigned long (easy3d::vec2::*)() const) &easy3d::vec2::dimension, "C++: easy3d::vec2::dimension() const --> unsigned long");
		cl.def("size", (unsigned long (easy3d::vec2::*)() const) &easy3d::vec2::size, "C++: easy3d::vec2::size() const --> unsigned long");
		cl.def("data", (float * (easy3d::vec2::*)()) &easy3d::vec2::data, "C++: easy3d::vec2::data() --> float *", pybind11::return_value_policy::automatic);

        cl.def(pybind11::init([](pybind11::array_t<float> arr) {
            if (arr.size() != 2)
                throw std::invalid_argument("Array must have exactly 2 elements.");
            auto buf = arr.unchecked<1>();
            return easy3d::vec2(buf(0), buf(1));
        }));

		{ // easy3d::vec2::(anonymous union at /Users/lnan/Documents/Projects/Easy3D/easy3d/core/vec.h:394:9) file:easy3d/core/vec.h line:394

			{ // easy3d::vec2::(anonymous union)::(anonymous struct at /Users/lnan/Documents/Projects/Easy3D/easy3d/core/vec.h:396:13) file:easy3d/core/vec.h line:396
				cl.def_readwrite("x", &easy3d::vec2::x);
				cl.def_readwrite("y", &easy3d::vec2::y);
			}

			{ // easy3d::vec2::(anonymous union)::(anonymous struct at /Users/lnan/Documents/Projects/Easy3D/easy3d/core/vec.h:397:13) file:easy3d/core/vec.h line:397
				cl.def_readwrite("u", &easy3d::vec2::u);
				cl.def_readwrite("v", &easy3d::vec2::v);
			}

		}

	}
	{ // easy3d::Vec file:easy3d/core/vec.h line:439
		pybind11::class_<easy3d::vec3, std::shared_ptr<easy3d::vec3>> cl(m, "vec3", "");
		cl.def( pybind11::init( [](){ return new easy3d::vec3(); } ) );
		cl.def( pybind11::init( [](const easy3d::vec2 & a0){ return new easy3d::vec3(a0); } ), "doc" , pybind11::arg("v"));
		cl.def( pybind11::init<const easy3d::vec2 &, const float &>(), pybind11::arg("v"), pybind11::arg("s") );

		cl.def( pybind11::init<const easy3d::vec4 &>(), pybind11::arg("v") );

		cl.def( pybind11::init<float, float, float>(), pybind11::arg("x_in"), pybind11::arg("y_in"), pybind11::arg("z_in") );

		cl.def( pybind11::init<const float &>(), pybind11::arg("s") );

		cl.def( pybind11::init( [](easy3d::vec3 const &o){ return new easy3d::vec3(o); } ) );

        cl.def("to_numpy", [](const easy3d::vec3 &v) {
            return pybind11::array_t<float>({3}, {sizeof(float)}, &v.x);
        }, "Converts the vector to a NumPy array.");

		cl.def("__imul__", (easy3d::vec3 & (easy3d::vec3::*)(float)) &easy3d::vec3::operator*=<float>, "C++: easy3d::vec3::operator*=(float) --> easy3d::vec3 &", pybind11::return_value_policy::automatic, pybind11::arg("s"));
		cl.def("__mul__", (easy3d::vec3 (easy3d::vec3::*)(float) const) &easy3d::vec3::operator*<float>, "C++: easy3d::vec3::operator*(float) const --> easy3d::vec3", pybind11::arg("s"));
		cl.def("length2", (float (easy3d::vec3::*)() const) &easy3d::vec3::length2, "C++: easy3d::vec3::length2() const --> float");
		cl.def("length", (float (easy3d::vec3::*)() const) &easy3d::vec3::length, "C++: easy3d::vec3::length() const --> float");
		cl.def("norm", (float (easy3d::vec3::*)() const) &easy3d::vec3::norm, "C++: easy3d::vec3::norm() const --> float");
		cl.def("distance2", (float (easy3d::vec3::*)(const easy3d::vec3 &) const) &easy3d::vec3::distance2, "C++: easy3d::vec3::distance2(const easy3d::vec3 &) const --> float", pybind11::arg("rhs"));
		cl.def("normalize", (easy3d::vec3 & (easy3d::vec3::*)()) &easy3d::vec3::normalize, "C++: easy3d::vec3::normalize() --> easy3d::vec3 &", pybind11::return_value_policy::automatic);
		cl.def("__iadd__", (easy3d::vec3 & (easy3d::vec3::*)(const easy3d::vec3 &)) &easy3d::vec3::operator+=, "C++: easy3d::vec3::operator+=(const easy3d::vec3 &) --> easy3d::vec3 &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
		cl.def("__isub__", (easy3d::vec3 & (easy3d::vec3::*)(const easy3d::vec3 &)) &easy3d::vec3::operator-=, "C++: easy3d::vec3::operator-=(const easy3d::vec3 &) --> easy3d::vec3 &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
		cl.def("__imul__", (easy3d::vec3 & (easy3d::vec3::*)(const easy3d::vec3 &)) &easy3d::vec3::operator*=, "C++: easy3d::vec3::operator*=(const easy3d::vec3 &) --> easy3d::vec3 &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
		cl.def("__itruediv__", (easy3d::vec3 & (easy3d::vec3::*)(const easy3d::vec3 &)) &easy3d::vec3::operator/=, "C++: easy3d::vec3::operator/=(const easy3d::vec3 &) --> easy3d::vec3 &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
		cl.def("__add__", (easy3d::vec3 (easy3d::vec3::*)(const easy3d::vec3 &) const) &easy3d::vec3::operator+, "C++: easy3d::vec3::operator+(const easy3d::vec3 &) const --> easy3d::vec3", pybind11::arg("v"));
		cl.def("__sub__", (easy3d::vec3 (easy3d::vec3::*)(const easy3d::vec3 &) const) &easy3d::vec3::operator-, "C++: easy3d::vec3::operator-(const easy3d::vec3 &) const --> easy3d::vec3", pybind11::arg("v"));
		cl.def("__neg__", (easy3d::vec3 (easy3d::vec3::*)() const) &easy3d::vec3::operator-, "C++: easy3d::vec3::operator-() const --> easy3d::vec3");
		cl.def("dimension", (unsigned long (easy3d::vec3::*)() const) &easy3d::vec3::dimension, "C++: easy3d::vec3::dimension() const --> unsigned long");
		cl.def("size", (unsigned long (easy3d::vec3::*)() const) &easy3d::vec3::size, "C++: easy3d::vec3::size() const --> unsigned long");
		cl.def("data", (float * (easy3d::vec3::*)()) &easy3d::vec3::data, "C++: easy3d::vec3::data() --> float *", pybind11::return_value_policy::automatic);
		cl.def("xy", (easy3d::vec2 (easy3d::vec3::*)() const) &easy3d::vec3::xy, "C++: easy3d::vec3::xy() const --> easy3d::vec2");
		cl.def("assign", (easy3d::vec3 & (easy3d::vec3::*)(const easy3d::vec3 &)) &easy3d::vec3::operator=, "C++: easy3d::vec3::operator=(const easy3d::vec3 &) --> easy3d::vec3 &", pybind11::return_value_policy::automatic, pybind11::arg(""));

        cl.def(pybind11::init([](pybind11::array_t<float> arr) {
            if (arr.size() != 3)
                throw std::invalid_argument("Array must have exactly 3 elements.");
            auto buf = arr.unchecked<1>();
            return easy3d::vec3(buf(0), buf(1), buf(2));
        }));

		{ // easy3d::vec3::(anonymous union at /Users/lnan/Documents/Projects/Easy3D/easy3d/core/vec.h:521:9) file:easy3d/core/vec.h line:521

			{ // easy3d::vec3::(anonymous union)::(anonymous struct at /Users/lnan/Documents/Projects/Easy3D/easy3d/core/vec.h:523:13) file:easy3d/core/vec.h line:523
				cl.def_readwrite("x", &easy3d::vec3::x);
				cl.def_readwrite("y", &easy3d::vec3::y);
				cl.def_readwrite("z", &easy3d::vec3::z);
			}

			{ // easy3d::vec3::(anonymous union)::(anonymous struct at /Users/lnan/Documents/Projects/Easy3D/easy3d/core/vec.h:524:13) file:easy3d/core/vec.h line:524
				cl.def_readwrite("r", &easy3d::vec3::r);
				cl.def_readwrite("g", &easy3d::vec3::g);
				cl.def_readwrite("b", &easy3d::vec3::b);
			}

		}

	}
	{ // easy3d::Vec file:easy3d/core/vec.h line:584
		pybind11::class_<easy3d::vec4, std::shared_ptr<easy3d::vec4>> cl(m, "vec4", "");
		cl.def( pybind11::init( [](){ return new easy3d::vec4(); } ) );
		cl.def( pybind11::init( [](const easy3d::vec3 & a0){ return new easy3d::vec4(a0); } ), "doc" , pybind11::arg("v"));
		cl.def( pybind11::init<const easy3d::vec3 &, const float &>(), pybind11::arg("v"), pybind11::arg("s") );

		cl.def( pybind11::init<float, float, float, float>(), pybind11::arg("x_in"), pybind11::arg("y_in"), pybind11::arg("z_in"), pybind11::arg("w_in") );

		cl.def( pybind11::init<const float &>(), pybind11::arg("s") );

		cl.def( pybind11::init( [](easy3d::vec4 const &o){ return new easy3d::vec4(o); } ) );

        cl.def("to_numpy", [](const easy3d::vec4 &v) {
            return pybind11::array_t<float>({4}, {sizeof(float)}, &v.x);
        }, "Converts the vector to a NumPy array.");

		cl.def("length2", (float (easy3d::vec4::*)() const) &easy3d::vec4::length2, "C++: easy3d::vec4::length2() const --> float");
		cl.def("length", (float (easy3d::vec4::*)() const) &easy3d::vec4::length, "C++: easy3d::vec4::length() const --> float");
		cl.def("norm", (float (easy3d::vec4::*)() const) &easy3d::vec4::norm, "C++: easy3d::vec4::norm() const --> float");
		cl.def("distance2", (float (easy3d::vec4::*)(const easy3d::vec4 &) const) &easy3d::vec4::distance2, "C++: easy3d::vec4::distance2(const easy3d::vec4 &) const --> float", pybind11::arg("rhs"));
		cl.def("normalize", (easy3d::vec4 & (easy3d::vec4::*)()) &easy3d::vec4::normalize, "C++: easy3d::vec4::normalize() --> easy3d::vec4 &", pybind11::return_value_policy::automatic);
		cl.def("dimension", (unsigned long (easy3d::vec4::*)() const) &easy3d::vec4::dimension, "C++: easy3d::vec4::dimension() const --> unsigned long");
		cl.def("size", (unsigned long (easy3d::vec4::*)() const) &easy3d::vec4::size, "C++: easy3d::vec4::size() const --> unsigned long");
		cl.def("__iadd__", (easy3d::vec4 & (easy3d::vec4::*)(const easy3d::vec4 &)) &easy3d::vec4::operator+=, "C++: easy3d::vec4::operator+=(const easy3d::vec4 &) --> easy3d::vec4 &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
		cl.def("__isub__", (easy3d::vec4 & (easy3d::vec4::*)(const easy3d::vec4 &)) &easy3d::vec4::operator-=, "C++: easy3d::vec4::operator-=(const easy3d::vec4 &) --> easy3d::vec4 &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
		cl.def("__imul__", (easy3d::vec4 & (easy3d::vec4::*)(const easy3d::vec4 &)) &easy3d::vec4::operator*=, "C++: easy3d::vec4::operator*=(const easy3d::vec4 &) --> easy3d::vec4 &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
		cl.def("__itruediv__", (easy3d::vec4 & (easy3d::vec4::*)(const easy3d::vec4 &)) &easy3d::vec4::operator/=, "C++: easy3d::vec4::operator/=(const easy3d::vec4 &) --> easy3d::vec4 &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
		cl.def("__add__", (easy3d::vec4 (easy3d::vec4::*)(const easy3d::vec4 &) const) &easy3d::vec4::operator+, "C++: easy3d::vec4::operator+(const easy3d::vec4 &) const --> easy3d::vec4", pybind11::arg("v"));
		cl.def("__sub__", (easy3d::vec4 (easy3d::vec4::*)(const easy3d::vec4 &) const) &easy3d::vec4::operator-, "C++: easy3d::vec4::operator-(const easy3d::vec4 &) const --> easy3d::vec4", pybind11::arg("v"));
		cl.def("__neg__", (easy3d::vec4 (easy3d::vec4::*)() const) &easy3d::vec4::operator-, "C++: easy3d::vec4::operator-() const --> easy3d::vec4");
		cl.def("data", (float * (easy3d::vec4::*)()) &easy3d::vec4::data, "C++: easy3d::vec4::data() --> float *", pybind11::return_value_policy::automatic);
		cl.def("xyz", (easy3d::vec3 (easy3d::vec4::*)() const) &easy3d::vec4::xyz, "C++: easy3d::vec4::xyz() const --> easy3d::vec3");

        cl.def(pybind11::init([](pybind11::array_t<float> arr) {
            if (arr.size() != 4)
                throw std::invalid_argument("Array must have exactly 4 elements.");
            auto buf = arr.unchecked<1>();
            return easy3d::vec4(buf(0), buf(1), buf(2), buf(3));
        }));

		{ // easy3d::vec4::(anonymous union at /Users/lnan/Documents/Projects/Easy3D/easy3d/core/vec.h:664:9) file:easy3d/core/vec.h line:664

			{ // easy3d::vec4::(anonymous union)::(anonymous struct at /Users/lnan/Documents/Projects/Easy3D/easy3d/core/vec.h:666:13) file:easy3d/core/vec.h line:666
				cl.def_readwrite("x", &easy3d::vec4::x);
				cl.def_readwrite("y", &easy3d::vec4::y);
				cl.def_readwrite("z", &easy3d::vec4::z);
				cl.def_readwrite("w", &easy3d::vec4::w);
			}

			{ // easy3d::vec4::(anonymous union)::(anonymous struct at /Users/lnan/Documents/Projects/Easy3D/easy3d/core/vec.h:667:13) file:easy3d/core/vec.h line:667
				cl.def_readwrite("r", &easy3d::vec4::r);
				cl.def_readwrite("g", &easy3d::vec4::g);
				cl.def_readwrite("b", &easy3d::vec4::b);
				cl.def_readwrite("a", &easy3d::vec4::a);
			}

		}

	}

    // easy3d::dot(const easy3d::vec3 &, const easy3d::vec3 &) file:easy3d/core/vec.h line:242
    m.def("dot", (float (*)(const easy3d::vec3 &, const easy3d::vec3 &)) &easy3d::dot<3UL,float>, "C++: easy3d::dot(const easy3d::vec3 &, const easy3d::vec3 &) --> float", pybind11::arg("v1"), pybind11::arg("v2"));

    // easy3d::length(const easy3d::vec3 &) file:easy3d/core/vec.h line:289
    m.def("length", (float (*)(const easy3d::vec3 &)) &easy3d::length<3UL,float>, "C++: easy3d::length(const easy3d::vec3 &) --> float", pybind11::arg("v"));

    // easy3d::norm(const easy3d::vec3 &) file:easy3d/core/vec.h line:291
    m.def("norm", (float (*)(const easy3d::vec3 &)) &easy3d::norm<3UL,float>, "C++: easy3d::norm(const easy3d::vec3 &) --> float", pybind11::arg("v"));

    // easy3d::length2(const easy3d::vec3 &) file:easy3d/core/vec.h line:293
    m.def("length2", (float (*)(const easy3d::vec3 &)) &easy3d::length2<3UL,float>, "C++: easy3d::length2(const easy3d::vec3 &) --> float", pybind11::arg("v"));

    // easy3d::normalize(const easy3d::vec3 &) file:easy3d/core/vec.h line:299
    m.def("normalize", (easy3d::vec3 (*)(const easy3d::vec3 &)) &easy3d::normalize<3UL,float>, "C++: easy3d::normalize(const easy3d::vec3 &) --> easy3d::vec3", pybind11::arg("v"));

    // easy3d::det(const easy3d::vec2 &, const easy3d::vec2 &) file:easy3d/core/vec.h line:405
    m.def("det", (float (*)(const easy3d::vec2 &, const easy3d::vec2 &)) &easy3d::det<float>, "C++: easy3d::det(const easy3d::vec2 &, const easy3d::vec2 &) --> float", pybind11::arg("v1"), pybind11::arg("v2"));

    // easy3d::dot(const easy3d::vec3 &, const easy3d::vec3 &) file:easy3d/core/vec.h line:529
    m.def("dot", (float (*)(const easy3d::vec3 &, const easy3d::vec3 &)) &easy3d::dot<float>, "C++: easy3d::dot(const easy3d::vec3 &, const easy3d::vec3 &) --> float", pybind11::arg("v1"), pybind11::arg("v2"));

    // easy3d::cross(const easy3d::vec3 &, const easy3d::vec3 &) file:easy3d/core/vec.h line:534
    m.def("cross", (easy3d::vec3 (*)(const easy3d::vec3 &, const easy3d::vec3 &)) &easy3d::cross<float>, "C++: easy3d::cross(const easy3d::vec3 &, const easy3d::vec3 &) --> easy3d::vec3", pybind11::arg("v1"), pybind11::arg("v2"));

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
