#include <easy3d/core/mat.h>
#include <easy3d/core/quat.h>
#include <easy3d/core/vec.h>
#include <easy3d/renderer/frustum.h>
#include <memory>
#include <sstream> // __str__
#include <vector>

#include <functional>
#include <pybind11/pybind11.h>
#include <string>
#include <pybind11/stl.h>


#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_renderer_frustum(pybind11::module_& m)
{
	{ // easy3d::Frustum file:easy3d/renderer/frustum.h line:42
		pybind11::class_<easy3d::Frustum, std::shared_ptr<easy3d::Frustum>> cl(m, "Frustum", "A Frustum description for perspective projection.\n \n\n\n     ");
		cl.def( pybind11::init<enum easy3d::Frustum::ProjectionType>(), pybind11::arg("type") );


		pybind11::enum_<easy3d::Frustum::ProjectionType>(cl, "ProjectionType", pybind11::arithmetic(), "")
			.value("PERSPECTIVE", easy3d::Frustum::PERSPECTIVE)
			.value("ORTHO", easy3d::Frustum::ORTHO)
			.export_values();

		cl.def("orient", (void (easy3d::Frustum::*)(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &)) &easy3d::Frustum::orient, "C++: easy3d::Frustum::orient(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &) --> void", pybind11::arg("pos"), pybind11::arg("at"), pybind11::arg("up"));
		cl.def("set_perspective", (void (easy3d::Frustum::*)(float, float, float, float)) &easy3d::Frustum::set_perspective, "C++: easy3d::Frustum::set_perspective(float, float, float, float) --> void", pybind11::arg("fovy"), pybind11::arg("aspect"), pybind11::arg("znear"), pybind11::arg("zfar"));
		cl.def("set_frustum", (void (easy3d::Frustum::*)(float, float, float, float)) &easy3d::Frustum::set_frustum, "C++: easy3d::Frustum::set_frustum(float, float, float, float) --> void", pybind11::arg("frustum_width"), pybind11::arg("frustum_height"), pybind11::arg("znear"), pybind11::arg("zfar"));
		cl.def("set_ortho", (void (easy3d::Frustum::*)(float, float, float, float, float, float)) &easy3d::Frustum::set_ortho, "C++: easy3d::Frustum::set_ortho(float, float, float, float, float, float) --> void", pybind11::arg("xmin"), pybind11::arg("xmax"), pybind11::arg("ymin"), pybind11::arg("ymax"), pybind11::arg("znear"), pybind11::arg("zfar"));
		cl.def("view_matrix", (class easy3d::Mat4<float> (easy3d::Frustum::*)() const) &easy3d::Frustum::view_matrix, "C++: easy3d::Frustum::view_matrix() const --> class easy3d::Mat4<float>");
		cl.def("projection_matrix", (class easy3d::Mat4<float> (easy3d::Frustum::*)() const) &easy3d::Frustum::projection_matrix, "C++: easy3d::Frustum::projection_matrix() const --> class easy3d::Mat4<float>");
		cl.def("near_center", (class easy3d::Vec<3, float> (easy3d::Frustum::*)() const) &easy3d::Frustum::near_center, "C++: easy3d::Frustum::near_center() const --> class easy3d::Vec<3, float>");
		cl.def("far_center", (class easy3d::Vec<3, float> (easy3d::Frustum::*)() const) &easy3d::Frustum::far_center, "C++: easy3d::Frustum::far_center() const --> class easy3d::Vec<3, float>");
		cl.def("near_width", (float (easy3d::Frustum::*)() const) &easy3d::Frustum::near_width, "C++: easy3d::Frustum::near_width() const --> float");
		cl.def("near_height", (float (easy3d::Frustum::*)() const) &easy3d::Frustum::near_height, "C++: easy3d::Frustum::near_height() const --> float");
		cl.def("far_width", (float (easy3d::Frustum::*)() const) &easy3d::Frustum::far_width, "C++: easy3d::Frustum::far_width() const --> float");
		cl.def("far_height", (float (easy3d::Frustum::*)() const) &easy3d::Frustum::far_height, "C++: easy3d::Frustum::far_height() const --> float");
		cl.def("position", (const class easy3d::Vec<3, float> & (easy3d::Frustum::*)() const) &easy3d::Frustum::position, "C++: easy3d::Frustum::position() const --> const class easy3d::Vec<3, float> &", pybind11::return_value_policy::automatic);
		cl.def("up_vector", (class easy3d::Vec<3, float> (easy3d::Frustum::*)() const) &easy3d::Frustum::up_vector, "C++: easy3d::Frustum::up_vector() const --> class easy3d::Vec<3, float>");
		cl.def("right_vector", (class easy3d::Vec<3, float> (easy3d::Frustum::*)() const) &easy3d::Frustum::right_vector, "C++: easy3d::Frustum::right_vector() const --> class easy3d::Vec<3, float>");
		cl.def("near_distance", (float (easy3d::Frustum::*)() const) &easy3d::Frustum::near_distance, "C++: easy3d::Frustum::near_distance() const --> float");
		cl.def("far_distance", (float (easy3d::Frustum::*)() const) &easy3d::Frustum::far_distance, "C++: easy3d::Frustum::far_distance() const --> float");
		cl.def("vertices", (class std::vector<class easy3d::Vec<3, float> > (easy3d::Frustum::*)() const) &easy3d::Frustum::vertices, "C++: easy3d::Frustum::vertices() const --> class std::vector<class easy3d::Vec<3, float> >");
	}
}
