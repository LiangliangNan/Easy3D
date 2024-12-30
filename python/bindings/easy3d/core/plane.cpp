#include <easy3d/core/line.h>
#include <easy3d/core/plane.h>
#include <easy3d/core/vec.h>

#include <string>

#include <pybind11/pybind11.h>


#ifndef BINDER_PYBIND11_TYPE_CASTER
#define BINDER_PYBIND11_TYPE_CASTER
PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_core_plane(pybind11::module_& m)
{
    { // easy3d::GenericPlane file:easy3d/core/plane.h line:41
        pybind11::class_<easy3d::GenericPlane<float>, std::shared_ptr<easy3d::GenericPlane<float>>> cl(m, "GenericPlane_float_t", "");
        cl.def( pybind11::init<const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &>(), pybind11::arg("p1"), pybind11::arg("p2"), pybind11::arg("p3") );

        cl.def( pybind11::init<const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &>(), pybind11::arg("p"), pybind11::arg("n") );

        cl.def( pybind11::init<float, float, float, float>(), pybind11::arg("a"), pybind11::arg("b"), pybind11::arg("c"), pybind11::arg("d") );

        cl.def( pybind11::init( [](){ return new easy3d::GenericPlane<float>(); } ) );
        cl.def( pybind11::init( [](easy3d::GenericPlane<float> const &o){ return new easy3d::GenericPlane<float>(o); } ) );
        cl.def("a", (float (easy3d::GenericPlane<float>::*)() const) &easy3d::GenericPlane<float>::a, "C++: easy3d::GenericPlane<float>::a() const --> float");
        cl.def("b", (float (easy3d::GenericPlane<float>::*)() const) &easy3d::GenericPlane<float>::b, "C++: easy3d::GenericPlane<float>::b() const --> float");
        cl.def("c", (float (easy3d::GenericPlane<float>::*)() const) &easy3d::GenericPlane<float>::c, "C++: easy3d::GenericPlane<float>::c() const --> float");
        cl.def("d", (float (easy3d::GenericPlane<float>::*)() const) &easy3d::GenericPlane<float>::d, "C++: easy3d::GenericPlane<float>::d() const --> float");
        cl.def("__getitem__", (float & (easy3d::GenericPlane<float>::*)(size_t)) &easy3d::GenericPlane<float>::operator[], "C++: easy3d::GenericPlane<float>::operator[](size_t) --> float &", pybind11::return_value_policy::automatic, pybind11::arg("idx"));
        cl.def("normal", (class easy3d::Vec<3, float> (easy3d::GenericPlane<float>::*)() const) &easy3d::GenericPlane<float>::normal, "C++: easy3d::GenericPlane<float>::normal() const --> class easy3d::Vec<3, float>");
        cl.def("point", (class easy3d::Vec<3, float> (easy3d::GenericPlane<float>::*)() const) &easy3d::GenericPlane<float>::point, "C++: easy3d::GenericPlane<float>::point() const --> class easy3d::Vec<3, float>");
        cl.def("base1", (class easy3d::Vec<3, float> (easy3d::GenericPlane<float>::*)() const) &easy3d::GenericPlane<float>::base1, "C++: easy3d::GenericPlane<float>::base1() const --> class easy3d::Vec<3, float>");
        cl.def("base2", (class easy3d::Vec<3, float> (easy3d::GenericPlane<float>::*)() const) &easy3d::GenericPlane<float>::base2, "C++: easy3d::GenericPlane<float>::base2() const --> class easy3d::Vec<3, float>");
        cl.def("to_2d", (class easy3d::Vec<2, float> (easy3d::GenericPlane<float>::*)(const class easy3d::Vec<3, float> &) const) &easy3d::GenericPlane<float>::to_2d, "C++: easy3d::GenericPlane<float>::to_2d(const class easy3d::Vec<3, float> &) const --> class easy3d::Vec<2, float>", pybind11::arg("p"));
        cl.def("to_3d", (class easy3d::Vec<3, float> (easy3d::GenericPlane<float>::*)(const class easy3d::Vec<2, float> &) const) &easy3d::GenericPlane<float>::to_3d, "C++: easy3d::GenericPlane<float>::to_3d(const class easy3d::Vec<2, float> &) const --> class easy3d::Vec<3, float>", pybind11::arg("p"));
        cl.def("projection", (class easy3d::Vec<3, float> (easy3d::GenericPlane<float>::*)(const class easy3d::Vec<3, float> &) const) &easy3d::GenericPlane<float>::projection, "C++: easy3d::GenericPlane<float>::projection(const class easy3d::Vec<3, float> &) const --> class easy3d::Vec<3, float>", pybind11::arg("p"));
        cl.def("value", (float (easy3d::GenericPlane<float>::*)(const class easy3d::Vec<3, float> &) const) &easy3d::GenericPlane<float>::value, "C++: easy3d::GenericPlane<float>::value(const class easy3d::Vec<3, float> &) const --> float", pybind11::arg("p"));
        cl.def("squared_distance", (float (easy3d::GenericPlane<float>::*)(const class easy3d::Vec<3, float> &) const) &easy3d::GenericPlane<float>::squared_distance, "C++: easy3d::GenericPlane<float>::squared_distance(const class easy3d::Vec<3, float> &) const --> float", pybind11::arg("p"));
        cl.def("intersect", (bool (easy3d::GenericPlane<float>::*)(const class easy3d::GenericLine<3, float> &, class easy3d::Vec<3, float> &) const) &easy3d::GenericPlane<float>::intersect, "C++: easy3d::GenericPlane<float>::intersect(const class easy3d::GenericLine<3, float> &, class easy3d::Vec<3, float> &) const --> bool", pybind11::arg("line"), pybind11::arg("p"));
        cl.def("intersect", (bool (easy3d::GenericPlane<float>::*)(const class easy3d::GenericLine<3, float> &) const) &easy3d::GenericPlane<float>::intersect, "C++: easy3d::GenericPlane<float>::intersect(const class easy3d::GenericLine<3, float> &) const --> bool", pybind11::arg("line"));
        cl.def("intersect", (bool (easy3d::GenericPlane<float>::*)(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, class easy3d::Vec<3, float> &) const) &easy3d::GenericPlane<float>::intersect, "C++: easy3d::GenericPlane<float>::intersect(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, class easy3d::Vec<3, float> &) const --> bool", pybind11::arg("s"), pybind11::arg("t"), pybind11::arg("p"));
        cl.def("intersect", (bool (easy3d::GenericPlane<float>::*)(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &) const) &easy3d::GenericPlane<float>::intersect, "C++: easy3d::GenericPlane<float>::intersect(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &) const --> bool", pybind11::arg("s"), pybind11::arg("t"));
        cl.def("intersect", (bool (easy3d::GenericPlane<float>::*)(const class easy3d::GenericPlane<float> &, class easy3d::GenericLine<3, float> &) const) &easy3d::GenericPlane<float>::intersect, "C++: easy3d::GenericPlane<float>::intersect(const class easy3d::GenericPlane<float> &, class easy3d::GenericLine<3, float> &) const --> bool", pybind11::arg("another"), pybind11::arg("line"));
        cl.def("intersect", (bool (easy3d::GenericPlane<float>::*)(const class easy3d::GenericPlane<float> &) const) &easy3d::GenericPlane<float>::intersect, "C++: easy3d::GenericPlane<float>::intersect(const class easy3d::GenericPlane<float> &) const --> bool", pybind11::arg("another"));
        cl.def("orient", (int (easy3d::GenericPlane<float>::*)(const class easy3d::Vec<3, float> &) const) &easy3d::GenericPlane<float>::orient, "C++: easy3d::GenericPlane<float>::orient(const class easy3d::Vec<3, float> &) const --> int", pybind11::arg("p"));
        cl.def("data", (float * (easy3d::GenericPlane<float>::*)()) &easy3d::GenericPlane<float>::data, "C++: easy3d::GenericPlane<float>::data() --> float *", pybind11::return_value_policy::automatic);
        cl.def("assign", (class easy3d::GenericPlane<float> & (easy3d::GenericPlane<float>::*)(const class easy3d::GenericPlane<float> &)) &easy3d::GenericPlane<float>::operator=, "C++: easy3d::GenericPlane<float>::operator=(const class easy3d::GenericPlane<float> &) --> class easy3d::GenericPlane<float> &", pybind11::return_value_policy::automatic, pybind11::arg(""));

        cl.def("__str__", [](easy3d::GenericPlane<float> const &o) -> std::string { std::ostringstream s; using namespace easy3d; s << o; return s.str(); } );
    }
}
