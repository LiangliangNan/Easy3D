#include <easy3d/core/line.h>
#include <easy3d/core/mat.h>
#include <easy3d/core/quat.h>
#include <easy3d/core/vec.h>
#include <easy3d/gui/picker.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/frame.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/state.h>
#include <easy3d/renderer/texture.h>
#include <memory>
#include <sstream> // __str__
#include <string>
#include <string_view>
#include <utility>
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

void bind_easy3d_gui_picker(pybind11::module_& m)
{
	{ // easy3d::Picker file:easy3d/gui/picker.h line:43
		pybind11::class_<easy3d::Picker, std::shared_ptr<easy3d::Picker>> cl(m, "Picker", "Base class for picking mechanism.\n \n\n\n \n ModelPicker, PointCloudPicker, SurfaceMeshPicker");
		cl.def( pybind11::init<const class easy3d::Camera *>(), pybind11::arg("cam") );

		cl.def( pybind11::init( [](easy3d::Picker const &o){ return new easy3d::Picker(o); } ) );
		cl.def("camera", (const class easy3d::Camera * (easy3d::Picker::*)() const) &easy3d::Picker::camera, "Returns the pointer of the camera.\n\nC++: easy3d::Picker::camera() const --> const class easy3d::Camera *", pybind11::return_value_policy::automatic);
		cl.def("picking_line", (class easy3d::GenericLine<3, float> (easy3d::Picker::*)(int, int) const) &easy3d::Picker::picking_line, "Construct a picking line.\n \n\n The cursor x-coordinate, relative to the left edge of the content area.\n \n\n The cursor y-coordinate, relative to the top edge of the content area.\n \n\n The screen point is expressed in the screen coordinate system with an origin in the upper left\n            corner. So it doesn't necessarily correspond to a pixel on High DPI devices, e.g. a Mac with\n            a Retina display.\n\nC++: easy3d::Picker::picking_line(int, int) const --> class easy3d::GenericLine<3, float>", pybind11::arg("x"), pybind11::arg("y"));
		cl.def("picking_dir", (class easy3d::Vec<3, float> (easy3d::Picker::*)(int, int) const) &easy3d::Picker::picking_dir, "The picking direction, pointing inside the screen.\n \n\n The cursor x-coordinate, relative to the left edge of the content area.\n \n\n The cursor y-coordinate, relative to the top edge of the content area.\n \n\n The screen point is expressed in the screen coordinate system with an origin in the upper left\n            corner. So it doesn't necessarily correspond to a pixel on High DPI devices, e.g. a Mac with\n            a Retina display.\n\nC++: easy3d::Picker::picking_dir(int, int) const --> class easy3d::Vec<3, float>", pybind11::arg("x"), pybind11::arg("y"));
		cl.def("project", (class easy3d::Vec<3, float> (easy3d::Picker::*)(const class easy3d::Vec<3, float> &) const) &easy3d::Picker::project, "Project a 3D point in the world coordinate system onto the 2D screen coordinate system.\n \n\n A 3D point in the world coordinate system.\n \n\n The x and y components of the returned value denote the projected screen point expressed in the\n         screen coordinate system, with (0, 0) being the upper left corner of the content area. The z\n         component ranges between 0.0 (near plane) and 1.0 (excluded, far plane).\n \n\n The screen point is expressed in the screen coordinate system with an origin in the upper left\n            corner. So it doesn't necessarily correspond to a pixel on High DPI devices, e.g. a Mac with\n            a Retina display.\n\nC++: easy3d::Picker::project(const class easy3d::Vec<3, float> &) const --> class easy3d::Vec<3, float>", pybind11::arg("p"));
		cl.def("unproject", (class easy3d::Vec<3, float> (easy3d::Picker::*)(int, int, float) const) &easy3d::Picker::unproject, "Compute the world coordinates of a point defined in the screen coordinate system.\n \n\n The cursor x-coordinate, relative to the left edge of the content area.\n \n\n The cursor y-coordinate, relative to the top edge of the content area.\n \n\n The depth value of the screen point, ranging between 0.0 and 1.0 (excluded).\n \n\n The world unprojected coordinates of the screen point.\n \n\n The screen point is expressed in the screen coordinate system with an origin in the upper left\n            corner. So it doesn't necessarily correspond to a pixel on High DPI devices, e.g. a Mac with\n            a Retina display.\n\nC++: easy3d::Picker::unproject(int, int, float) const --> class easy3d::Vec<3, float>", pybind11::arg("x"), pybind11::arg("y"), pybind11::arg("depth"));
		cl.def("screen_to_opengl", (void (easy3d::Picker::*)(int, int, int &, int &, int, int) const) &easy3d::Picker::screen_to_opengl, "Convert a point expressed in the screen coordinate system (with an origin in the upper left corner)\n into the OpenGL coordinate system (with an origin in the lower left corner). The high DPI scaling is also\n taken into consideration, so the result always corresponds to its image pixel.\n \n\n The x-coordinate, relative to the left edge of the content area.\n \n\n The y-coordinate, relative to the top edge of the content area.\n \n\n Returns the x component of the point in the OpenGL coordinate system.\n \n\n Returns the y component of the point in the OpenGL coordinate system.\n \n\n The width of the OpenGL viewport (may not be identical to the width of the screen in pixels).\n \n\n The width of the OpenGL viewport (may not be identical to the height of the screen in pixels).\n\nC++: easy3d::Picker::screen_to_opengl(int, int, int &, int &, int, int) const --> void", pybind11::arg("x"), pybind11::arg("y"), pybind11::arg("gl_x"), pybind11::arg("gl_y"), pybind11::arg("width"), pybind11::arg("height"));
		cl.def("assign", (class easy3d::Picker & (easy3d::Picker::*)(const class easy3d::Picker &)) &easy3d::Picker::operator=, "C++: easy3d::Picker::operator=(const class easy3d::Picker &) --> class easy3d::Picker &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}

}
