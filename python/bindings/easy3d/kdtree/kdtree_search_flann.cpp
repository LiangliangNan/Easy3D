#include <easy3d/core/vec.h>
#include <easy3d/kdtree/kdtree_search_flann.h>

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

// easy3d::KdTreeSearch_FLANN file:easy3d/kdtree/kdtree_search_flann.h line:42
struct PyCallBack_easy3d_KdTreeSearch_FLANN : public easy3d::KdTreeSearch_FLANN {
	using easy3d::KdTreeSearch_FLANN::KdTreeSearch_FLANN;

	int find_closest_point(const class easy3d::Vec<3, float> & a0, float & a1) const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::KdTreeSearch_FLANN *>(this), "find_closest_point");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
			if (pybind11::detail::cast_is_temporary_value_reference<int>::value) {
				static pybind11::detail::override_caster_t<int> caster;
				return pybind11::detail::cast_ref<int>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<int>(std::move(o));
		}
		return KdTreeSearch_FLANN::find_closest_point(a0, a1);
	}
	int find_closest_point(const class easy3d::Vec<3, float> & a0) const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::KdTreeSearch_FLANN *>(this), "find_closest_point");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
			if (pybind11::detail::cast_is_temporary_value_reference<int>::value) {
				static pybind11::detail::override_caster_t<int> caster;
				return pybind11::detail::cast_ref<int>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<int>(std::move(o));
		}
		return KdTreeSearch_FLANN::find_closest_point(a0);
	}
	void find_closest_k_points(const class easy3d::Vec<3, float> & a0, int a1, class std::vector<int> & a2, class std::vector<float> & a3) const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::KdTreeSearch_FLANN *>(this), "find_closest_k_points");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2, a3);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return KdTreeSearch_FLANN::find_closest_k_points(a0, a1, a2, a3);
	}
	void find_closest_k_points(const class easy3d::Vec<3, float> & a0, int a1, class std::vector<int> & a2) const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::KdTreeSearch_FLANN *>(this), "find_closest_k_points");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return KdTreeSearch_FLANN::find_closest_k_points(a0, a1, a2);
	}
	void find_points_in_range(const class easy3d::Vec<3, float> & a0, float a1, class std::vector<int> & a2, class std::vector<float> & a3) const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::KdTreeSearch_FLANN *>(this), "find_points_in_range");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2, a3);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return KdTreeSearch_FLANN::find_points_in_range(a0, a1, a2, a3);
	}
	void find_points_in_range(const class easy3d::Vec<3, float> & a0, float a1, class std::vector<int> & a2) const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::KdTreeSearch_FLANN *>(this), "find_points_in_range");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return KdTreeSearch_FLANN::find_points_in_range(a0, a1, a2);
	}
};


