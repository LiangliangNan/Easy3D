#include <easy3d/core/mat.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/property.h>
#include <easy3d/core/quat.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/vec.h>
#include <easy3d/gui/picker_point_cloud.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/frame.h>
#include <ios>
#include <locale>
#include <memory>
#include <ostream>
#include <sstream> // __str__
#include <streambuf>
#include <string>
#include <string_view>
#include <typeinfo>
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


void bind_easy3d_gui_picker_point_cloud(pybind11::module_& m)
{
	{ // easy3d::PointCloudPicker file:easy3d/gui/picker_point_cloud.h line:44
		pybind11::class_<easy3d::PointCloudPicker, std::shared_ptr<easy3d::PointCloudPicker>, easy3d::Picker> cl(m, "PointCloudPicker", "Implementation of picking points from a point cloud.\n \n\n\n \n Picker, ModelPicker, SurfaceMeshPicker");
		cl.def( pybind11::init<const class easy3d::Camera *>(), pybind11::arg("cam") );

		cl.def( pybind11::init( [](easy3d::PointCloudPicker const &o){ return new easy3d::PointCloudPicker(o); } ) );
		cl.def("resolution", (unsigned int (easy3d::PointCloudPicker::*)() const) &easy3d::PointCloudPicker::resolution, "Returns the picker resolution (in pixels).\n \n\n The picker resolution indicates the sensitivity of picking a point. It is used only for\n     the CPU implementation of picking a single point.\n\nC++: easy3d::PointCloudPicker::resolution() const --> unsigned int");
		cl.def("set_resolution", (void (easy3d::PointCloudPicker::*)(unsigned int)) &easy3d::PointCloudPicker::set_resolution, "Sets the picker resolution (in pixels).\n \n\n The picker resolution indicates the sensitivity of picking a point. It is used only for\n     the CPU implementation of picking a single point.\n\nC++: easy3d::PointCloudPicker::set_resolution(unsigned int) --> void", pybind11::arg("r"));
		cl.def("pick_vertex", (struct easy3d::PointCloud::Vertex (easy3d::PointCloudPicker::*)(class easy3d::PointCloud *, int, int)) &easy3d::PointCloudPicker::pick_vertex, "Pick vertex at a given screen location.\n \n\n y) The screen point.\n \n\n True to perform an inverse operation.\n \n\n The picked vertex.\n\nC++: easy3d::PointCloudPicker::pick_vertex(class easy3d::PointCloud *, int, int) --> struct easy3d::PointCloud::Vertex", pybind11::arg("model"), pybind11::arg("x"), pybind11::arg("y"));
		cl.def("assign", (class easy3d::PointCloudPicker & (easy3d::PointCloudPicker::*)(const class easy3d::PointCloudPicker &)) &easy3d::PointCloudPicker::operator=, "C++: easy3d::PointCloudPicker::operator=(const class easy3d::PointCloudPicker &) --> class easy3d::PointCloudPicker &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}

}
