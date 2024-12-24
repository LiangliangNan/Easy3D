#include <easy3d/core/box.h>
#include <easy3d/core/vec.h>

#include <string>

#include <pybind11/pybind11.h>


#ifndef BINDER_PYBIND11_TYPE_CASTER
#define BINDER_PYBIND11_TYPE_CASTER
PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_core_box(pybind11::module_& m)
{
    { // easy3d::GenericBox file:easy3d/core/box.h line:47
        pybind11::class_<easy3d::GenericBox<3,float>, std::shared_ptr<easy3d::GenericBox<3,float>>> cl(m, "GenericBox_3_float_t", "");
        cl.def( pybind11::init( [](){ return new easy3d::GenericBox<3,float>(); } ) );
        cl.def( pybind11::init<const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &>(), pybind11::arg("pmin"), pybind11::arg("pmax") );

        cl.def( pybind11::init<const class easy3d::Vec<3, float> &, float>(), pybind11::arg("c"), pybind11::arg("r") );

        cl.def( pybind11::init( [](easy3d::GenericBox<3,float> const &o){ return new easy3d::GenericBox<3,float>(o); } ) );
        cl.def("is_valid", (bool (easy3d::GenericBox<3,float>::*)() const) &easy3d::GenericBox<3, float>::is_valid, "C++: easy3d::GenericBox<3, float>::is_valid() const --> bool");
        cl.def("clear", (void (easy3d::GenericBox<3,float>::*)()) &easy3d::GenericBox<3, float>::clear, "C++: easy3d::GenericBox<3, float>::clear() --> void");
        cl.def("min_point", (class easy3d::Vec<3, float> & (easy3d::GenericBox<3,float>::*)()) &easy3d::GenericBox<3, float>::min_point, "C++: easy3d::GenericBox<3, float>::min_point() --> class easy3d::Vec<3, float> &", pybind11::return_value_policy::automatic);
        cl.def("max_point", (class easy3d::Vec<3, float> & (easy3d::GenericBox<3,float>::*)()) &easy3d::GenericBox<3, float>::max_point, "C++: easy3d::GenericBox<3, float>::max_point() --> class easy3d::Vec<3, float> &", pybind11::return_value_policy::automatic);
        cl.def("min_coord", (float (easy3d::GenericBox<3,float>::*)(unsigned int) const) &easy3d::GenericBox<3, float>::min_coord, "C++: easy3d::GenericBox<3, float>::min_coord(unsigned int) const --> float", pybind11::arg("axis"));
        cl.def("max_coord", (float (easy3d::GenericBox<3,float>::*)(unsigned int) const) &easy3d::GenericBox<3, float>::max_coord, "C++: easy3d::GenericBox<3, float>::max_coord(unsigned int) const --> float", pybind11::arg("axis"));
        cl.def("range", (float (easy3d::GenericBox<3,float>::*)(unsigned int) const) &easy3d::GenericBox<3, float>::range, "C++: easy3d::GenericBox<3, float>::range(unsigned int) const --> float", pybind11::arg("axis"));
        cl.def("max_range", (float (easy3d::GenericBox<3,float>::*)() const) &easy3d::GenericBox<3, float>::max_range, "C++: easy3d::GenericBox<3, float>::max_range() const --> float");
        cl.def("min_range", (float (easy3d::GenericBox<3,float>::*)() const) &easy3d::GenericBox<3, float>::min_range, "C++: easy3d::GenericBox<3, float>::min_range() const --> float");
        cl.def("max_range_axis", (unsigned int (easy3d::GenericBox<3,float>::*)() const) &easy3d::GenericBox<3, float>::max_range_axis, "C++: easy3d::GenericBox<3, float>::max_range_axis() const --> unsigned int");
        cl.def("min_range_axis", (unsigned int (easy3d::GenericBox<3,float>::*)() const) &easy3d::GenericBox<3, float>::min_range_axis, "C++: easy3d::GenericBox<3, float>::min_range_axis() const --> unsigned int");
        cl.def("center", (class easy3d::Vec<3, float> (easy3d::GenericBox<3,float>::*)() const) &easy3d::GenericBox<3, float>::center, "C++: easy3d::GenericBox<3, float>::center() const --> class easy3d::Vec<3, float>");
        cl.def("diagonal_vector", (class easy3d::Vec<3, float> (easy3d::GenericBox<3,float>::*)() const) &easy3d::GenericBox<3, float>::diagonal_vector, "C++: easy3d::GenericBox<3, float>::diagonal_vector() const --> class easy3d::Vec<3, float>");
        cl.def("diagonal_length", (float (easy3d::GenericBox<3,float>::*)() const) &easy3d::GenericBox<3, float>::diagonal_length, "C++: easy3d::GenericBox<3, float>::diagonal_length() const --> float");
        cl.def("radius", (float (easy3d::GenericBox<3,float>::*)() const) &easy3d::GenericBox<3, float>::radius, "C++: easy3d::GenericBox<3, float>::radius() const --> float");
        cl.def("surface_area", (float (easy3d::GenericBox<3,float>::*)() const) &easy3d::GenericBox<3, float>::surface_area, "C++: easy3d::GenericBox<3, float>::surface_area() const --> float");
        cl.def("grow", (void (easy3d::GenericBox<3,float>::*)(const class easy3d::Vec<3, float> &)) &easy3d::GenericBox<3, float>::grow, "C++: easy3d::GenericBox<3, float>::grow(const class easy3d::Vec<3, float> &) --> void", pybind11::arg("p"));
        cl.def("grow", (void (easy3d::GenericBox<3,float>::*)(const class easy3d::GenericBox<3, float> &)) &easy3d::GenericBox<3, float>::grow, "C++: easy3d::GenericBox<3, float>::grow(const class easy3d::GenericBox<3, float> &) --> void", pybind11::arg("b"));
        cl.def("__add__", (class easy3d::GenericBox<3, float> (easy3d::GenericBox<3,float>::*)(const class easy3d::GenericBox<3, float> &) const) &easy3d::GenericBox<3, float>::operator+, "C++: easy3d::GenericBox<3, float>::operator+(const class easy3d::GenericBox<3, float> &) const --> class easy3d::GenericBox<3, float>", pybind11::arg("b"));
        cl.def("__iadd__", (class easy3d::GenericBox<3, float> & (easy3d::GenericBox<3,float>::*)(const class easy3d::GenericBox<3, float> &)) &easy3d::GenericBox<3, float>::operator+=, "C++: easy3d::GenericBox<3, float>::operator+=(const class easy3d::GenericBox<3, float> &) --> class easy3d::GenericBox<3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("b"));
        cl.def("contains", (bool (easy3d::GenericBox<3,float>::*)(const class easy3d::Vec<3, float> &) const) &easy3d::GenericBox<3, float>::contains, "C++: easy3d::GenericBox<3, float>::contains(const class easy3d::Vec<3, float> &) const --> bool", pybind11::arg("p"));
        cl.def("contains", (bool (easy3d::GenericBox<3,float>::*)(const class easy3d::GenericBox<3, float> &) const) &easy3d::GenericBox<3, float>::contains, "C++: easy3d::GenericBox<3, float>::contains(const class easy3d::GenericBox<3, float> &) const --> bool", pybind11::arg("b"));
        cl.def("intersects", (bool (easy3d::GenericBox<3,float>::*)(const class easy3d::GenericBox<3, float> &) const) &easy3d::GenericBox<3, float>::intersects, "C++: easy3d::GenericBox<3, float>::intersects(const class easy3d::GenericBox<3, float> &) const --> bool", pybind11::arg("b"));
        cl.def("assign", (class easy3d::GenericBox<3, float> & (easy3d::GenericBox<3,float>::*)(const class easy3d::GenericBox<3, float> &)) &easy3d::GenericBox<3, float>::operator=, "C++: easy3d::GenericBox<3, float>::operator=(const class easy3d::GenericBox<3, float> &) --> class easy3d::GenericBox<3, float> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
    }
}