void bind_easy3d_kdtree_kdtree_search_flann(pybind11::module_& m)
{
	{ // easy3d::KdTreeSearch_FLANN file:easy3d/kdtree/kdtree_search_flann.h line:42
		pybind11::class_<easy3d::KdTreeSearch_FLANN, std::shared_ptr<easy3d::KdTreeSearch_FLANN>, PyCallBack_easy3d_KdTreeSearch_FLANN, easy3d::KdTreeSearch> cl(m, "KdTreeSearch_FLANN", "KdTree implementation based on [FLANN](https://github.com/mariusmuja/flann).\n \n\n\n \n KdTreeSearch_ANN, KdTreeSearch_ETH, and KdTreeSearch_NanoFLANN.");
		cl.def( pybind11::init<const class std::vector<class easy3d::Vec<3, float> > &>(), pybind11::arg("points") );

		cl.def( pybind11::init( [](PyCallBack_easy3d_KdTreeSearch_FLANN const &o){ return new PyCallBack_easy3d_KdTreeSearch_FLANN(o); } ) );
		cl.def( pybind11::init( [](easy3d::KdTreeSearch_FLANN const &o){ return new easy3d::KdTreeSearch_FLANN(o); } ) );
		cl.def("set_checks", (void (easy3d::KdTreeSearch_FLANN::*)(int)) &easy3d::KdTreeSearch_FLANN::set_checks, "Specifies the maximum number of leaves to visit when searching for neighbors.\n \n\n The maximum number of leaves to visit.\n A higher value for this parameter would give better search precision, but also take more time.\n For all leaves to be checked use the value FLANN_CHECKS_UNLIMITED. If automatic configuration was used when\n the index was created, the number of checks required to achieve the specified precision was also computed,\n to use that value specify FLANN_CHECKS_AUTOTUNED. The default value is FLANN_CHECKS_AUTOTUNED.\n (Liangliang: 32 is also a good value).\n\nC++: easy3d::KdTreeSearch_FLANN::set_checks(int) --> void", pybind11::arg("chk"));
		cl.def("find_closest_point", (int (easy3d::KdTreeSearch_FLANN::*)(const class easy3d::Vec<3, float> &, float &) const) &easy3d::KdTreeSearch_FLANN::find_closest_point, "Queries the closest point for a given point.\n \n\n The query point.\n \n\n The squared distance between the query point and its closest neighbor.\n \n\n A  distance is returned by the second argument \n \n\n The index of the nearest neighbor found.\n\nC++: easy3d::KdTreeSearch_FLANN::find_closest_point(const class easy3d::Vec<3, float> &, float &) const --> int", pybind11::arg("p"), pybind11::arg("squared_distance"));
		cl.def("find_closest_point", (int (easy3d::KdTreeSearch_FLANN::*)(const class easy3d::Vec<3, float> &) const) &easy3d::KdTreeSearch_FLANN::find_closest_point, "Queries the closest point for a given point.\n \n\n The query point.\n \n\n The index of the nearest neighbor found.\n\nC++: easy3d::KdTreeSearch_FLANN::find_closest_point(const class easy3d::Vec<3, float> &) const --> int", pybind11::arg("p"));
		cl.def("find_closest_k_points", (void (easy3d::KdTreeSearch_FLANN::*)(const class easy3d::Vec<3, float> &, int, class std::vector<int> &, class std::vector<float> &) const) &easy3d::KdTreeSearch_FLANN::find_closest_k_points, "Queries the K nearest neighbors for a given point.\n \n\n The query point.\n \n\n The number of required neighbors.\n \n\n The indices of the neighbors found.\n \n\n The squared distances between the query point and its K nearest neighbors.\n The values are stored in accordance with their indices.\n \n\n The  distances are returned by the argument \n         \n\nC++: easy3d::KdTreeSearch_FLANN::find_closest_k_points(const class easy3d::Vec<3, float> &, int, class std::vector<int> &, class std::vector<float> &) const --> void", pybind11::arg("p"), pybind11::arg("k"), pybind11::arg("neighbors"), pybind11::arg("squared_distances"));
		cl.def("find_closest_k_points", (void (easy3d::KdTreeSearch_FLANN::*)(const class easy3d::Vec<3, float> &, int, class std::vector<int> &) const) &easy3d::KdTreeSearch_FLANN::find_closest_k_points, "Queries the K nearest neighbors for a given point.\n \n\n The query point.\n \n\n The number of required neighbors.\n \n\n The indices of the neighbors found.\n\nC++: easy3d::KdTreeSearch_FLANN::find_closest_k_points(const class easy3d::Vec<3, float> &, int, class std::vector<int> &) const --> void", pybind11::arg("p"), pybind11::arg("k"), pybind11::arg("neighbors"));
		cl.def("find_points_in_range", (void (easy3d::KdTreeSearch_FLANN::*)(const class easy3d::Vec<3, float> &, float, class std::vector<int> &, class std::vector<float> &) const) &easy3d::KdTreeSearch_FLANN::find_points_in_range, "Queries the nearest neighbors within a fixed range.\n \n\n The query point.\n \n\n The search range (which is required to be \n \n\n The indices of the neighbors found.\n \n\n The squared distances between the query point and the neighbors found.\n The values are stored in accordance with their indices.\n \n\n The  distances are returned by the argument \n         \n\nC++: easy3d::KdTreeSearch_FLANN::find_points_in_range(const class easy3d::Vec<3, float> &, float, class std::vector<int> &, class std::vector<float> &) const --> void", pybind11::arg("p"), pybind11::arg("squared_radius"), pybind11::arg("neighbors"), pybind11::arg("squared_distances"));
		cl.def("find_points_in_range", (void (easy3d::KdTreeSearch_FLANN::*)(const class easy3d::Vec<3, float> &, float, class std::vector<int> &) const) &easy3d::KdTreeSearch_FLANN::find_points_in_range, "Queries the nearest neighbors within a fixed range.\n \n\n The query point.\n \n\n The search range (which is required to be \n \n\n The indices of the neighbors found.\n\nC++: easy3d::KdTreeSearch_FLANN::find_points_in_range(const class easy3d::Vec<3, float> &, float, class std::vector<int> &) const --> void", pybind11::arg("p"), pybind11::arg("squared_radius"), pybind11::arg("neighbors"));
		cl.def("assign", (class easy3d::KdTreeSearch_FLANN & (easy3d::KdTreeSearch_FLANN::*)(const class easy3d::KdTreeSearch_FLANN &)) &easy3d::KdTreeSearch_FLANN::operator=, "C++: easy3d::KdTreeSearch_FLANN::operator=(const class easy3d::KdTreeSearch_FLANN &) --> class easy3d::KdTreeSearch_FLANN &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}
}
