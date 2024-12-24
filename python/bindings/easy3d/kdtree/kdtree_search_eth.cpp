#include <easy3d/core/vec.h>
#include <easy3d/kdtree/kdtree_search.h>
#include <easy3d/kdtree/kdtree_search_eth.h>

#include <memory>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif


// easy3d::KdTreeSearch_ETH file:easy3d/kdtree/kdtree_search_eth.h line:42
struct PyCallBack_easy3d_KdTreeSearch_ETH : public easy3d::KdTreeSearch_ETH {
    using easy3d::KdTreeSearch_ETH::KdTreeSearch_ETH;

    int find_closest_point(const class easy3d::Vec<3, float> & a0, float & a1) const override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::KdTreeSearch_ETH *>(this), "find_closest_point");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value) {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<int>(std::move(o));
        }
        return KdTreeSearch_ETH::find_closest_point(a0, a1);
    }
    int find_closest_point(const class easy3d::Vec<3, float> & a0) const override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::KdTreeSearch_ETH *>(this), "find_closest_point");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value) {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<int>(std::move(o));
        }
        return KdTreeSearch_ETH::find_closest_point(a0);
    }
    void find_closest_k_points(const class easy3d::Vec<3, float> & a0, int a1, class std::vector<int> & a2, class std::vector<float> & a3) const override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::KdTreeSearch_ETH *>(this), "find_closest_k_points");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2, a3);
            if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
                static pybind11::detail::override_caster_t<void> caster;
                return pybind11::detail::cast_ref<void>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<void>(std::move(o));
        }
        return KdTreeSearch_ETH::find_closest_k_points(a0, a1, a2, a3);
    }
    void find_closest_k_points(const class easy3d::Vec<3, float> & a0, int a1, class std::vector<int> & a2) const override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::KdTreeSearch_ETH *>(this), "find_closest_k_points");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2);
            if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
                static pybind11::detail::override_caster_t<void> caster;
                return pybind11::detail::cast_ref<void>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<void>(std::move(o));
        }
        return KdTreeSearch_ETH::find_closest_k_points(a0, a1, a2);
    }
    void find_points_in_range(const class easy3d::Vec<3, float> & a0, float a1, class std::vector<int> & a2, class std::vector<float> & a3) const override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::KdTreeSearch_ETH *>(this), "find_points_in_range");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2, a3);
            if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
                static pybind11::detail::override_caster_t<void> caster;
                return pybind11::detail::cast_ref<void>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<void>(std::move(o));
        }
        return KdTreeSearch_ETH::find_points_in_range(a0, a1, a2, a3);
    }
    void find_points_in_range(const class easy3d::Vec<3, float> & a0, float a1, class std::vector<int> & a2) const override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::KdTreeSearch_ETH *>(this), "find_points_in_range");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2);
            if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
                static pybind11::detail::override_caster_t<void> caster;
                return pybind11::detail::cast_ref<void>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<void>(std::move(o));
        }
        return KdTreeSearch_ETH::find_points_in_range(a0, a1, a2);
    }
};


