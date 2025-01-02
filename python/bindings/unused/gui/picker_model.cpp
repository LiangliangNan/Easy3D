#include <easy3d/core/box.h>
#include <easy3d/core/mat.h>
#include <easy3d/core/model.h>
#include <easy3d/core/property.h>
#include <easy3d/core/quat.h>
#include <easy3d/core/vec.h>
#include <easy3d/gui/picker_model.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/frame.h>
#include <ios>
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


void bind_easy3d_gui_picker_model(pybind11::module_& m)
{
	{ // easy3d::ModelPicker file:easy3d/gui/picker_model.h line:48
		pybind11::class_<easy3d::ModelPicker, std::shared_ptr<easy3d::ModelPicker>, easy3d::Picker> cl(m, "ModelPicker", "Implementation of picking mechanism for set of models.\n \n\n\n \n Picker, PointCloudPicker, SurfaceMeshPicker");
		cl.def( pybind11::init<const class easy3d::Camera *>(), pybind11::arg("cam") );

		cl.def( pybind11::init( [](easy3d::ModelPicker const &o){ return new easy3d::ModelPicker(o); } ) );
		cl.def("pick", (class easy3d::Model * (easy3d::ModelPicker::*)(const class std::vector<class easy3d::Model *> &, int, int)) &easy3d::ModelPicker::pick, "Pick a model from a set of models given the cursor position in the screen coordinate system.\n \n\n The models\n \n\n The cursor x-coordinate, relative to the left edge of the content area.\n \n\n The cursor y-coordinate, relative to the top edge of the content area.\n \n\n The screen point is expressed in the screen coordinate system with an origin in the upper left\n            corner. So it doesn't necessarily correspond to a pixel on High DPI devices, e.g. a Mac with\n            a Retina display.\n \n\n The picked model.\n\nC++: easy3d::ModelPicker::pick(const class std::vector<class easy3d::Model *> &, int, int) --> class easy3d::Model *", pybind11::return_value_policy::automatic, pybind11::arg("models"), pybind11::arg("x"), pybind11::arg("y"));
		cl.def("assign", (class easy3d::ModelPicker & (easy3d::ModelPicker::*)(const class easy3d::ModelPicker &)) &easy3d::ModelPicker::operator=, "C++: easy3d::ModelPicker::operator=(const class easy3d::ModelPicker &) --> class easy3d::ModelPicker &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}

}
