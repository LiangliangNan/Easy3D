#include <easy3d/algo/surface_mesh_simplification.h>
#include <easy3d/core/surface_mesh.h>

#include <memory>

#include <pybind11/pybind11.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif



//// easy3d::Property file:easy3d/core/property.h line:252
//struct PyCallBack_easy3d_Property_easy3d_Quadric_t : public easy3d::Property<easy3d::Quadric> {
//    using easy3d::Property<easy3d::Quadric>::Property;
//
//    class easy3d::Quadric & operator[](size_t a0) override {
//        pybind11::gil_scoped_acquire gil;
//        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Property<easy3d::Quadric> *>(this), "__getitem__");
//        if (overload) {
//            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
//            if (pybind11::detail::cast_is_temporary_value_reference<class easy3d::Quadric &>::value) {
//                static pybind11::detail::override_caster_t<class easy3d::Quadric &> caster;
//                return pybind11::detail::cast_ref<class easy3d::Quadric &>(std::move(o), caster);
//            }
//            return pybind11::detail::cast_safe<class easy3d::Quadric &>(std::move(o));
//        }
//        return Property::operator[](a0);
//    }
//};
//
//// easy3d::Property file:easy3d/core/property.h line:252
//struct PyCallBack_easy3d_Property_easy3d_NormalCone_t : public easy3d::Property<easy3d::NormalCone> {
//    using easy3d::Property<easy3d::NormalCone>::Property;
//
//    class easy3d::NormalCone & operator[](size_t a0) override {
//        pybind11::gil_scoped_acquire gil;
//        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Property<easy3d::NormalCone> *>(this), "__getitem__");
//        if (overload) {
//            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
//            if (pybind11::detail::cast_is_temporary_value_reference<class easy3d::NormalCone &>::value) {
//                static pybind11::detail::override_caster_t<class easy3d::NormalCone &> caster;
//                return pybind11::detail::cast_ref<class easy3d::NormalCone &>(std::move(o), caster);
//            }
//            return pybind11::detail::cast_safe<class easy3d::NormalCone &>(std::move(o));
//        }
//        return Property::operator[](a0);
//    }
//};


