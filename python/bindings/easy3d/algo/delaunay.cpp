#include <easy3d/algo/delaunay.h>
#include <easy3d/algo/delaunay_2d.h>
#include <easy3d/algo/delaunay_3d.h>
#include <easy3d/core/vec.h>

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


// easy3d::Delaunay file:easy3d/algo/delaunay.h line:25
struct PyCallBack_easy3d_Delaunay : public easy3d::Delaunay {
	using easy3d::Delaunay::Delaunay;

	void set_vertices(unsigned int a0, const float * a1) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Delaunay *>(this), "set_vertices");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return Delaunay::set_vertices(a0, a1);
	}
	unsigned int nearest_vertex(const float * a0) const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Delaunay *>(this), "nearest_vertex");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
			if (pybind11::detail::cast_is_temporary_value_reference<unsigned int>::value) {
				static pybind11::detail::override_caster_t<unsigned int> caster;
				return pybind11::detail::cast_ref<unsigned int>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<unsigned int>(std::move(o));
		}
		return Delaunay::nearest_vertex(a0);
	}
	void get_neighbors(unsigned int a0, class std::vector<unsigned int> & a1) const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Delaunay *>(this), "get_neighbors");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return Delaunay::get_neighbors(a0, a1);
	}
	void set_arrays(unsigned int a0, const int * a1, const int * a2) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Delaunay *>(this), "set_arrays");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return Delaunay::set_arrays(a0, a1, a2);
	}
};

// easy3d::Delaunay2 file:easy3d/algo/delaunay_2d.h line:26
struct PyCallBack_easy3d_Delaunay2 : public easy3d::Delaunay2 {
	using easy3d::Delaunay2::Delaunay2;

	void set_vertices(unsigned int a0, const float * a1) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Delaunay2 *>(this), "set_vertices");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return Delaunay2::set_vertices(a0, a1);
	}
	unsigned int nearest_vertex(const float * a0) const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Delaunay2 *>(this), "nearest_vertex");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
			if (pybind11::detail::cast_is_temporary_value_reference<unsigned int>::value) {
				static pybind11::detail::override_caster_t<unsigned int> caster;
				return pybind11::detail::cast_ref<unsigned int>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<unsigned int>(std::move(o));
		}
		return Delaunay2::nearest_vertex(a0);
	}
	void get_neighbors(unsigned int a0, class std::vector<unsigned int> & a1) const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Delaunay2 *>(this), "get_neighbors");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return Delaunay::get_neighbors(a0, a1);
	}
	void set_arrays(unsigned int a0, const int * a1, const int * a2) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Delaunay2 *>(this), "set_arrays");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return Delaunay::set_arrays(a0, a1, a2);
	}
};

// easy3d::Delaunay3 file:easy3d/algo/delaunay_3d.h line:31
struct PyCallBack_easy3d_Delaunay3 : public easy3d::Delaunay3 {
	using easy3d::Delaunay3::Delaunay3;

	void set_vertices(unsigned int a0, const float * a1) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Delaunay3 *>(this), "set_vertices");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return Delaunay3::set_vertices(a0, a1);
	}
	unsigned int nearest_vertex(const float * a0) const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Delaunay3 *>(this), "nearest_vertex");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
			if (pybind11::detail::cast_is_temporary_value_reference<unsigned int>::value) {
				static pybind11::detail::override_caster_t<unsigned int> caster;
				return pybind11::detail::cast_ref<unsigned int>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<unsigned int>(std::move(o));
		}
		return Delaunay3::nearest_vertex(a0);
	}
	void get_neighbors(unsigned int a0, class std::vector<unsigned int> & a1) const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Delaunay3 *>(this), "get_neighbors");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return Delaunay::get_neighbors(a0, a1);
	}
	void set_arrays(unsigned int a0, const int * a1, const int * a2) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Delaunay3 *>(this), "set_arrays");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return Delaunay::set_arrays(a0, a1, a2);
	}
};

