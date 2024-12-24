#include <easy3d/core/line.h>
#include <easy3d/core/oriented_line.h>
#include <easy3d/core/vec.h>

#include <string>

#include <pybind11/pybind11.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
#define BINDER_PYBIND11_TYPE_CASTER
PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_core_line(pybind11::module_& m)
{
    { // easy3d::GenericLine file:easy3d/core/line.h line:40
        pybind11::class_<easy3d::GenericLine<3,float>, std::shared_ptr<easy3d::GenericLine<3,float>>> cl(m, "GenericLine_3_float_t", "");
        cl.def( pybind11::init( [](){ return new easy3d::GenericLine<3,float>(); } ) );
        cl.def( pybind11::init( [](easy3d::GenericLine<3,float> const &o){ return new easy3d::GenericLine<3,float>(o); } ) );
        cl.def_static("from_point_and_direction", (class easy3d::GenericLine<3, float> (*)(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &)) &easy3d::GenericLine<3, float>::from_point_and_direction, "C++: easy3d::GenericLine<3, float>::from_point_and_direction(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &) --> class easy3d::GenericLine<3, float>", pybind11::arg("p"), pybind11::arg("dir"));
        cl.def_static("from_two_points", (class easy3d::GenericLine<3, float> (*)(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &)) &easy3d::GenericLine<3, float>::from_two_points, "C++: easy3d::GenericLine<3, float>::from_two_points(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &) --> class easy3d::GenericLine<3, float>", pybind11::arg("p"), pybind11::arg("q"));
        cl.def("set", (void (easy3d::GenericLine<3,float>::*)(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &)) &easy3d::GenericLine<3, float>::set, "C++: easy3d::GenericLine<3, float>::set(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &) --> void", pybind11::arg("p"), pybind11::arg("dir"));
        cl.def("direction", (const class easy3d::Vec<3, float> & (easy3d::GenericLine<3,float>::*)() const) &easy3d::GenericLine<3, float>::direction, "C++: easy3d::GenericLine<3, float>::direction() const --> const class easy3d::Vec<3, float> &", pybind11::return_value_policy::automatic);
        cl.def("point", (const class easy3d::Vec<3, float> & (easy3d::GenericLine<3,float>::*)() const) &easy3d::GenericLine<3, float>::point, "C++: easy3d::GenericLine<3, float>::point() const --> const class easy3d::Vec<3, float> &", pybind11::return_value_policy::automatic);
        cl.def("projection", (class easy3d::Vec<3, float> (easy3d::GenericLine<3,float>::*)(const class easy3d::Vec<3, float> &) const) &easy3d::GenericLine<3, float>::projection, "C++: easy3d::GenericLine<3, float>::projection(const class easy3d::Vec<3, float> &) const --> class easy3d::Vec<3, float>", pybind11::arg("p"));
        cl.def("squared_distance", (float (easy3d::GenericLine<3,float>::*)(const class easy3d::Vec<3, float> &) const) &easy3d::GenericLine<3, float>::squared_distance, "C++: easy3d::GenericLine<3, float>::squared_distance(const class easy3d::Vec<3, float> &) const --> float", pybind11::arg("p"));
        cl.def("feet", (bool (easy3d::GenericLine<3,float>::*)(const class easy3d::GenericLine<3, float> &, class easy3d::Vec<3, float> &, class easy3d::Vec<3, float> &) const) &easy3d::GenericLine<3, float>::feet, "C++: easy3d::GenericLine<3, float>::feet(const class easy3d::GenericLine<3, float> &, class easy3d::Vec<3, float> &, class easy3d::Vec<3, float> &) const --> bool", pybind11::arg("other"), pybind11::arg("p1"), pybind11::arg("p2"));
    }
    // easy3d::Sign file:easy3d/core/oriented_line.h line:20
    pybind11::enum_<easy3d::Sign>(m, "Sign", pybind11::arithmetic(), "The sign.")
            .value("NEGATIVE", easy3d::NEGATIVE)
            .value("ZERO", easy3d::ZERO)
            .value("POSITIVE", easy3d::POSITIVE)
            .export_values();

    // easy3d::sign(float) file:easy3d/core/oriented_line.h line:28
    m.def("sign", (enum easy3d::Sign (*)(float)) &easy3d::sign<float>, "C++: easy3d::sign(float) --> enum easy3d::Sign", pybind11::arg("x"));

    { // easy3d::GenericOrientedLine file:easy3d/core/oriented_line.h line:43
        pybind11::class_<easy3d::GenericOrientedLine<float>, std::shared_ptr<easy3d::GenericOrientedLine<float>>> cl(m, "GenericOrientedLine_float_t", "");
        cl.def( pybind11::init<const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &>(), pybind11::arg("p"), pybind11::arg("q") );

        cl.def( pybind11::init( [](){ return new easy3d::GenericOrientedLine<float>(); } ) );
        cl.def( pybind11::init( [](easy3d::GenericOrientedLine<float> const &o){ return new easy3d::GenericOrientedLine<float>(o); } ) );
        cl.def_static("side", (enum easy3d::Sign (*)(const class easy3d::GenericOrientedLine<float> &, const class easy3d::GenericOrientedLine<float> &)) &easy3d::GenericOrientedLine<float>::side, "C++: easy3d::GenericOrientedLine<float>::side(const class easy3d::GenericOrientedLine<float> &, const class easy3d::GenericOrientedLine<float> &) --> enum easy3d::Sign", pybind11::arg("a"), pybind11::arg("b"));
    }

}
