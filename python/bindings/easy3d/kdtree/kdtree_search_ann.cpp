#include <easy3d/core/point_cloud.h>
#include <easy3d/core/vec.h>
#include <easy3d/kdtree/kdtree_search_ann.h>

#include <memory>
#include <typeinfo>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>


#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif


// easy3d::KdTreeSearch file:easy3d/kdtree/kdtree_search.h line:80
struct PyCallBack_easy3d_KdTreeSearch : public easy3d::KdTreeSearch {
    using easy3d::KdTreeSearch::KdTreeSearch;

    int find_closest_point(const class easy3d::Vec<3, float> & a0, float & a1) const override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::KdTreeSearch *>(this), "find_closest_point");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value) {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<int>(std::move(o));
        }
        pybind11::pybind11_fail("Tried to call pure virtual function \"KdTreeSearch::find_closest_point\"");
    }
    int find_closest_point(const class easy3d::Vec<3, float> & a0) const override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::KdTreeSearch *>(this), "find_closest_point");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<int>::value) {
                static pybind11::detail::override_caster_t<int> caster;
                return pybind11::detail::cast_ref<int>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<int>(std::move(o));
        }
        pybind11::pybind11_fail("Tried to call pure virtual function \"KdTreeSearch::find_closest_point\"");
    }
    void find_closest_k_points(const class easy3d::Vec<3, float> & a0, int a1, class std::vector<int> & a2, class std::vector<float> & a3) const override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::KdTreeSearch *>(this), "find_closest_k_points");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2, a3);
            if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
                static pybind11::detail::override_caster_t<void> caster;
                return pybind11::detail::cast_ref<void>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<void>(std::move(o));
        }
        pybind11::pybind11_fail("Tried to call pure virtual function \"KdTreeSearch::find_closest_k_points\"");
    }
    void find_closest_k_points(const class easy3d::Vec<3, float> & a0, int a1, class std::vector<int> & a2) const override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::KdTreeSearch *>(this), "find_closest_k_points");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2);
            if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
                static pybind11::detail::override_caster_t<void> caster;
                return pybind11::detail::cast_ref<void>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<void>(std::move(o));
        }
        pybind11::pybind11_fail("Tried to call pure virtual function \"KdTreeSearch::find_closest_k_points\"");
    }
    void find_points_in_range(const class easy3d::Vec<3, float> & a0, float a1, class std::vector<int> & a2, class std::vector<float> & a3) const override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::KdTreeSearch *>(this), "find_points_in_range");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2, a3);
            if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
                static pybind11::detail::override_caster_t<void> caster;
                return pybind11::detail::cast_ref<void>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<void>(std::move(o));
        }
        pybind11::pybind11_fail("Tried to call pure virtual function \"KdTreeSearch::find_points_in_range\"");
    }
    void find_points_in_range(const class easy3d::Vec<3, float> & a0, float a1, class std::vector<int> & a2) const override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::KdTreeSearch *>(this), "find_points_in_range");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2);
            if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
                static pybind11::detail::override_caster_t<void> caster;
                return pybind11::detail::cast_ref<void>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<void>(std::move(o));
        }
        pybind11::pybind11_fail("Tried to call pure virtual function \"KdTreeSearch::find_points_in_range\"");
    }
};


// easy3d::KdTreeSearch_ANN file:easy3d/kdtree/kdtree_search_ann.h line:42
struct PyCallBack_easy3d_KdTreeSearch_ANN : public easy3d::KdTreeSearch_ANN {
	using easy3d::KdTreeSearch_ANN::KdTreeSearch_ANN;

