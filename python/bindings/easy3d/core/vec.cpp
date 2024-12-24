#include <easy3d/core/vec.h>
#include <easy3d/core/constant.h>

#include <pybind11/pybind11.h>
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
		pybind11::class_<easy3d::Vec<2UL,float>, std::shared_ptr<easy3d::Vec<2UL,float>>> cl(m, "Vec_2UL_float_t", "");
		cl.def( pybind11::init( [](){ return new easy3d::Vec<2UL,float>(); } ) );
		cl.def( pybind11::init<float, float>(), pybind11::arg("x_in"), pybind11::arg("y_in") );

		cl.def( pybind11::init<const class easy3d::Vec<3, float> &>(), pybind11::arg("v") );

		cl.def( pybind11::init<const float &>(), pybind11::arg("s") );

		cl.def( pybind11::init( [](easy3d::Vec<2UL,float> const &o){ return new easy3d::Vec<2UL,float>(o); } ) );
		cl.def("length2", (float (easy3d::Vec<2UL,float>::*)() const) &easy3d::Vec<2, float>::length2, "C++: easy3d::Vec<2, float>::length2() const --> float");
		cl.def("length", (float (easy3d::Vec<2UL,float>::*)() const) &easy3d::Vec<2, float>::length, "C++: easy3d::Vec<2, float>::length() const --> float");
		cl.def("norm", (float (easy3d::Vec<2UL,float>::*)() const) &easy3d::Vec<2, float>::norm, "C++: easy3d::Vec<2, float>::norm() const --> float");
		cl.def("distance2", (float (easy3d::Vec<2UL,float>::*)(const class easy3d::Vec<2, float> &) const) &easy3d::Vec<2, float>::distance2, "C++: easy3d::Vec<2, float>::distance2(const class easy3d::Vec<2, float> &) const --> float", pybind11::arg("rhs"));
		cl.def("normalize", (class easy3d::Vec<2, float> & (easy3d::Vec<2UL,float>::*)()) &easy3d::Vec<2, float>::normalize, "C++: easy3d::Vec<2, float>::normalize() --> class easy3d::Vec<2, float> &", pybind11::return_value_policy::automatic);
		cl.def("__iadd__", (class easy3d::Vec<2, float> & (easy3d::Vec<2UL,float>::*)(const class easy3d::Vec<2, float> &)) &easy3d::Vec<2, float>::operator+=, "C++: easy3d::Vec<2, float>::operator+=(const class easy3d::Vec<2, float> &) --> class easy3d::Vec<2, float> &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
		cl.def("__isub__", (class easy3d::Vec<2, float> & (easy3d::Vec<2UL,float>::*)(const class easy3d::Vec<2, float> &)) &easy3d::Vec<2, float>::operator-=, "C++: easy3d::Vec<2, float>::operator-=(const class easy3d::Vec<2, float> &) --> class easy3d::Vec<2, float> &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
		cl.def("__imul__", (class easy3d::Vec<2, float> & (easy3d::Vec<2UL,float>::*)(const class easy3d::Vec<2, float> &)) &easy3d::Vec<2, float>::operator*=, "C++: easy3d::Vec<2, float>::operator*=(const class easy3d::Vec<2, float> &) --> class easy3d::Vec<2, float> &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
		cl.def("__itruediv__", (class easy3d::Vec<2, float> & (easy3d::Vec<2UL,float>::*)(const class easy3d::Vec<2, float> &)) &easy3d::Vec<2, float>::operator/=, "C++: easy3d::Vec<2, float>::operator/=(const class easy3d::Vec<2, float> &) --> class easy3d::Vec<2, float> &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
		cl.def("__add__", (class easy3d::Vec<2, float> (easy3d::Vec<2UL,float>::*)(const class easy3d::Vec<2, float> &) const) &easy3d::Vec<2, float>::operator+, "C++: easy3d::Vec<2, float>::operator+(const class easy3d::Vec<2, float> &) const --> class easy3d::Vec<2, float>", pybind11::arg("v"));
		cl.def("__sub__", (class easy3d::Vec<2, float> (easy3d::Vec<2UL,float>::*)(const class easy3d::Vec<2, float> &) const) &easy3d::Vec<2, float>::operator-, "C++: easy3d::Vec<2, float>::operator-(const class easy3d::Vec<2, float> &) const --> class easy3d::Vec<2, float>", pybind11::arg("v"));
		cl.def("__neg__", (class easy3d::Vec<2, float> (easy3d::Vec<2UL,float>::*)() const) &easy3d::Vec<2, float>::operator-, "C++: easy3d::Vec<2, float>::operator-() const --> class easy3d::Vec<2, float>");
		cl.def("dimension", (unsigned long (easy3d::Vec<2UL,float>::*)() const) &easy3d::Vec<2, float>::dimension, "C++: easy3d::Vec<2, float>::dimension() const --> unsigned long");
		cl.def("size", (unsigned long (easy3d::Vec<2UL,float>::*)() const) &easy3d::Vec<2, float>::size, "C++: easy3d::Vec<2, float>::size() const --> unsigned long");
		cl.def("data", (float * (easy3d::Vec<2UL,float>::*)()) &easy3d::Vec<2, float>::data, "C++: easy3d::Vec<2, float>::data() --> float *", pybind11::return_value_policy::automatic);

		{ // easy3d::Vec<2, float>::(anonymous union at /Users/lnan/Documents/Projects/Easy3D/easy3d/core/vec.h:394:9) file:easy3d/core/vec.h line:394

			{ // easy3d::Vec<2, float>::(anonymous union)::(anonymous struct at /Users/lnan/Documents/Projects/Easy3D/easy3d/core/vec.h:396:13) file:easy3d/core/vec.h line:396
				cl.def_readwrite("x", &easy3d::Vec<2, float>::x);
				cl.def_readwrite("y", &easy3d::Vec<2, float>::y);
			}

			{ // easy3d::Vec<2, float>::(anonymous union)::(anonymous struct at /Users/lnan/Documents/Projects/Easy3D/easy3d/core/vec.h:397:13) file:easy3d/core/vec.h line:397
				cl.def_readwrite("u", &easy3d::Vec<2, float>::u);
				cl.def_readwrite("v", &easy3d::Vec<2, float>::v);
			}

		}

	}
	{ // easy3d::Vec file:easy3d/core/vec.h line:439
		pybind11::class_<easy3d::Vec<3UL,float>, std::shared_ptr<easy3d::Vec<3UL,float>>> cl(m, "Vec_3UL_float_t", "");
		cl.def( pybind11::init( [](){ return new easy3d::Vec<3UL,float>(); } ) );
		cl.def( pybind11::init( [](const class easy3d::Vec<2, float> & a0){ return new easy3d::Vec<3UL,float>(a0); } ), "doc" , pybind11::arg("v"));
		cl.def( pybind11::init<const class easy3d::Vec<2, float> &, const float &>(), pybind11::arg("v"), pybind11::arg("s") );

		cl.def( pybind11::init<const class easy3d::Vec<4, float> &>(), pybind11::arg("v") );

		cl.def( pybind11::init<float, float, float>(), pybind11::arg("x_in"), pybind11::arg("y_in"), pybind11::arg("z_in") );

		cl.def( pybind11::init<const float &>(), pybind11::arg("s") );

		cl.def( pybind11::init( [](easy3d::Vec<3UL,float> const &o){ return new easy3d::Vec<3UL,float>(o); } ) );
		cl.def("__imul__", (class easy3d::Vec<3, float> & (easy3d::Vec<3UL,float>::*)(float)) &easy3d::Vec<3, float>::operator*=<float>, "C++: easy3d::Vec<3, float>::operator*=(float) --> class easy3d::Vec<3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("s"));
		cl.def("__mul__", (class easy3d::Vec<3, float> (easy3d::Vec<3UL,float>::*)(float) const) &easy3d::Vec<3, float>::operator*<float>, "C++: easy3d::Vec<3, float>::operator*(float) const --> class easy3d::Vec<3, float>", pybind11::arg("s"));
		cl.def("length2", (float (easy3d::Vec<3UL,float>::*)() const) &easy3d::Vec<3, float>::length2, "C++: easy3d::Vec<3, float>::length2() const --> float");
		cl.def("length", (float (easy3d::Vec<3UL,float>::*)() const) &easy3d::Vec<3, float>::length, "C++: easy3d::Vec<3, float>::length() const --> float");
		cl.def("norm", (float (easy3d::Vec<3UL,float>::*)() const) &easy3d::Vec<3, float>::norm, "C++: easy3d::Vec<3, float>::norm() const --> float");
		cl.def("distance2", (float (easy3d::Vec<3UL,float>::*)(const class easy3d::Vec<3, float> &) const) &easy3d::Vec<3, float>::distance2, "C++: easy3d::Vec<3, float>::distance2(const class easy3d::Vec<3, float> &) const --> float", pybind11::arg("rhs"));
		cl.def("normalize", (class easy3d::Vec<3, float> & (easy3d::Vec<3UL,float>::*)()) &easy3d::Vec<3, float>::normalize, "C++: easy3d::Vec<3, float>::normalize() --> class easy3d::Vec<3, float> &", pybind11::return_value_policy::automatic);
		cl.def("__iadd__", (class easy3d::Vec<3, float> & (easy3d::Vec<3UL,float>::*)(const class easy3d::Vec<3, float> &)) &easy3d::Vec<3, float>::operator+=, "C++: easy3d::Vec<3, float>::operator+=(const class easy3d::Vec<3, float> &) --> class easy3d::Vec<3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
		cl.def("__isub__", (class easy3d::Vec<3, float> & (easy3d::Vec<3UL,float>::*)(const class easy3d::Vec<3, float> &)) &easy3d::Vec<3, float>::operator-=, "C++: easy3d::Vec<3, float>::operator-=(const class easy3d::Vec<3, float> &) --> class easy3d::Vec<3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
		cl.def("__imul__", (class easy3d::Vec<3, float> & (easy3d::Vec<3UL,float>::*)(const class easy3d::Vec<3, float> &)) &easy3d::Vec<3, float>::operator*=, "C++: easy3d::Vec<3, float>::operator*=(const class easy3d::Vec<3, float> &) --> class easy3d::Vec<3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
		cl.def("__itruediv__", (class easy3d::Vec<3, float> & (easy3d::Vec<3UL,float>::*)(const class easy3d::Vec<3, float> &)) &easy3d::Vec<3, float>::operator/=, "C++: easy3d::Vec<3, float>::operator/=(const class easy3d::Vec<3, float> &) --> class easy3d::Vec<3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
		cl.def("__add__", (class easy3d::Vec<3, float> (easy3d::Vec<3UL,float>::*)(const class easy3d::Vec<3, float> &) const) &easy3d::Vec<3, float>::operator+, "C++: easy3d::Vec<3, float>::operator+(const class easy3d::Vec<3, float> &) const --> class easy3d::Vec<3, float>", pybind11::arg("v"));
		cl.def("__sub__", (class easy3d::Vec<3, float> (easy3d::Vec<3UL,float>::*)(const class easy3d::Vec<3, float> &) const) &easy3d::Vec<3, float>::operator-, "C++: easy3d::Vec<3, float>::operator-(const class easy3d::Vec<3, float> &) const --> class easy3d::Vec<3, float>", pybind11::arg("v"));
		cl.def("__neg__", (class easy3d::Vec<3, float> (easy3d::Vec<3UL,float>::*)() const) &easy3d::Vec<3, float>::operator-, "C++: easy3d::Vec<3, float>::operator-() const --> class easy3d::Vec<3, float>");
		cl.def("dimension", (unsigned long (easy3d::Vec<3UL,float>::*)() const) &easy3d::Vec<3, float>::dimension, "C++: easy3d::Vec<3, float>::dimension() const --> unsigned long");
		cl.def("size", (unsigned long (easy3d::Vec<3UL,float>::*)() const) &easy3d::Vec<3, float>::size, "C++: easy3d::Vec<3, float>::size() const --> unsigned long");
		cl.def("data", (float * (easy3d::Vec<3UL,float>::*)()) &easy3d::Vec<3, float>::data, "C++: easy3d::Vec<3, float>::data() --> float *", pybind11::return_value_policy::automatic);
		cl.def("xy", (class easy3d::Vec<2, float> (easy3d::Vec<3UL,float>::*)() const) &easy3d::Vec<3, float>::xy, "C++: easy3d::Vec<3, float>::xy() const --> class easy3d::Vec<2, float>");
		cl.def("assign", (class easy3d::Vec<3, float> & (easy3d::Vec<3UL,float>::*)(const class easy3d::Vec<3, float> &)) &easy3d::Vec<3, float>::operator=, "C++: easy3d::Vec<3, float>::operator=(const class easy3d::Vec<3, float> &) --> class easy3d::Vec<3, float> &", pybind11::return_value_policy::automatic, pybind11::arg(""));

		{ // easy3d::Vec<3, float>::(anonymous union at /Users/lnan/Documents/Projects/Easy3D/easy3d/core/vec.h:521:9) file:easy3d/core/vec.h line:521

			{ // easy3d::Vec<3, float>::(anonymous union)::(anonymous struct at /Users/lnan/Documents/Projects/Easy3D/easy3d/core/vec.h:523:13) file:easy3d/core/vec.h line:523
				cl.def_readwrite("x", &easy3d::Vec<3, float>::x);
				cl.def_readwrite("y", &easy3d::Vec<3, float>::y);
				cl.def_readwrite("z", &easy3d::Vec<3, float>::z);
			}

			{ // easy3d::Vec<3, float>::(anonymous union)::(anonymous struct at /Users/lnan/Documents/Projects/Easy3D/easy3d/core/vec.h:524:13) file:easy3d/core/vec.h line:524
				cl.def_readwrite("r", &easy3d::Vec<3, float>::r);
				cl.def_readwrite("g", &easy3d::Vec<3, float>::g);
				cl.def_readwrite("b", &easy3d::Vec<3, float>::b);
			}

		}

	}
	{ // easy3d::Vec file:easy3d/core/vec.h line:584
		pybind11::class_<easy3d::Vec<4UL,float>, std::shared_ptr<easy3d::Vec<4UL,float>>> cl(m, "Vec_4UL_float_t", "");
		cl.def( pybind11::init( [](){ return new easy3d::Vec<4UL,float>(); } ) );
		cl.def( pybind11::init( [](const class easy3d::Vec<3, float> & a0){ return new easy3d::Vec<4UL,float>(a0); } ), "doc" , pybind11::arg("v"));
		cl.def( pybind11::init<const class easy3d::Vec<3, float> &, const float &>(), pybind11::arg("v"), pybind11::arg("s") );

		cl.def( pybind11::init<float, float, float, float>(), pybind11::arg("x_in"), pybind11::arg("y_in"), pybind11::arg("z_in"), pybind11::arg("w_in") );

		cl.def( pybind11::init<const float &>(), pybind11::arg("s") );

		cl.def( pybind11::init( [](easy3d::Vec<4UL,float> const &o){ return new easy3d::Vec<4UL,float>(o); } ) );
		cl.def("length2", (float (easy3d::Vec<4UL,float>::*)() const) &easy3d::Vec<4, float>::length2, "C++: easy3d::Vec<4, float>::length2() const --> float");
		cl.def("length", (float (easy3d::Vec<4UL,float>::*)() const) &easy3d::Vec<4, float>::length, "C++: easy3d::Vec<4, float>::length() const --> float");
		cl.def("norm", (float (easy3d::Vec<4UL,float>::*)() const) &easy3d::Vec<4, float>::norm, "C++: easy3d::Vec<4, float>::norm() const --> float");
		cl.def("distance2", (float (easy3d::Vec<4UL,float>::*)(const class easy3d::Vec<4, float> &) const) &easy3d::Vec<4, float>::distance2, "C++: easy3d::Vec<4, float>::distance2(const class easy3d::Vec<4, float> &) const --> float", pybind11::arg("rhs"));
		cl.def("normalize", (class easy3d::Vec<4, float> & (easy3d::Vec<4UL,float>::*)()) &easy3d::Vec<4, float>::normalize, "C++: easy3d::Vec<4, float>::normalize() --> class easy3d::Vec<4, float> &", pybind11::return_value_policy::automatic);
		cl.def("dimension", (unsigned long (easy3d::Vec<4UL,float>::*)() const) &easy3d::Vec<4, float>::dimension, "C++: easy3d::Vec<4, float>::dimension() const --> unsigned long");
		cl.def("size", (unsigned long (easy3d::Vec<4UL,float>::*)() const) &easy3d::Vec<4, float>::size, "C++: easy3d::Vec<4, float>::size() const --> unsigned long");
		cl.def("__iadd__", (class easy3d::Vec<4, float> & (easy3d::Vec<4UL,float>::*)(const class easy3d::Vec<4, float> &)) &easy3d::Vec<4, float>::operator+=, "C++: easy3d::Vec<4, float>::operator+=(const class easy3d::Vec<4, float> &) --> class easy3d::Vec<4, float> &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
		cl.def("__isub__", (class easy3d::Vec<4, float> & (easy3d::Vec<4UL,float>::*)(const class easy3d::Vec<4, float> &)) &easy3d::Vec<4, float>::operator-=, "C++: easy3d::Vec<4, float>::operator-=(const class easy3d::Vec<4, float> &) --> class easy3d::Vec<4, float> &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
		cl.def("__imul__", (class easy3d::Vec<4, float> & (easy3d::Vec<4UL,float>::*)(const class easy3d::Vec<4, float> &)) &easy3d::Vec<4, float>::operator*=, "C++: easy3d::Vec<4, float>::operator*=(const class easy3d::Vec<4, float> &) --> class easy3d::Vec<4, float> &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
		cl.def("__itruediv__", (class easy3d::Vec<4, float> & (easy3d::Vec<4UL,float>::*)(const class easy3d::Vec<4, float> &)) &easy3d::Vec<4, float>::operator/=, "C++: easy3d::Vec<4, float>::operator/=(const class easy3d::Vec<4, float> &) --> class easy3d::Vec<4, float> &", pybind11::return_value_policy::automatic, pybind11::arg("v"));
		cl.def("__add__", (class easy3d::Vec<4, float> (easy3d::Vec<4UL,float>::*)(const class easy3d::Vec<4, float> &) const) &easy3d::Vec<4, float>::operator+, "C++: easy3d::Vec<4, float>::operator+(const class easy3d::Vec<4, float> &) const --> class easy3d::Vec<4, float>", pybind11::arg("v"));
		cl.def("__sub__", (class easy3d::Vec<4, float> (easy3d::Vec<4UL,float>::*)(const class easy3d::Vec<4, float> &) const) &easy3d::Vec<4, float>::operator-, "C++: easy3d::Vec<4, float>::operator-(const class easy3d::Vec<4, float> &) const --> class easy3d::Vec<4, float>", pybind11::arg("v"));
		cl.def("__neg__", (class easy3d::Vec<4, float> (easy3d::Vec<4UL,float>::*)() const) &easy3d::Vec<4, float>::operator-, "C++: easy3d::Vec<4, float>::operator-() const --> class easy3d::Vec<4, float>");
		cl.def("data", (float * (easy3d::Vec<4UL,float>::*)()) &easy3d::Vec<4, float>::data, "C++: easy3d::Vec<4, float>::data() --> float *", pybind11::return_value_policy::automatic);
		cl.def("xyz", (class easy3d::Vec<3, float> (easy3d::Vec<4UL,float>::*)() const) &easy3d::Vec<4, float>::xyz, "C++: easy3d::Vec<4, float>::xyz() const --> class easy3d::Vec<3, float>");

		{ // easy3d::Vec<4, float>::(anonymous union at /Users/lnan/Documents/Projects/Easy3D/easy3d/core/vec.h:664:9) file:easy3d/core/vec.h line:664

			{ // easy3d::Vec<4, float>::(anonymous union)::(anonymous struct at /Users/lnan/Documents/Projects/Easy3D/easy3d/core/vec.h:666:13) file:easy3d/core/vec.h line:666
				cl.def_readwrite("x", &easy3d::Vec<4, float>::x);
				cl.def_readwrite("y", &easy3d::Vec<4, float>::y);
				cl.def_readwrite("z", &easy3d::Vec<4, float>::z);
				cl.def_readwrite("w", &easy3d::Vec<4, float>::w);
			}

			{ // easy3d::Vec<4, float>::(anonymous union)::(anonymous struct at /Users/lnan/Documents/Projects/Easy3D/easy3d/core/vec.h:667:13) file:easy3d/core/vec.h line:667
				cl.def_readwrite("r", &easy3d::Vec<4, float>::r);
				cl.def_readwrite("g", &easy3d::Vec<4, float>::g);
				cl.def_readwrite("b", &easy3d::Vec<4, float>::b);
				cl.def_readwrite("a", &easy3d::Vec<4, float>::a);
			}

		}

	}

    // easy3d::dot(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &) file:easy3d/core/vec.h line:242
    m.def("dot", (float (*)(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &)) &easy3d::dot<3UL,float>, "C++: easy3d::dot(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &) --> float", pybind11::arg("v1"), pybind11::arg("v2"));

    // easy3d::length(const class easy3d::Vec<3, float> &) file:easy3d/core/vec.h line:289
    m.def("length", (float (*)(const class easy3d::Vec<3, float> &)) &easy3d::length<3UL,float>, "C++: easy3d::length(const class easy3d::Vec<3, float> &) --> float", pybind11::arg("v"));

    // easy3d::norm(const class easy3d::Vec<3, float> &) file:easy3d/core/vec.h line:291
    m.def("norm", (float (*)(const class easy3d::Vec<3, float> &)) &easy3d::norm<3UL,float>, "C++: easy3d::norm(const class easy3d::Vec<3, float> &) --> float", pybind11::arg("v"));

    // easy3d::length2(const class easy3d::Vec<3, float> &) file:easy3d/core/vec.h line:293
    m.def("length2", (float (*)(const class easy3d::Vec<3, float> &)) &easy3d::length2<3UL,float>, "C++: easy3d::length2(const class easy3d::Vec<3, float> &) --> float", pybind11::arg("v"));

    // easy3d::normalize(const class easy3d::Vec<3, float> &) file:easy3d/core/vec.h line:299
    m.def("normalize", (class easy3d::Vec<3, float> (*)(const class easy3d::Vec<3, float> &)) &easy3d::normalize<3UL,float>, "C++: easy3d::normalize(const class easy3d::Vec<3, float> &) --> class easy3d::Vec<3, float>", pybind11::arg("v"));

    // easy3d::det(const class easy3d::Vec<2, float> &, const class easy3d::Vec<2, float> &) file:easy3d/core/vec.h line:405
    m.def("det", (float (*)(const class easy3d::Vec<2, float> &, const class easy3d::Vec<2, float> &)) &easy3d::det<float>, "C++: easy3d::det(const class easy3d::Vec<2, float> &, const class easy3d::Vec<2, float> &) --> float", pybind11::arg("v1"), pybind11::arg("v2"));

    // easy3d::dot(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &) file:easy3d/core/vec.h line:529
    m.def("dot", (float (*)(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &)) &easy3d::dot<float>, "C++: easy3d::dot(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &) --> float", pybind11::arg("v1"), pybind11::arg("v2"));

    // easy3d::cross(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &) file:easy3d/core/vec.h line:534
    m.def("cross", (class easy3d::Vec<3, float> (*)(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &)) &easy3d::cross<float>, "C++: easy3d::cross(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &) --> class easy3d::Vec<3, float>", pybind11::arg("v1"), pybind11::arg("v2"));

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