void bind_easy3d_algo_surface_mesh_simplification(pybind11::module_& m)
{
	{ // easy3d::SurfaceMeshSimplification file:easy3d/algo/surface_mesh_simplification.h line:166
		pybind11::class_<easy3d::SurfaceMeshSimplification, std::shared_ptr<easy3d::SurfaceMeshSimplification>> cl(m, "SurfaceMeshSimplification", "Surface mesh simplification based on approximation error and fairness criteria.\n \n\n\n \n It performs incremental greedy mesh simplification based on halfedge collapses. See the following paper\n for more details:\n  - Michael Garland and Paul Seagrave Heckbert. Surface simplification using quadric error metrics. SIGGRAPH 1997.\n  - Leif Kobbelt et al. A general framework for mesh decimation. In Proceedings of Graphics Interface, 1998.");
		cl.def( pybind11::init<class easy3d::SurfaceMesh *>(), pybind11::arg("mesh") );

		cl.def("initialize", [](easy3d::SurfaceMeshSimplification &o) -> void { return o.initialize(); }, "");
		cl.def("initialize", [](easy3d::SurfaceMeshSimplification &o, float const & a0) -> void { return o.initialize(a0); }, "", pybind11::arg("aspect_ratio"));
		cl.def("initialize", [](easy3d::SurfaceMeshSimplification &o, float const & a0, float const & a1) -> void { return o.initialize(a0, a1); }, "", pybind11::arg("aspect_ratio"), pybind11::arg("edge_length"));
		cl.def("initialize", [](easy3d::SurfaceMeshSimplification &o, float const & a0, float const & a1, unsigned int const & a2) -> void { return o.initialize(a0, a1, a2); }, "", pybind11::arg("aspect_ratio"), pybind11::arg("edge_length"), pybind11::arg("max_valence"));
		cl.def("initialize", [](easy3d::SurfaceMeshSimplification &o, float const & a0, float const & a1, unsigned int const & a2, float const & a3) -> void { return o.initialize(a0, a1, a2, a3); }, "", pybind11::arg("aspect_ratio"), pybind11::arg("edge_length"), pybind11::arg("max_valence"), pybind11::arg("normal_deviation"));
		cl.def("initialize", (void (easy3d::SurfaceMeshSimplification::*)(float, float, unsigned int, float, float)) &easy3d::SurfaceMeshSimplification::initialize, "Initialize with given parameters.\n\nC++: easy3d::SurfaceMeshSimplification::initialize(float, float, unsigned int, float, float) --> void", pybind11::arg("aspect_ratio"), pybind11::arg("edge_length"), pybind11::arg("max_valence"), pybind11::arg("normal_deviation"), pybind11::arg("hausdorff_error"));
		cl.def("simplify", (void (easy3d::SurfaceMeshSimplification::*)(unsigned int)) &easy3d::SurfaceMeshSimplification::simplify, "Simplify mesh to  vertices.\n\nC++: easy3d::SurfaceMeshSimplification::simplify(unsigned int) --> void", pybind11::arg("n_vertices"));
	}

//    { // easy3d::Quadric file:easy3d/algo/surface_mesh_simplification.h line:25
//        pybind11::class_<easy3d::Quadric, std::shared_ptr<easy3d::Quadric>> cl(m, "Quadric", "A quadric as a symmetric 4x4 matrix. Used by the error quadric mesh decimation algorithms.");
//        cl.def( pybind11::init<double, double, double, double, double, double, double, double, double, double>(), pybind11::arg("a"), pybind11::arg("b"), pybind11::arg("c"), pybind11::arg("d"), pybind11::arg("e"), pybind11::arg("f"), pybind11::arg("g"), pybind11::arg("h"), pybind11::arg("i"), pybind11::arg("j") );
//
//        cl.def( pybind11::init( [](){ return new easy3d::Quadric(); } ), "doc" );
//        cl.def( pybind11::init( [](double const & a0){ return new easy3d::Quadric(a0); } ), "doc" , pybind11::arg("a"));
//        cl.def( pybind11::init( [](double const & a0, double const & a1){ return new easy3d::Quadric(a0, a1); } ), "doc" , pybind11::arg("a"), pybind11::arg("b"));
//        cl.def( pybind11::init( [](double const & a0, double const & a1, double const & a2){ return new easy3d::Quadric(a0, a1, a2); } ), "doc" , pybind11::arg("a"), pybind11::arg("b"), pybind11::arg("c"));
//        cl.def( pybind11::init<double, double, double, double>(), pybind11::arg("a"), pybind11::arg("b"), pybind11::arg("c"), pybind11::arg("d") );
//
//        cl.def( pybind11::init<const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &>(), pybind11::arg("n"), pybind11::arg("p") );
//
//        cl.def( pybind11::init( [](easy3d::Quadric const &o){ return new easy3d::Quadric(o); } ) );
//        cl.def("clear", (void (easy3d::Quadric::*)()) &easy3d::Quadric::clear, "set all matrix entries to zero\n\nC++: easy3d::Quadric::clear() --> void");
//        cl.def("__iadd__", (class easy3d::Quadric & (easy3d::Quadric::*)(const class easy3d::Quadric &)) &easy3d::Quadric::operator+=, "add given quadric to this quadric\n\nC++: easy3d::Quadric::operator+=(const class easy3d::Quadric &) --> class easy3d::Quadric &", pybind11::return_value_policy::automatic, pybind11::arg("q"));
//        cl.def("__imul__", (class easy3d::Quadric & (easy3d::Quadric::*)(double)) &easy3d::Quadric::operator*=, "multiply quadric by a scalar\n\nC++: easy3d::Quadric::operator*=(double) --> class easy3d::Quadric &", pybind11::return_value_policy::automatic, pybind11::arg("s"));
//        cl.def("__call__", (double (easy3d::Quadric::*)(const class easy3d::Vec<3, float> &) const) &easy3d::Quadric::operator(), "evaluate quadric Q at position p by computing (p^T * Q * p)\n\nC++: easy3d::Quadric::operator()(const class easy3d::Vec<3, float> &) const --> double", pybind11::arg("p"));
//        cl.def("assign", (class easy3d::Quadric & (easy3d::Quadric::*)(const class easy3d::Quadric &)) &easy3d::Quadric::operator=, "C++: easy3d::Quadric::operator=(const class easy3d::Quadric &) --> class easy3d::Quadric &", pybind11::return_value_policy::automatic, pybind11::arg(""));
//    }
//    { // easy3d::NormalCone file:easy3d/algo/surface_mesh_simplification.h line:102
//        pybind11::class_<easy3d::NormalCone, std::shared_ptr<easy3d::NormalCone>> cl(m, "NormalCone", "A class implementing a normal cone.");
//        cl.def( pybind11::init( [](){ return new easy3d::NormalCone(); } ) );
//        cl.def( pybind11::init( [](const class easy3d::Vec<3, float> & a0){ return new easy3d::NormalCone(a0); } ), "doc" , pybind11::arg("normal"));
//        cl.def( pybind11::init<const class easy3d::Vec<3, float> &, float>(), pybind11::arg("normal"), pybind11::arg("angle") );
//
//        cl.def( pybind11::init( [](easy3d::NormalCone const &o){ return new easy3d::NormalCone(o); } ) );
//        cl.def("center_normal", (const class easy3d::Vec<3, float> & (easy3d::NormalCone::*)() const) &easy3d::NormalCone::center_normal, "returns center normal\n\nC++: easy3d::NormalCone::center_normal() const --> const class easy3d::Vec<3, float> &", pybind11::return_value_policy::automatic);
//        cl.def("angle", (float (easy3d::NormalCone::*)() const) &easy3d::NormalCone::angle, "returns size of cone (radius in radians)\n\nC++: easy3d::NormalCone::angle() const --> float");
//        cl.def("merge", (class easy3d::NormalCone & (easy3d::NormalCone::*)(const class easy3d::Vec<3, float> &)) &easy3d::NormalCone::merge, "merge *this with n.\n\nC++: easy3d::NormalCone::merge(const class easy3d::Vec<3, float> &) --> class easy3d::NormalCone &", pybind11::return_value_policy::automatic, pybind11::arg("n"));
//        cl.def("merge", (class easy3d::NormalCone & (easy3d::NormalCone::*)(const class easy3d::NormalCone &)) &easy3d::NormalCone::merge, "merge *this with nc. *this will then enclose both cones.\n\nC++: easy3d::NormalCone::merge(const class easy3d::NormalCone &) --> class easy3d::NormalCone &", pybind11::return_value_policy::automatic, pybind11::arg("nc"));
//        cl.def("assign", (class easy3d::NormalCone & (easy3d::NormalCone::*)(const class easy3d::NormalCone &)) &easy3d::NormalCone::operator=, "C++: easy3d::NormalCone::operator=(const class easy3d::NormalCone &) --> class easy3d::NormalCone &", pybind11::return_value_policy::automatic, pybind11::arg(""));
//    }
}
