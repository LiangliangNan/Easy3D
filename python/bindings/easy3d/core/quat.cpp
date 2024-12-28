#include <easy3d/core/mat.h>
#include <easy3d/core/quat.h>
#include <easy3d/core/vec.h>
#include <string>

#include <pybind11/pybind11.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_core_quat(pybind11::module_ &m)
{
	{ // easy3d::Quat file:easy3d/core/quat.h line:106
		pybind11::class_<easy3d::Quat<float>, std::shared_ptr<easy3d::Quat<float>>> cl(m, "quat", "");
		cl.def( pybind11::init( [](){ return new easy3d::Quat<float>(); } ) );
		cl.def( pybind11::init<const class easy3d::Mat3<float> &>(), pybind11::arg("m") );

		cl.def( pybind11::init<const class easy3d::Vec<3, float> &, float>(), pybind11::arg("axis"), pybind11::arg("angle") );

		cl.def( pybind11::init<const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &>(), pybind11::arg("from"), pybind11::arg("to") );

		cl.def( pybind11::init<float, float, float, float>(), pybind11::arg("q0"), pybind11::arg("q1"), pybind11::arg("q2"), pybind11::arg("q3") );

		cl.def( pybind11::init( [](easy3d::Quat<float> const &o){ return new easy3d::Quat<float>(o); } ) );
		cl.def("assign", (class easy3d::Quat<float> & (easy3d::Quat<float>::*)(const class easy3d::Quat<float> &)) &easy3d::Quat<float>::operator=, "C++: easy3d::Quat<float>::operator=(const class easy3d::Quat<float> &) --> class easy3d::Quat<float> &", pybind11::return_value_policy::automatic, pybind11::arg("Q"));
		cl.def("set_axis_angle", (void (easy3d::Quat<float>::*)(const class easy3d::Vec<3, float> &, float)) &easy3d::Quat<float>::set_axis_angle, "C++: easy3d::Quat<float>::set_axis_angle(const class easy3d::Vec<3, float> &, float) --> void", pybind11::arg("axis"), pybind11::arg("angle"));
		cl.def("set_value", (void (easy3d::Quat<float>::*)(float, float, float, float)) &easy3d::Quat<float>::set_value, "C++: easy3d::Quat<float>::set_value(float, float, float, float) --> void", pybind11::arg("q0"), pybind11::arg("q1"), pybind11::arg("q2"), pybind11::arg("q3"));
		cl.def("set_from_rotation_matrix", (void (easy3d::Quat<float>::*)(const class easy3d::Mat3<float> &)) &easy3d::Quat<float>::set_from_rotation_matrix, "C++: easy3d::Quat<float>::set_from_rotation_matrix(const class easy3d::Mat3<float> &) --> void", pybind11::arg("m"));
		cl.def("set_from_rotated_basis", (void (easy3d::Quat<float>::*)(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &)) &easy3d::Quat<float>::set_from_rotated_basis, "C++: easy3d::Quat<float>::set_from_rotated_basis(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &) --> void", pybind11::arg("X"), pybind11::arg("Y"), pybind11::arg("Z"));
		cl.def("axis", (class easy3d::Vec<3, float> (easy3d::Quat<float>::*)() const) &easy3d::Quat<float>::axis, "C++: easy3d::Quat<float>::axis() const --> class easy3d::Vec<3, float>");
		cl.def("angle", (float (easy3d::Quat<float>::*)() const) &easy3d::Quat<float>::angle, "C++: easy3d::Quat<float>::angle() const --> float");
		cl.def("get_axis_angle", (void (easy3d::Quat<float>::*)(class easy3d::Vec<3, float> &, float &) const) &easy3d::Quat<float>::get_axis_angle, "C++: easy3d::Quat<float>::get_axis_angle(class easy3d::Vec<3, float> &, float &) const --> void", pybind11::arg("axis"), pybind11::arg("angle"));
		cl.def("__getitem__", (float & (easy3d::Quat<float>::*)(int)) &easy3d::Quat<float>::operator[], "C++: easy3d::Quat<float>::operator[](int) --> float &", pybind11::return_value_policy::automatic, pybind11::arg("i"));
		cl.def("__imul__", (class easy3d::Quat<float> & (easy3d::Quat<float>::*)(const class easy3d::Quat<float> &)) &easy3d::Quat<float>::operator*=, "C++: easy3d::Quat<float>::operator*=(const class easy3d::Quat<float> &) --> class easy3d::Quat<float> &", pybind11::return_value_policy::automatic, pybind11::arg("q"));
		cl.def("rotate", (class easy3d::Vec<3, float> (easy3d::Quat<float>::*)(const class easy3d::Vec<3, float> &) const) &easy3d::Quat<float>::rotate, "C++: easy3d::Quat<float>::rotate(const class easy3d::Vec<3, float> &) const --> class easy3d::Vec<3, float>", pybind11::arg("v"));
		cl.def("inverse_rotate", (class easy3d::Vec<3, float> (easy3d::Quat<float>::*)(const class easy3d::Vec<3, float> &) const) &easy3d::Quat<float>::inverse_rotate, "C++: easy3d::Quat<float>::inverse_rotate(const class easy3d::Vec<3, float> &) const --> class easy3d::Vec<3, float>", pybind11::arg("v"));
		cl.def("inverse", (class easy3d::Quat<float> (easy3d::Quat<float>::*)() const) &easy3d::Quat<float>::inverse, "C++: easy3d::Quat<float>::inverse() const --> class easy3d::Quat<float>");
		cl.def("invert", (void (easy3d::Quat<float>::*)()) &easy3d::Quat<float>::invert, "C++: easy3d::Quat<float>::invert() --> void");
		cl.def("negate", (void (easy3d::Quat<float>::*)()) &easy3d::Quat<float>::negate, "C++: easy3d::Quat<float>::negate() --> void");
		cl.def("length", (float (easy3d::Quat<float>::*)() const) &easy3d::Quat<float>::length, "C++: easy3d::Quat<float>::length() const --> float");
		cl.def("normalize", (float (easy3d::Quat<float>::*)()) &easy3d::Quat<float>::normalize, "C++: easy3d::Quat<float>::normalize() --> float");
		cl.def("normalized", (class easy3d::Quat<float> (easy3d::Quat<float>::*)() const) &easy3d::Quat<float>::normalized, "C++: easy3d::Quat<float>::normalized() const --> class easy3d::Quat<float>");
		cl.def("matrix", (class easy3d::Mat4<float> (easy3d::Quat<float>::*)() const) &easy3d::Quat<float>::matrix, "C++: easy3d::Quat<float>::matrix() const --> class easy3d::Mat4<float>");
		cl.def("inverse_matrix", (class easy3d::Mat4<float> (easy3d::Quat<float>::*)() const) &easy3d::Quat<float>::inverse_matrix, "C++: easy3d::Quat<float>::inverse_matrix() const --> class easy3d::Mat4<float>");
		cl.def_static("slerp", [](const class easy3d::Quat<float> & a0, const class easy3d::Quat<float> & a1, float const & a2) -> easy3d::Quat<float> { return easy3d::Quat<float>::slerp(a0, a1, a2); }, "", pybind11::arg("a"), pybind11::arg("b"), pybind11::arg("t"));
		cl.def_static("slerp", (class easy3d::Quat<float> (*)(const class easy3d::Quat<float> &, const class easy3d::Quat<float> &, float, bool)) &easy3d::Quat<float>::slerp, "C++: easy3d::Quat<float>::slerp(const class easy3d::Quat<float> &, const class easy3d::Quat<float> &, float, bool) --> class easy3d::Quat<float>", pybind11::arg("a"), pybind11::arg("b"), pybind11::arg("t"), pybind11::arg("allowFlip"));
		cl.def_static("squad", (class easy3d::Quat<float> (*)(const class easy3d::Quat<float> &, const class easy3d::Quat<float> &, const class easy3d::Quat<float> &, const class easy3d::Quat<float> &, float)) &easy3d::Quat<float>::squad, "C++: easy3d::Quat<float>::squad(const class easy3d::Quat<float> &, const class easy3d::Quat<float> &, const class easy3d::Quat<float> &, const class easy3d::Quat<float> &, float) --> class easy3d::Quat<float>", pybind11::arg("a"), pybind11::arg("tgA"), pybind11::arg("tgB"), pybind11::arg("b"), pybind11::arg("t"));
		cl.def_static("dot", (float (*)(const class easy3d::Quat<float> &, const class easy3d::Quat<float> &)) &easy3d::Quat<float>::dot, "C++: easy3d::Quat<float>::dot(const class easy3d::Quat<float> &, const class easy3d::Quat<float> &) --> float", pybind11::arg("a"), pybind11::arg("b"));
		cl.def("log", (class easy3d::Quat<float> (easy3d::Quat<float>::*)()) &easy3d::Quat<float>::log, "C++: easy3d::Quat<float>::log() --> class easy3d::Quat<float>");
		cl.def("exp", (class easy3d::Quat<float> (easy3d::Quat<float>::*)()) &easy3d::Quat<float>::exp, "C++: easy3d::Quat<float>::exp() --> class easy3d::Quat<float>");
		cl.def_static("ln_dif", (class easy3d::Quat<float> (*)(const class easy3d::Quat<float> &, const class easy3d::Quat<float> &)) &easy3d::Quat<float>::ln_dif, "C++: easy3d::Quat<float>::ln_dif(const class easy3d::Quat<float> &, const class easy3d::Quat<float> &) --> class easy3d::Quat<float>", pybind11::arg("a"), pybind11::arg("b"));
		cl.def_static("squad_tangent", (class easy3d::Quat<float> (*)(const class easy3d::Quat<float> &, const class easy3d::Quat<float> &, const class easy3d::Quat<float> &)) &easy3d::Quat<float>::squad_tangent, "C++: easy3d::Quat<float>::squad_tangent(const class easy3d::Quat<float> &, const class easy3d::Quat<float> &, const class easy3d::Quat<float> &) --> class easy3d::Quat<float>", pybind11::arg("before"), pybind11::arg("center"), pybind11::arg("after"));
		cl.def_static("random_quat", (class easy3d::Quat<float> (*)()) &easy3d::Quat<float>::random_quat, "C++: easy3d::Quat<float>::random_quat() --> class easy3d::Quat<float>");

		cl.def("__str__", [](easy3d::Quat<float> const &o) -> std::string { std::ostringstream s; using namespace easy3d; s << o; return s.str(); } );

		{ // easy3d::Quat<float>::(anonymous union at /Users/lnan/Documents/Projects/Easy3D/easy3d/core/quat.h:371:9) file:easy3d/core/quat.h line:371

			{ // easy3d::Quat<float>::(anonymous union)::(anonymous struct at /Users/lnan/Documents/Projects/Easy3D/easy3d/core/quat.h:372:13) file:easy3d/core/quat.h line:372
                cl.def_property("x",
                        [](const easy3d::Quat<float>& q) { return q._q[0]; },  // Getter
                        [](easy3d::Quat<float>& q, float value) { q._q[0] = value; }  // Setter
                );
                cl.def_property("y",
                        [](const easy3d::Quat<float>& q) { return q._q[1]; },  // Getter
                        [](easy3d::Quat<float>& q, float value) { q._q[1] = value; }  // Setter
                );
                cl.def_property("z",
                        [](const easy3d::Quat<float>& q) { return q._q[2]; },  // Getter
                        [](easy3d::Quat<float>& q, float value) { q._q[2] = value; }  // Setter
                );
                cl.def_property("w",
                        [](const easy3d::Quat<float>& q) { return q._q[3]; },  // Getter
                        [](easy3d::Quat<float>& q, float value) { q._q[3] = value; }  // Setter
                );
            }

		}

	}
}