	int find_closest_point(const class easy3d::Vec<3, float> & a0, float & a1) const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::KdTreeSearch_ANN *>(this), "find_closest_point");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
			if (pybind11::detail::cast_is_temporary_value_reference<int>::value) {
				static pybind11::detail::override_caster_t<int> caster;
				return pybind11::detail::cast_ref<int>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<int>(std::move(o));
		}
		return KdTreeSearch_ANN::find_closest_point(a0, a1);
	}
	int find_closest_point(const class easy3d::Vec<3, float> & a0) const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::KdTreeSearch_ANN *>(this), "find_closest_point");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
			if (pybind11::detail::cast_is_temporary_value_reference<int>::value) {
				static pybind11::detail::override_caster_t<int> caster;
				return pybind11::detail::cast_ref<int>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<int>(std::move(o));
		}
		return KdTreeSearch_ANN::find_closest_point(a0);
	}
	void find_closest_k_points(const class easy3d::Vec<3, float> & a0, int a1, class std::vector<int> & a2, class std::vector<float> & a3) const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::KdTreeSearch_ANN *>(this), "find_closest_k_points");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2, a3);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return KdTreeSearch_ANN::find_closest_k_points(a0, a1, a2, a3);
	}
	void find_closest_k_points(const class easy3d::Vec<3, float> & a0, int a1, class std::vector<int> & a2) const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::KdTreeSearch_ANN *>(this), "find_closest_k_points");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return KdTreeSearch_ANN::find_closest_k_points(a0, a1, a2);
	}
	void find_points_in_range(const class easy3d::Vec<3, float> & a0, float a1, class std::vector<int> & a2, class std::vector<float> & a3) const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::KdTreeSearch_ANN *>(this), "find_points_in_range");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2, a3);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return KdTreeSearch_ANN::find_points_in_range(a0, a1, a2, a3);
	}
	void find_points_in_range(const class easy3d::Vec<3, float> & a0, float a1, class std::vector<int> & a2) const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::KdTreeSearch_ANN *>(this), "find_points_in_range");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return KdTreeSearch_ANN::find_points_in_range(a0, a1, a2);
	}
};