void bind_easy3d_algo_delaunay(pybind11::module_& m)
{
	{ // easy3d::Delaunay file:easy3d/algo/delaunay.h line:25
		pybind11::class_<easy3d::Delaunay, std::shared_ptr<easy3d::Delaunay>, PyCallBack_easy3d_Delaunay> cl(m, "Delaunay", "Base class for Delaunay triangulation.\n \n\n\n \n Delaunay2D, Delaunay3D.");
		cl.def( pybind11::init<unsigned int>(), pybind11::arg("dimension") );

		cl.def( pybind11::init( [](PyCallBack_easy3d_Delaunay const &o){ return new PyCallBack_easy3d_Delaunay(o); } ) );
		cl.def( pybind11::init( [](easy3d::Delaunay const &o){ return new easy3d::Delaunay(o); } ) );
		cl.def("dimension", (unsigned int (easy3d::Delaunay::*)() const) &easy3d::Delaunay::dimension, "Returns the dimension.\n\nC++: easy3d::Delaunay::dimension() const --> unsigned int");
		cl.def("cell_size", (unsigned int (easy3d::Delaunay::*)() const) &easy3d::Delaunay::cell_size, "Returns the size of the cell.\n\nC++: easy3d::Delaunay::cell_size() const --> unsigned int");
		cl.def("set_vertices", (void (easy3d::Delaunay::*)(unsigned int, const float *)) &easy3d::Delaunay::set_vertices, "Sets the vertices.\n\nC++: easy3d::Delaunay::set_vertices(unsigned int, const float *) --> void", pybind11::arg("nb_vertices"), pybind11::arg("vertices"));
		cl.def("vertices_ptr", (const float * (easy3d::Delaunay::*)() const) &easy3d::Delaunay::vertices_ptr, "Returns the pointer to the vertices.\n\nC++: easy3d::Delaunay::vertices_ptr() const --> const float *", pybind11::return_value_policy::automatic);
		cl.def("vertex_ptr", (const float * (easy3d::Delaunay::*)(unsigned int) const) &easy3d::Delaunay::vertex_ptr, "Returns the pointer to the vertex of index \n\nC++: easy3d::Delaunay::vertex_ptr(unsigned int) const --> const float *", pybind11::return_value_policy::automatic, pybind11::arg("i"));
		cl.def("nb_vertices", (unsigned int (easy3d::Delaunay::*)() const) &easy3d::Delaunay::nb_vertices, "Returns the number of vertices.\n\nC++: easy3d::Delaunay::nb_vertices() const --> unsigned int");
		cl.def("nb_cells", (unsigned int (easy3d::Delaunay::*)() const) &easy3d::Delaunay::nb_cells, "Returns the number of cells.\n\nC++: easy3d::Delaunay::nb_cells() const --> unsigned int");
		cl.def("cell_to_v", (const int * (easy3d::Delaunay::*)() const) &easy3d::Delaunay::cell_to_v, "C++: easy3d::Delaunay::cell_to_v() const --> const int *", pybind11::return_value_policy::automatic);
		cl.def("cell_to_cell", (const int * (easy3d::Delaunay::*)() const) &easy3d::Delaunay::cell_to_cell, "C++: easy3d::Delaunay::cell_to_cell() const --> const int *", pybind11::return_value_policy::automatic);
		cl.def("nearest_vertex", (unsigned int (easy3d::Delaunay::*)(const float *) const) &easy3d::Delaunay::nearest_vertex, "C++: easy3d::Delaunay::nearest_vertex(const float *) const --> unsigned int", pybind11::arg("p"));
		cl.def("cell_vertex", (int (easy3d::Delaunay::*)(unsigned int, unsigned int) const) &easy3d::Delaunay::cell_vertex, "Returns the index of the  vertex in the  cell.\n\nC++: easy3d::Delaunay::cell_vertex(unsigned int, unsigned int) const --> int", pybind11::arg("c"), pybind11::arg("lv"));
		cl.def("cell_adjacent", (int (easy3d::Delaunay::*)(unsigned int, unsigned int) const) &easy3d::Delaunay::cell_adjacent, "C++: easy3d::Delaunay::cell_adjacent(unsigned int, unsigned int) const --> int", pybind11::arg("c"), pybind11::arg("lf"));
		cl.def("vertex_cell", (int (easy3d::Delaunay::*)(unsigned int) const) &easy3d::Delaunay::vertex_cell, "C++: easy3d::Delaunay::vertex_cell(unsigned int) const --> int", pybind11::arg("v"));
		cl.def("index", (unsigned int (easy3d::Delaunay::*)(unsigned int, int) const) &easy3d::Delaunay::index, "C++: easy3d::Delaunay::index(unsigned int, int) const --> unsigned int", pybind11::arg("c"), pybind11::arg("v"));
		cl.def("adjacent_index", (unsigned int (easy3d::Delaunay::*)(unsigned int, unsigned int) const) &easy3d::Delaunay::adjacent_index, "C++: easy3d::Delaunay::adjacent_index(unsigned int, unsigned int) const --> unsigned int", pybind11::arg("c1"), pybind11::arg("c2"));
		cl.def("next_around_vertex", (unsigned int (easy3d::Delaunay::*)(unsigned int, unsigned int) const) &easy3d::Delaunay::next_around_vertex, "C++: easy3d::Delaunay::next_around_vertex(unsigned int, unsigned int) const --> unsigned int", pybind11::arg("c"), pybind11::arg("lv"));
		cl.def("get_neighbors", (void (easy3d::Delaunay::*)(unsigned int, class std::vector<unsigned int> &) const) &easy3d::Delaunay::get_neighbors, "Retrieves the one-ring neighbors of vertex v.\n\nC++: easy3d::Delaunay::get_neighbors(unsigned int, class std::vector<unsigned int> &) const --> void", pybind11::arg("v"), pybind11::arg("neighbors"));
		cl.def("check_duplicate_vertices", (bool (easy3d::Delaunay::*)()) &easy3d::Delaunay::check_duplicate_vertices, "Checks for duplicate vertices in stored neighbor lists.\n Returns true if there where some duplicate vertices.\n\nC++: easy3d::Delaunay::check_duplicate_vertices() --> bool");
		cl.def("assign", (class easy3d::Delaunay & (easy3d::Delaunay::*)(const class easy3d::Delaunay &)) &easy3d::Delaunay::operator=, "C++: easy3d::Delaunay::operator=(const class easy3d::Delaunay &) --> class easy3d::Delaunay &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}
	{ // easy3d::Delaunay2 file:easy3d/algo/delaunay_2d.h line:26
		pybind11::class_<easy3d::Delaunay2, std::shared_ptr<easy3d::Delaunay2>, PyCallBack_easy3d_Delaunay2, easy3d::Delaunay> cl(m, "Delaunay2", "2D Delaunay triangulation, using Jonathan Richard Shewchuk's \"triangle\" implementation.\n \n\n\n \n Delaunay, Delaunay3.");
		cl.def( pybind11::init( [](){ return new easy3d::Delaunay2(); }, [](){ return new PyCallBack_easy3d_Delaunay2(); } ) );
		cl.def( pybind11::init( [](PyCallBack_easy3d_Delaunay2 const &o){ return new PyCallBack_easy3d_Delaunay2(o); } ) );
		cl.def( pybind11::init( [](easy3d::Delaunay2 const &o){ return new easy3d::Delaunay2(o); } ) );
		cl.def("set_vertices", (void (easy3d::Delaunay2::*)(unsigned int, const float *)) &easy3d::Delaunay2::set_vertices, "Sets the vertices from an array of floating point numbers in which each consecutive number pair\n denotes a 2D point.\n\nC++: easy3d::Delaunay2::set_vertices(unsigned int, const float *) --> void", pybind11::arg("nb_vertices"), pybind11::arg("pts"));
		cl.def("set_vertices", (void (easy3d::Delaunay2::*)(const class std::vector<class easy3d::Vec<2, float> > &)) &easy3d::Delaunay2::set_vertices, "Sets the vertices from an array of 2D points.\n\nC++: easy3d::Delaunay2::set_vertices(const class std::vector<class easy3d::Vec<2, float> > &) --> void", pybind11::arg("vertices"));
		cl.def("nearest_vertex", (unsigned int (easy3d::Delaunay2::*)(const float *) const) &easy3d::Delaunay2::nearest_vertex, "C++: easy3d::Delaunay2::nearest_vertex(const float *) const --> unsigned int", pybind11::arg("p"));
		cl.def("nearest_vertex", (unsigned int (easy3d::Delaunay2::*)(const class easy3d::Vec<2, float> &) const) &easy3d::Delaunay2::nearest_vertex, "C++: easy3d::Delaunay2::nearest_vertex(const class easy3d::Vec<2, float> &) const --> unsigned int", pybind11::arg("p"));
		cl.def("vertex", (const class easy3d::Vec<2, float> & (easy3d::Delaunay2::*)(unsigned int) const) &easy3d::Delaunay2::vertex, "C++: easy3d::Delaunay2::vertex(unsigned int) const --> const class easy3d::Vec<2, float> &", pybind11::return_value_policy::automatic, pybind11::arg("i"));
		cl.def("tri_to_v", (const int * (easy3d::Delaunay2::*)() const) &easy3d::Delaunay2::tri_to_v, "C++: easy3d::Delaunay2::tri_to_v() const --> const int *", pybind11::return_value_policy::automatic);
		cl.def("tri_to_tri", (const int * (easy3d::Delaunay2::*)() const) &easy3d::Delaunay2::tri_to_tri, "C++: easy3d::Delaunay2::tri_to_tri() const --> const int *", pybind11::return_value_policy::automatic);
		cl.def("vertex_tri", (int (easy3d::Delaunay2::*)(int) const) &easy3d::Delaunay2::vertex_tri, "C++: easy3d::Delaunay2::vertex_tri(int) const --> int", pybind11::arg("v"));
		cl.def("nb_triangles", (unsigned int (easy3d::Delaunay2::*)() const) &easy3d::Delaunay2::nb_triangles, "Returns the number of triangles.\n\nC++: easy3d::Delaunay2::nb_triangles() const --> unsigned int");
		cl.def("tri_vertex", (int (easy3d::Delaunay2::*)(unsigned int, unsigned int) const) &easy3d::Delaunay2::tri_vertex, "Returns the index of the  vertex in the  triangle.\n\nC++: easy3d::Delaunay2::tri_vertex(unsigned int, unsigned int) const --> int", pybind11::arg("t"), pybind11::arg("lv"));
		cl.def("tri_adjacent", (int (easy3d::Delaunay2::*)(unsigned int, unsigned int) const) &easy3d::Delaunay2::tri_adjacent, "C++: easy3d::Delaunay2::tri_adjacent(unsigned int, unsigned int) const --> int", pybind11::arg("t"), pybind11::arg("le"));
		cl.def("assign", (class easy3d::Delaunay2 & (easy3d::Delaunay2::*)(const class easy3d::Delaunay2 &)) &easy3d::Delaunay2::operator=, "C++: easy3d::Delaunay2::operator=(const class easy3d::Delaunay2 &) --> class easy3d::Delaunay2 &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}
	{ // easy3d::Delaunay3 file:easy3d/algo/delaunay_3d.h line:31
		pybind11::class_<easy3d::Delaunay3, std::shared_ptr<easy3d::Delaunay3>, PyCallBack_easy3d_Delaunay3, easy3d::Delaunay> cl(m, "Delaunay3", "3D Delaunay triangulation, using Hang Si's tetgen.\n \n\n\n \n Delaunay, Delaunay2.");
		cl.def( pybind11::init( [](){ return new easy3d::Delaunay3(); }, [](){ return new PyCallBack_easy3d_Delaunay3(); } ) );
		cl.def( pybind11::init( [](PyCallBack_easy3d_Delaunay3 const &o){ return new PyCallBack_easy3d_Delaunay3(o); } ) );
		cl.def( pybind11::init( [](easy3d::Delaunay3 const &o){ return new easy3d::Delaunay3(o); } ) );
		cl.def("set_vertices", (void (easy3d::Delaunay3::*)(unsigned int, const float *)) &easy3d::Delaunay3::set_vertices, "Sets the vertices from an array of floating point numbers in which each consecutive number triple\n denotes a 3D point.\n\nC++: easy3d::Delaunay3::set_vertices(unsigned int, const float *) --> void", pybind11::arg("nb_vertices"), pybind11::arg("vertices"));
		cl.def("set_vertices", (void (easy3d::Delaunay3::*)(const class std::vector<class easy3d::Vec<3, float> > &)) &easy3d::Delaunay3::set_vertices, "Sets the vertices from an array of 3D points.\n\nC++: easy3d::Delaunay3::set_vertices(const class std::vector<class easy3d::Vec<3, float> > &) --> void", pybind11::arg("vertices"));
		cl.def("nb_tets", (unsigned int (easy3d::Delaunay3::*)() const) &easy3d::Delaunay3::nb_tets, "Returns the number of tetrahedra.\n\nC++: easy3d::Delaunay3::nb_tets() const --> unsigned int");
		cl.def("tet_to_v", (const int * (easy3d::Delaunay3::*)() const) &easy3d::Delaunay3::tet_to_v, "C++: easy3d::Delaunay3::tet_to_v() const --> const int *", pybind11::return_value_policy::automatic);
		cl.def("tet_to_tet", (const int * (easy3d::Delaunay3::*)() const) &easy3d::Delaunay3::tet_to_tet, "C++: easy3d::Delaunay3::tet_to_tet() const --> const int *", pybind11::return_value_policy::automatic);
		cl.def("vertex_tet", (int (easy3d::Delaunay3::*)(int) const) &easy3d::Delaunay3::vertex_tet, "C++: easy3d::Delaunay3::vertex_tet(int) const --> int", pybind11::arg("v"));
		cl.def("nearest_vertex", (unsigned int (easy3d::Delaunay3::*)(const float *) const) &easy3d::Delaunay3::nearest_vertex, "C++: easy3d::Delaunay3::nearest_vertex(const float *) const --> unsigned int", pybind11::arg("p"));
		cl.def("nearest_vertex", (unsigned int (easy3d::Delaunay3::*)(const class easy3d::Vec<3, float> &) const) &easy3d::Delaunay3::nearest_vertex, "C++: easy3d::Delaunay3::nearest_vertex(const class easy3d::Vec<3, float> &) const --> unsigned int", pybind11::arg("p"));
		cl.def("vertex", (const class easy3d::Vec<3, float> & (easy3d::Delaunay3::*)(unsigned int) const) &easy3d::Delaunay3::vertex, "C++: easy3d::Delaunay3::vertex(unsigned int) const --> const class easy3d::Vec<3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("i"));
		cl.def("tet_vertex", (int (easy3d::Delaunay3::*)(unsigned int, unsigned int) const) &easy3d::Delaunay3::tet_vertex, "Returns the index of the  vertex in the  tetrahedron.\n\nC++: easy3d::Delaunay3::tet_vertex(unsigned int, unsigned int) const --> int", pybind11::arg("t"), pybind11::arg("lv"));
		cl.def("tet_adjacent", (int (easy3d::Delaunay3::*)(unsigned int, unsigned int) const) &easy3d::Delaunay3::tet_adjacent, "C++: easy3d::Delaunay3::tet_adjacent(unsigned int, unsigned int) const --> int", pybind11::arg("t"), pybind11::arg("lf"));
		cl.def("tet_facet_vertex", (int (easy3d::Delaunay3::*)(unsigned int, unsigned int, unsigned int) const) &easy3d::Delaunay3::tet_facet_vertex, "C++: easy3d::Delaunay3::tet_facet_vertex(unsigned int, unsigned int, unsigned int) const --> int", pybind11::arg("t"), pybind11::arg("lf"), pybind11::arg("lv"));
		cl.def("next_around_halfedge", (int (easy3d::Delaunay3::*)(int, unsigned int, unsigned int) const) &easy3d::Delaunay3::next_around_halfedge, "C++: easy3d::Delaunay3::next_around_halfedge(int, unsigned int, unsigned int) const --> int", pybind11::arg("t"), pybind11::arg("lv1"), pybind11::arg("lv2"));
		cl.def("prev_around_halfedge", (int (easy3d::Delaunay3::*)(int, unsigned int, unsigned int) const) &easy3d::Delaunay3::prev_around_halfedge, "C++: easy3d::Delaunay3::prev_around_halfedge(int, unsigned int, unsigned int) const --> int", pybind11::arg("t"), pybind11::arg("lv1"), pybind11::arg("lv2"));
		cl.def("facet_normal", (class easy3d::Vec<3, float> (easy3d::Delaunay3::*)(unsigned int, unsigned int) const) &easy3d::Delaunay3::facet_normal, "C++: easy3d::Delaunay3::facet_normal(unsigned int, unsigned int) const --> class easy3d::Vec<3, float>", pybind11::arg("t"), pybind11::arg("f"));
		cl.def("tet_circumcenter", (class easy3d::Vec<3, float> (easy3d::Delaunay3::*)(unsigned int) const) &easy3d::Delaunay3::tet_circumcenter, "C++: easy3d::Delaunay3::tet_circumcenter(unsigned int) const --> class easy3d::Vec<3, float>", pybind11::arg("t"));
		cl.def("get_voronoi_cell", [](easy3d::Delaunay3 const &o, unsigned int const & a0, class easy3d::VoronoiCell3d & a1) -> void { return o.get_voronoi_cell(a0, a1); }, "", pybind11::arg("v"), pybind11::arg("cell"));
		cl.def("get_voronoi_cell", (void (easy3d::Delaunay3::*)(unsigned int, class easy3d::VoronoiCell3d &, bool) const) &easy3d::Delaunay3::get_voronoi_cell, "Returns the Voronoi cell associated with vertex v.\n\nC++: easy3d::Delaunay3::get_voronoi_cell(unsigned int, class easy3d::VoronoiCell3d &, bool) const --> void", pybind11::arg("v"), pybind11::arg("cell"), pybind11::arg("geometry"));
		cl.def("assign", (class easy3d::Delaunay3 & (easy3d::Delaunay3::*)(const class easy3d::Delaunay3 &)) &easy3d::Delaunay3::operator=, "C++: easy3d::Delaunay3::operator=(const class easy3d::Delaunay3 &) --> class easy3d::Delaunay3 &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}
	{ // easy3d::VoronoiCell3d file:easy3d/algo/delaunay_3d.h line:167
		pybind11::class_<easy3d::VoronoiCell3d, std::shared_ptr<easy3d::VoronoiCell3d>> cl(m, "VoronoiCell3d", "A data structure for 3D Voronoi cells\n \n\n\n \n A 3D Voronoi cell stores the dual facets in a Compressed Row Storage array.\n - Each facet knows the bisector it is on, and the list of vertices/edges.\n    - Each vertex knows the tet it is dual to.\n    - Each edge knows the other bisector it is on (an edge is defined as the\n intersection between the facet bisector and the edge bisector).");
		cl.def( pybind11::init( [](){ return new easy3d::VoronoiCell3d(); } ) );
		cl.def( pybind11::init( [](easy3d::VoronoiCell3d const &o){ return new easy3d::VoronoiCell3d(o); } ) );
		cl.def("clear", (void (easy3d::VoronoiCell3d::*)()) &easy3d::VoronoiCell3d::clear, "C++: easy3d::VoronoiCell3d::clear() --> void");
		cl.def("nb_facets", (unsigned int (easy3d::VoronoiCell3d::*)() const) &easy3d::VoronoiCell3d::nb_facets, "C++: easy3d::VoronoiCell3d::nb_facets() const --> unsigned int");
		cl.def("facet_begin", (unsigned int (easy3d::VoronoiCell3d::*)(unsigned int) const) &easy3d::VoronoiCell3d::facet_begin, "C++: easy3d::VoronoiCell3d::facet_begin(unsigned int) const --> unsigned int", pybind11::arg("f"));
		cl.def("facet_end", (unsigned int (easy3d::VoronoiCell3d::*)(unsigned int) const) &easy3d::VoronoiCell3d::facet_end, "C++: easy3d::VoronoiCell3d::facet_end(unsigned int) const --> unsigned int", pybind11::arg("f"));
		cl.def("nb_vertices", (unsigned int (easy3d::VoronoiCell3d::*)(unsigned int) const) &easy3d::VoronoiCell3d::nb_vertices, "C++: easy3d::VoronoiCell3d::nb_vertices(unsigned int) const --> unsigned int", pybind11::arg("f"));
		cl.def("next_around_facet", (unsigned int (easy3d::VoronoiCell3d::*)(unsigned int, unsigned int) const) &easy3d::VoronoiCell3d::next_around_facet, "C++: easy3d::VoronoiCell3d::next_around_facet(unsigned int, unsigned int) const --> unsigned int", pybind11::arg("f"), pybind11::arg("i"));
		cl.def("prev_around_facet", (unsigned int (easy3d::VoronoiCell3d::*)(unsigned int, unsigned int) const) &easy3d::VoronoiCell3d::prev_around_facet, "C++: easy3d::VoronoiCell3d::prev_around_facet(unsigned int, unsigned int) const --> unsigned int", pybind11::arg("f"), pybind11::arg("i"));
		cl.def("facet_bisector", (unsigned int (easy3d::VoronoiCell3d::*)(unsigned int) const) &easy3d::VoronoiCell3d::facet_bisector, "- This VoronoiCell3d is the dual of vertex v1,\n - Facet f lies on the bisector plane of [v1,v2],\n - facet_bisector(f) returns v2.\n\nC++: easy3d::VoronoiCell3d::facet_bisector(unsigned int) const --> unsigned int", pybind11::arg("f"));
		cl.def("edge_bisector", (int (easy3d::VoronoiCell3d::*)(unsigned int) const) &easy3d::VoronoiCell3d::edge_bisector, "-The vertices of facet(f) are denoted by indices i in the range\n   [facet_begin(f) ... facet_end(f) - 1].\n - vertex i is the dual of a tetrahedron t incident to the edge [v1,v2]\n - edge_bisector(i) returns the vertex of t oriented clockwise\n    relative to the edge [v1,v2].\n - Note: an infinite facet has two vertices i1 and i2 that are\n     dual to infinite tetrahedra.\n   Supposing the proper ordering of i1 and i2, edge_bisector(i1)\n   returns the vertex of the boundary facet that is incident to [v1,v2],\n   and edge_bisector(i2) returns -1 (infinite vertex). (the other vertex\n   of the other boundary facet that is incident to [v1,v2] is encountered\n   when traversing the dual vertex/tetrahedron that precedes v2).\n\nC++: easy3d::VoronoiCell3d::edge_bisector(unsigned int) const --> int", pybind11::arg("i"));
		cl.def("vertex", (const class easy3d::Vec<3, float> & (easy3d::VoronoiCell3d::*)(unsigned int) const) &easy3d::VoronoiCell3d::vertex, "- For finite dual vertices, returns the Voronoi vertex.\n - For dual vertices at infinity, returns the direction\n    (normal to primal facet on the boundary).\n\nC++: easy3d::VoronoiCell3d::vertex(unsigned int) const --> const class easy3d::Vec<3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("i"));
		cl.def("vertex_is_infinite", (bool (easy3d::VoronoiCell3d::*)(unsigned int) const) &easy3d::VoronoiCell3d::vertex_is_infinite, "C++: easy3d::VoronoiCell3d::vertex_is_infinite(unsigned int) const --> bool", pybind11::arg("i"));
		cl.def("begin_facet", (void (easy3d::VoronoiCell3d::*)(unsigned int)) &easy3d::VoronoiCell3d::begin_facet, "C++: easy3d::VoronoiCell3d::begin_facet(unsigned int) --> void", pybind11::arg("f_bisector"));
		cl.def("add_to_facet", (void (easy3d::VoronoiCell3d::*)(int, const class easy3d::Vec<3, float> &, bool)) &easy3d::VoronoiCell3d::add_to_facet, "C++: easy3d::VoronoiCell3d::add_to_facet(int, const class easy3d::Vec<3, float> &, bool) --> void", pybind11::arg("e_bisector"), pybind11::arg("v"), pybind11::arg("infinite"));
		cl.def("add_to_facet", (void (easy3d::VoronoiCell3d::*)(int, bool)) &easy3d::VoronoiCell3d::add_to_facet, "C++: easy3d::VoronoiCell3d::add_to_facet(int, bool) --> void", pybind11::arg("e_bisector"), pybind11::arg("infinite"));
		cl.def("end_facet", (void (easy3d::VoronoiCell3d::*)()) &easy3d::VoronoiCell3d::end_facet, "C++: easy3d::VoronoiCell3d::end_facet() --> void");
		cl.def("find_facet", (unsigned int (easy3d::VoronoiCell3d::*)(unsigned int)) &easy3d::VoronoiCell3d::find_facet, "C++: easy3d::VoronoiCell3d::find_facet(unsigned int) --> unsigned int", pybind11::arg("bisector"));
		cl.def("assign", (class easy3d::VoronoiCell3d & (easy3d::VoronoiCell3d::*)(const class easy3d::VoronoiCell3d &)) &easy3d::VoronoiCell3d::operator=, "C++: easy3d::VoronoiCell3d::operator=(const class easy3d::VoronoiCell3d &) --> class easy3d::VoronoiCell3d &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}
}