void bind_easy3d_kdtree_kdtree_search_eth(pybind11::module_& m)
{
	{ // easy3d::KdTreeSearch_ETH file:easy3d/kdtree/kdtree_search_eth.h line:42
		pybind11::class_<easy3d::KdTreeSearch_ETH, std::shared_ptr<easy3d::KdTreeSearch_ETH>, PyCallBack_easy3d_KdTreeSearch_ETH, easy3d::KdTreeSearch> cl(m, "KdTreeSearch_ETH", "KdTree implementation based on Richard Keiser's KdTree code.\n \n\n\n \n KdTreeSearch_ANN, KdTreeSearch_FLANN, and KdTreeSearch_NanoFLANN.");
		cl.def( pybind11::init<const class std::vector<class easy3d::Vec<3, float> > &>(), pybind11::arg("points") );

		cl.def( pybind11::init( [](PyCallBack_easy3d_KdTreeSearch_ETH const &o){ return new PyCallBack_easy3d_KdTreeSearch_ETH(o); } ) );
		cl.def( pybind11::init( [](easy3d::KdTreeSearch_ETH const &o){ return new easy3d::KdTreeSearch_ETH(o); } ) );
		cl.def("find_closest_point", (int (easy3d::KdTreeSearch_ETH::*)(const class easy3d::Vec<3, float> &, float &) const) &easy3d::KdTreeSearch_ETH::find_closest_point, "Queries the closest point for a given point.\n \n\n The query point.\n \n\n The squared distance between the query point and its closest neighbor.\n \n\n A  distance is returned by the second argument \n \n\n The index of the nearest neighbor found.\n\nC++: easy3d::KdTreeSearch_ETH::find_closest_point(const class easy3d::Vec<3, float> &, float &) const --> int", pybind11::arg("p"), pybind11::arg("squared_distance"));
		cl.def("find_closest_point", (int (easy3d::KdTreeSearch_ETH::*)(const class easy3d::Vec<3, float> &) const) &easy3d::KdTreeSearch_ETH::find_closest_point, "Queries the closest point for a given point.\n \n\n The query point.\n \n\n The index of the nearest neighbor found.\n\nC++: easy3d::KdTreeSearch_ETH::find_closest_point(const class easy3d::Vec<3, float> &) const --> int", pybind11::arg("p"));
		cl.def("find_closest_k_points", (void (easy3d::KdTreeSearch_ETH::*)(const class easy3d::Vec<3, float> &, int, class std::vector<int> &, class std::vector<float> &) const) &easy3d::KdTreeSearch_ETH::find_closest_k_points, "Queries the K nearest neighbors for a given point.\n \n\n The query point.\n \n\n The number of required neighbors.\n \n\n The indices of the neighbors found.\n \n\n The squared distances between the query point and its K nearest neighbors.\n The values are stored in accordance with their indices.\n \n\n The  distances are returned by the argument \n         \n\nC++: easy3d::KdTreeSearch_ETH::find_closest_k_points(const class easy3d::Vec<3, float> &, int, class std::vector<int> &, class std::vector<float> &) const --> void", pybind11::arg("p"), pybind11::arg("k"), pybind11::arg("neighbors"), pybind11::arg("squared_distances"));
		cl.def("find_closest_k_points", (void (easy3d::KdTreeSearch_ETH::*)(const class easy3d::Vec<3, float> &, int, class std::vector<int> &) const) &easy3d::KdTreeSearch_ETH::find_closest_k_points, "Queries the K nearest neighbors for a given point.\n \n\n The query point.\n \n\n The number of required neighbors.\n \n\n The indices of the neighbors found.\n\nC++: easy3d::KdTreeSearch_ETH::find_closest_k_points(const class easy3d::Vec<3, float> &, int, class std::vector<int> &) const --> void", pybind11::arg("p"), pybind11::arg("k"), pybind11::arg("neighbors"));
		cl.def("find_points_in_range", (void (easy3d::KdTreeSearch_ETH::*)(const class easy3d::Vec<3, float> &, float, class std::vector<int> &, class std::vector<float> &) const) &easy3d::KdTreeSearch_ETH::find_points_in_range, "Queries the nearest neighbors within a fixed range.\n \n\n The query point.\n \n\n The search range (which is required to be \n \n\n The indices of the neighbors found.\n \n\n The squared distances between the query point and the neighbors found.\n The values are stored in accordance with their indices.\n \n\n The  distances are returned by the argument \n         \n\nC++: easy3d::KdTreeSearch_ETH::find_points_in_range(const class easy3d::Vec<3, float> &, float, class std::vector<int> &, class std::vector<float> &) const --> void", pybind11::arg("p"), pybind11::arg("squared_radius"), pybind11::arg("neighbors"), pybind11::arg("squared_distances"));
		cl.def("find_points_in_range", (void (easy3d::KdTreeSearch_ETH::*)(const class easy3d::Vec<3, float> &, float, class std::vector<int> &) const) &easy3d::KdTreeSearch_ETH::find_points_in_range, "Queries the nearest neighbors within a fixed range.\n \n\n The query point.\n \n\n The search range (which is required to be \n \n\n The indices of the neighbors found.\n\nC++: easy3d::KdTreeSearch_ETH::find_points_in_range(const class easy3d::Vec<3, float> &, float, class std::vector<int> &) const --> void", pybind11::arg("p"), pybind11::arg("squared_radius"), pybind11::arg("neighbors"));
		cl.def("find_points_in_cylinder", [](easy3d::KdTreeSearch_ETH const &o, const class easy3d::Vec<3, float> & a0, const class easy3d::Vec<3, float> & a1, float const & a2, class std::vector<int> & a3, class std::vector<float> & a4) -> int { return o.find_points_in_cylinder(a0, a1, a2, a3, a4); }, "", pybind11::arg("p1"), pybind11::arg("p2"), pybind11::arg("radius"), pybind11::arg("neighbors"), pybind11::arg("squared_distances"));
		cl.def("find_points_in_cylinder", (int (easy3d::KdTreeSearch_ETH::*)(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, float, class std::vector<int> &, class std::vector<float> &, bool) const) &easy3d::KdTreeSearch_ETH::find_points_in_cylinder, "Queries the nearest neighbors within a cylinder range.\n \n\n Searches for the nearest points whose distances to line segment  -  are smaller than\n  ( squared).\n \n\n One end point of the query line segment.\n \n\n The other end point of the query line segment.\n \n\n The search range (which is  squared).\n \n\n The indices of the neighbors found.\n \n\n The squared distances between the found neighbors to the query line. The values\n are stored in accordance with their indices.\n \n\n Specifies how the points found are ordered. If =true, the points found are ordered\n by their distances to the line. Otherwise, they are ordered by their distances to \n \n\n The range is specified by radius ( squared) but  distances are returned.\n\nC++: easy3d::KdTreeSearch_ETH::find_points_in_cylinder(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, float, class std::vector<int> &, class std::vector<float> &, bool) const --> int", pybind11::arg("p1"), pybind11::arg("p2"), pybind11::arg("radius"), pybind11::arg("neighbors"), pybind11::arg("squared_distances"), pybind11::arg("bToLine"));
		cl.def("find_points_in_cylinder", [](easy3d::KdTreeSearch_ETH const &o, const class easy3d::Vec<3, float> & a0, const class easy3d::Vec<3, float> & a1, float const & a2, class std::vector<int> & a3) -> int { return o.find_points_in_cylinder(a0, a1, a2, a3); }, "", pybind11::arg("p1"), pybind11::arg("p2"), pybind11::arg("radius"), pybind11::arg("neighbors"));
		cl.def("find_points_in_cylinder", (int (easy3d::KdTreeSearch_ETH::*)(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, float, class std::vector<int> &, bool) const) &easy3d::KdTreeSearch_ETH::find_points_in_cylinder, "Queries the nearest neighbors within a cylinder range.\n \n\n Searches for the nearest points whose distances to line segment  -  are smaller than\n  ( squared).\n \n\n One end point of the query line segment.\n \n\n The other end point of the query line segment.\n \n\n The search range (which is  squared).\n \n\n The indices of the neighbors found.\n \n\n Specifies how the points found are ordered. If =true, the points found are ordered\n by their distances to the line. Otherwise, they are ordered by their distances to \n \n\n The range is specified by radius ( squared) but  distances are returned.\n\nC++: easy3d::KdTreeSearch_ETH::find_points_in_cylinder(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, float, class std::vector<int> &, bool) const --> int", pybind11::arg("p1"), pybind11::arg("p2"), pybind11::arg("radius"), pybind11::arg("neighbors"), pybind11::arg("bToLine"));
		cl.def("find_points_in_cone", [](easy3d::KdTreeSearch_ETH const &o, const class easy3d::Vec<3, float> & a0, const class easy3d::Vec<3, float> & a1, const class easy3d::Vec<3, float> & a2, float const & a3, class std::vector<int> & a4, class std::vector<float> & a5) -> int { return o.find_points_in_cone(a0, a1, a2, a3, a4, a5); }, "", pybind11::arg("eye"), pybind11::arg("p1"), pybind11::arg("p2"), pybind11::arg("angle_range"), pybind11::arg("neighbors"), pybind11::arg("squared_distances"));
		cl.def("find_points_in_cone", (int (easy3d::KdTreeSearch_ETH::*)(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, float, class std::vector<int> &, class std::vector<float> &, bool) const) &easy3d::KdTreeSearch_ETH::find_points_in_cone, "Queries the nearest neighbors within a cone.\n \n\n Searches for the nearest points with a cone from  to \n \n\n The view point.\n \n\n One end point of a line segment.\n \n\n The other end point of a line segment.\n \n\n The maximal angle in radian allowed between  -  and  -  where \n is a candidate point.\n \n\n The indices of the neighbors found.\n \n\n The squared distances between the found neighbors to the query line segment.\n The values are stored in accordance with their indices.\n \n\n Specifies how the points found are ordered. If =true, the points found are ordered\n by their distances to the line. Otherwise, they are ordered by their distances to the view point \n \n\n The angle is specified in  and  distances are returned.\n\nC++: easy3d::KdTreeSearch_ETH::find_points_in_cone(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, float, class std::vector<int> &, class std::vector<float> &, bool) const --> int", pybind11::arg("eye"), pybind11::arg("p1"), pybind11::arg("p2"), pybind11::arg("angle_range"), pybind11::arg("neighbors"), pybind11::arg("squared_distances"), pybind11::arg("bToLine"));
		cl.def("find_points_in_cone", [](easy3d::KdTreeSearch_ETH const &o, const class easy3d::Vec<3, float> & a0, const class easy3d::Vec<3, float> & a1, const class easy3d::Vec<3, float> & a2, float const & a3, class std::vector<int> & a4) -> int { return o.find_points_in_cone(a0, a1, a2, a3, a4); }, "", pybind11::arg("eye"), pybind11::arg("p1"), pybind11::arg("p2"), pybind11::arg("angle_range"), pybind11::arg("neighbors"));
		cl.def("find_points_in_cone", (int (easy3d::KdTreeSearch_ETH::*)(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, float, class std::vector<int> &, bool) const) &easy3d::KdTreeSearch_ETH::find_points_in_cone, "Queries the nearest neighbors within a cone.\n \n\n Searches for the nearest points with a cone from  to \n \n\n The view point.\n \n\n One end point of a line segment.\n \n\n The other end point of a line segment.\n \n\n The maximal angle in radian allowed between  -  and  -  where \n is a candidate point.\n \n\n The indices of the neighbors found.\n \n\n Specifies how the points found are ordered. If  the points found are ordered\n by their distances to the line. Otherwise, they are ordered by their distances to the view point \n \n\n The angle is specified in  and  distances are returned.\n\nC++: easy3d::KdTreeSearch_ETH::find_points_in_cone(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, float, class std::vector<int> &, bool) const --> int", pybind11::arg("eye"), pybind11::arg("p1"), pybind11::arg("p2"), pybind11::arg("angle_range"), pybind11::arg("neighbors"), pybind11::arg("bToLine"));
		cl.def("assign", (class easy3d::KdTreeSearch_ETH & (easy3d::KdTreeSearch_ETH::*)(const class easy3d::KdTreeSearch_ETH &)) &easy3d::KdTreeSearch_ETH::operator=, "C++: easy3d::KdTreeSearch_ETH::operator=(const class easy3d::KdTreeSearch_ETH &) --> class easy3d::KdTreeSearch_ETH &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}

}