void bind_easy3d_kdtree_kdtree_search_ann(pybind11::module_& m)
{
    { // easy3d::KdTreeSearch file:easy3d/kdtree/kdtree_search.h line:80
        pybind11::class_<easy3d::KdTreeSearch, std::shared_ptr<easy3d::KdTreeSearch>, PyCallBack_easy3d_KdTreeSearch> cl(m, "KdTreeSearch", "Base class for nearest neighbor search using KdTree.\n \n\n\n \n KdTreeSearch_ANN, KdTreeSearch_ETH, KdTreeSearch_FLANN, and KdTreeSearch_NanoFLANN\n\n \n Easy3D has a collection of KdTree implementations, including [ANN](http://www.cs.umd.edu/~mount/ANN/),\n ETH, [FLANN](https://github.com/mariusmuja/flann), and [NanoFLANN](https://github.com/jlblancoc/nanoflann) and\n tried to get the best performance of each implementation. Runtime tests (on Windows, using a Visual C++ 2008,\n release build) indicated that the ETH implementation has the best performance. Bellow is the statistic summary\n of the test on two point clouds.\n\n \n Build:  time for constructing the kd-tree.\n \n\n Single: time for querying closest vertex (for each point in the point cloud).\n \n\n KNN:    time for querying K(=16) closest vertex.\n \n\n Radius: time for querying closest vertex within a radius.\n\n Test 1: 362, 271 points (the Stanford bunny). radius = 0.001\n \n\n\n\n\n\n\n\n\n\n Test 2: 4, 116, 466 points (an noisy urban scan). radius = 0.03\n \n\n\n\n\n\n\n\n\n\n \n KdTreeSearch_FLANN and KdTreeSearch_NanoFLANN are thread-safe. Others seem not (not tested yet).");
        cl.def( pybind11::init<const class std::vector<class easy3d::Vec<3, float> > &>(), pybind11::arg("points") );

        cl.def(pybind11::init<PyCallBack_easy3d_KdTreeSearch const &>());
        cl.def("find_closest_point", (int (easy3d::KdTreeSearch::*)(const class easy3d::Vec<3, float> &, float &) const) &easy3d::KdTreeSearch::find_closest_point, "Queries the closest point for a given point.\n \n\n The query point.\n \n\n The squared distance between the query point and its closest neighbor.\n \n\n A  distance is returned by the second argument \n \n\n The index of the nearest neighbor found (the same as in the original point cloud).\n\nC++: easy3d::KdTreeSearch::find_closest_point(const class easy3d::Vec<3, float> &, float &) const --> int", pybind11::arg("p"), pybind11::arg("squared_distance"));
        cl.def("find_closest_point", (int (easy3d::KdTreeSearch::*)(const class easy3d::Vec<3, float> &) const) &easy3d::KdTreeSearch::find_closest_point, "Queries the closest point for a given point.\n \n\n The query point.\n \n\n The index of the nearest neighbor found (the same as in the original point cloud).\n\nC++: easy3d::KdTreeSearch::find_closest_point(const class easy3d::Vec<3, float> &) const --> int", pybind11::arg("p"));
        cl.def("find_closest_k_points", (void (easy3d::KdTreeSearch::*)(const class easy3d::Vec<3, float> &, int, class std::vector<int> &, class std::vector<float> &) const) &easy3d::KdTreeSearch::find_closest_k_points, "Queries the K nearest neighbors for a given point.\n \n\n The query point.\n \n\n The number of required neighbors.\n \n\n The indices of the neighbors found (the same as in the original point cloud).\n \n\n The squared distances between the query point and its K nearest neighbors.\n The values are stored in accordance with their indices.\n \n\n The  distances are returned by the argument \n         \n\nC++: easy3d::KdTreeSearch::find_closest_k_points(const class easy3d::Vec<3, float> &, int, class std::vector<int> &, class std::vector<float> &) const --> void", pybind11::arg("p"), pybind11::arg("k"), pybind11::arg("neighbors"), pybind11::arg("squared_distances"));
        cl.def("find_closest_k_points", (void (easy3d::KdTreeSearch::*)(const class easy3d::Vec<3, float> &, int, class std::vector<int> &) const) &easy3d::KdTreeSearch::find_closest_k_points, "Queries the K nearest neighbors for a given point.\n \n\n The query point.\n \n\n The number of required neighbors.\n \n\n The indices of the neighbors found (the same as in the original point cloud).\n\nC++: easy3d::KdTreeSearch::find_closest_k_points(const class easy3d::Vec<3, float> &, int, class std::vector<int> &) const --> void", pybind11::arg("p"), pybind11::arg("k"), pybind11::arg("neighbors"));
        cl.def("find_points_in_range", (void (easy3d::KdTreeSearch::*)(const class easy3d::Vec<3, float> &, float, class std::vector<int> &, class std::vector<float> &) const) &easy3d::KdTreeSearch::find_points_in_range, "Queries the nearest neighbors within a fixed range.\n \n\n The query point.\n \n\n The search range (which is required to be \n \n\n The indices of the neighbors found (the same as in the original point cloud).\n \n\n The squared distances between the query point and the neighbors found.\n The values are stored in accordance with their indices.\n \n\n The  distances are returned by the argument \n         \n\nC++: easy3d::KdTreeSearch::find_points_in_range(const class easy3d::Vec<3, float> &, float, class std::vector<int> &, class std::vector<float> &) const --> void", pybind11::arg("p"), pybind11::arg("squared_radius"), pybind11::arg("neighbors"), pybind11::arg("squared_distances"));
        cl.def("find_points_in_range", (void (easy3d::KdTreeSearch::*)(const class easy3d::Vec<3, float> &, float, class std::vector<int> &) const) &easy3d::KdTreeSearch::find_points_in_range, "Queries the nearest neighbors within a fixed range.\n \n\n The query point.\n \n\n The search range (which is required to be \n \n\n The indices of the neighbors found (the same as in the original point cloud).\n\nC++: easy3d::KdTreeSearch::find_points_in_range(const class easy3d::Vec<3, float> &, float, class std::vector<int> &) const --> void", pybind11::arg("p"), pybind11::arg("squared_radius"), pybind11::arg("neighbors"));
        cl.def("assign", (class easy3d::KdTreeSearch & (easy3d::KdTreeSearch::*)(const class easy3d::KdTreeSearch &)) &easy3d::KdTreeSearch::operator=, "C++: easy3d::KdTreeSearch::operator=(const class easy3d::KdTreeSearch &) --> class easy3d::KdTreeSearch &", pybind11::return_value_policy::automatic, pybind11::arg(""));
    }

	{ // easy3d::KdTreeSearch_ANN file:easy3d/kdtree/kdtree_search_ann.h line:42
		pybind11::class_<easy3d::KdTreeSearch_ANN, std::shared_ptr<easy3d::KdTreeSearch_ANN>, PyCallBack_easy3d_KdTreeSearch_ANN, easy3d::KdTreeSearch> cl(m, "KdTreeSearch_ANN", "KdTree implementation based on [ANN](http://www.cs.umd.edu/~mount/ANN/).\n \n\n\n \n KdTreeSearch_ETH, KdTreeSearch_FLANN, and KdTreeSearch_NanoFLANN.");
		cl.def( pybind11::init<const class easy3d::PointCloud *>(), pybind11::arg("cloud") );

		cl.def( pybind11::init<const class std::vector<class easy3d::Vec<3, float> > &>(), pybind11::arg("points") );

		cl.def( pybind11::init( [](PyCallBack_easy3d_KdTreeSearch_ANN const &o){ return new PyCallBack_easy3d_KdTreeSearch_ANN(o); } ) );
		cl.def( pybind11::init( [](easy3d::KdTreeSearch_ANN const &o){ return new easy3d::KdTreeSearch_ANN(o); } ) );

        // Liangliang: Python doesn’t have the concept of pass-by-reference as in C++. We must explicitly wrap such
        //      arguments to achieve the desired behavior.
//		cl.def("find_closest_point", (int (easy3d::KdTreeSearch_ANN::*)(const class easy3d::Vec<3, float> &, float &) const) &easy3d::KdTreeSearch_ANN::find_closest_point, "Queries the closest point for a given point.\n \n\n The query point.\n \n\n The squared distance between the query point and its closest neighbor.\n \n\n A  distance is returned by the second argument \n \n\n The index of the nearest neighbor found.\n\nC++: easy3d::KdTreeSearch_ANN::find_closest_point(const class easy3d::Vec<3, float> &, float &) const --> int", pybind11::arg("p"), pybind11::arg("squared_distance"));
//		cl.def("find_closest_point", (int (easy3d::KdTreeSearch_ANN::*)(const class easy3d::Vec<3, float> &) const) &easy3d::KdTreeSearch_ANN::find_closest_point, "Queries the closest point for a given point.\n \n\n The query point.\n \n\n The index of the nearest neighbor found.\n\nC++: easy3d::KdTreeSearch_ANN::find_closest_point(const class easy3d::Vec<3, float> &) const --> int", pybind11::arg("p"));
        cl.def("find_closest_point",
               [](const easy3d::KdTreeSearch_ANN &self, const easy3d::vec3 &p) {
                   return self.find_closest_point(p);
               },
               "Queries the closest point for a given point (without returning squared distance)",
               pybind11::arg("p"));

        cl.def("find_closest_point_with_squared_distance",
               [](const easy3d::KdTreeSearch_ANN &self, const easy3d::vec3 &p) {
                   float squared_distance = 0.0f;
                   int result = self.find_closest_point(p, squared_distance);
                   return std::make_pair(result, squared_distance);
               },
               "Queries the closest point for a given point (returns squared distance as well)",
               pybind11::arg("p"));

		cl.def("find_closest_k_points", (void (easy3d::KdTreeSearch_ANN::*)(const class easy3d::Vec<3, float> &, int, class std::vector<int> &, class std::vector<float> &) const) &easy3d::KdTreeSearch_ANN::find_closest_k_points, "Queries the K nearest neighbors for a given point.\n \n\n The query point.\n \n\n The number of required neighbors.\n \n\n The indices of the neighbors found.\n \n\n The squared distances between the query point and its K nearest neighbors.\n The values are stored in accordance with their indices.\n \n\n The  distances are returned by the argument \n         \n\nC++: easy3d::KdTreeSearch_ANN::find_closest_k_points(const class easy3d::Vec<3, float> &, int, class std::vector<int> &, class std::vector<float> &) const --> void", pybind11::arg("p"), pybind11::arg("k"), pybind11::arg("neighbors"), pybind11::arg("squared_distances"));
		cl.def("find_closest_k_points", (void (easy3d::KdTreeSearch_ANN::*)(const class easy3d::Vec<3, float> &, int, class std::vector<int> &) const) &easy3d::KdTreeSearch_ANN::find_closest_k_points, "Queries the K nearest neighbors for a given point.\n \n\n The query point.\n \n\n The number of required neighbors.\n \n\n The indices of the neighbors found.\n\nC++: easy3d::KdTreeSearch_ANN::find_closest_k_points(const class easy3d::Vec<3, float> &, int, class std::vector<int> &) const --> void", pybind11::arg("p"), pybind11::arg("k"), pybind11::arg("neighbors"));
		cl.def("set_k_for_radius_search", (void (easy3d::KdTreeSearch_ANN::*)(int)) &easy3d::KdTreeSearch_ANN::set_k_for_radius_search, "Specifies the K for fixed range search.\n \n\n ANN's annkFRSearch() needs to specify k. ANN's fixed range search is done in two steps.\n First, it computes the k nearest neighbors within the radius bound. Second, it returns the total number\n of points lying within the radius bound. It is permitted to set k = 0, in which case it only answers a range\n counting query.\n\nC++: easy3d::KdTreeSearch_ANN::set_k_for_radius_search(int) --> void", pybind11::arg("k"));
		cl.def("find_points_in_range", (void (easy3d::KdTreeSearch_ANN::*)(const class easy3d::Vec<3, float> &, float, class std::vector<int> &, class std::vector<float> &) const) &easy3d::KdTreeSearch_ANN::find_points_in_range, "Queries the nearest neighbors within a fixed range.\n \n\n The query point.\n \n\n The search range (which is required to be \n \n\n The indices of the neighbors found.\n \n\n The squared distances between the query point and the neighbors found.\n The values are stored in accordance with their indices.\n \n\n The  distances are returned by the argument \n         \n\nC++: easy3d::KdTreeSearch_ANN::find_points_in_range(const class easy3d::Vec<3, float> &, float, class std::vector<int> &, class std::vector<float> &) const --> void", pybind11::arg("p"), pybind11::arg("squared_radius"), pybind11::arg("neighbors"), pybind11::arg("squared_distances"));
		cl.def("find_points_in_range", (void (easy3d::KdTreeSearch_ANN::*)(const class easy3d::Vec<3, float> &, float, class std::vector<int> &) const) &easy3d::KdTreeSearch_ANN::find_points_in_range, "Queries the nearest neighbors within a fixed range.\n \n\n The query point.\n \n\n The search range (which is required to be \n \n\n The indices of the neighbors found.\n\nC++: easy3d::KdTreeSearch_ANN::find_points_in_range(const class easy3d::Vec<3, float> &, float, class std::vector<int> &) const --> void", pybind11::arg("p"), pybind11::arg("squared_radius"), pybind11::arg("neighbors"));
		cl.def("assign", (class easy3d::KdTreeSearch_ANN & (easy3d::KdTreeSearch_ANN::*)(const class easy3d::KdTreeSearch_ANN &)) &easy3d::KdTreeSearch_ANN::operator=, "C++: easy3d::KdTreeSearch_ANN::operator=(const class easy3d::KdTreeSearch_ANN &) --> class easy3d::KdTreeSearch_ANN &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}

}
