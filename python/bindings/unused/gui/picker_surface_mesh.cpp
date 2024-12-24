#include <easy3d/core/mat.h>
#include <easy3d/core/quat.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/vec.h>
#include <easy3d/gui/picker_surface_mesh.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/frame.h>
#include <memory>
#include <ostream>
#include <sstream> // __str__
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

void bind_easy3d_gui_picker_surface_mesh(pybind11::module_& m)
{
	{ // easy3d::SurfaceMeshPicker file:easy3d/gui/picker_surface_mesh.h line:44
		pybind11::class_<easy3d::SurfaceMeshPicker, std::shared_ptr<easy3d::SurfaceMeshPicker>, easy3d::Picker> cl(m, "SurfaceMeshPicker", "Implementation of picking elements (i.e, vertices, faces, edges) from a surface mesh.\n \n\n\n \n Picker, ModelPicker, PointCloudPicker");
		cl.def( pybind11::init<const class easy3d::Camera *>(), pybind11::arg("cam") );

		cl.def( pybind11::init( [](easy3d::SurfaceMeshPicker const &o){ return new easy3d::SurfaceMeshPicker(o); } ) );
		cl.def("resolution", (unsigned int (easy3d::SurfaceMeshPicker::*)() const) &easy3d::SurfaceMeshPicker::resolution, "Returns the picker resolution (in pixels).\n \n\n The picker resolution indicates the sensitivity for picking a vertex or an edge.\n\nC++: easy3d::SurfaceMeshPicker::resolution() const --> unsigned int");
		cl.def("set_resolution", (void (easy3d::SurfaceMeshPicker::*)(unsigned int)) &easy3d::SurfaceMeshPicker::set_resolution, "Sets the picker resolution (in pixels).\n \n\n The picker resolution indicates the sensitivity for picking a vertex or an edge.\n\nC++: easy3d::SurfaceMeshPicker::set_resolution(unsigned int) --> void", pybind11::arg("r"));
		cl.def("pick_face", (struct easy3d::SurfaceMesh::Face (easy3d::SurfaceMeshPicker::*)(class easy3d::SurfaceMesh *, int, int)) &easy3d::SurfaceMeshPicker::pick_face, "Pick a face from a surface mesh given the cursor position.\n \n\n The cursor x-coordinate, relative to the left edge of the content area.\n \n\n The cursor y-coordinate, relative to the top edge of the content area.\n \n\n The screen point is expressed in the screen coordinate system with an origin in the upper left\n            corner. So it doesn't necessarily correspond to a pixel on High DPI devices, e.g. a Mac with\n            a Retina display.\n \n\n The picked face.\n\nC++: easy3d::SurfaceMeshPicker::pick_face(class easy3d::SurfaceMesh *, int, int) --> struct easy3d::SurfaceMesh::Face", pybind11::arg("model"), pybind11::arg("x"), pybind11::arg("y"));
		cl.def("pick_vertex", (struct easy3d::SurfaceMesh::Vertex (easy3d::SurfaceMeshPicker::*)(class easy3d::SurfaceMesh *, int, int)) &easy3d::SurfaceMeshPicker::pick_vertex, "Pick a vertex from a surface mesh given the cursor position.\n \n\n The cursor x-coordinate, relative to the left edge of the content area.\n \n\n The cursor y-coordinate, relative to the top edge of the content area.\n \n\n The screen point is expressed in the screen coordinate system with an origin in the upper left\n            corner. So it doesn't necessarily correspond to a pixel on High DPI devices, e.g. a Mac with\n            a Retina display.\n \n\n The picked vertex.\n\nC++: easy3d::SurfaceMeshPicker::pick_vertex(class easy3d::SurfaceMesh *, int, int) --> struct easy3d::SurfaceMesh::Vertex", pybind11::arg("model"), pybind11::arg("x"), pybind11::arg("y"));
		cl.def("pick_edge", (struct easy3d::SurfaceMesh::Halfedge (easy3d::SurfaceMeshPicker::*)(class easy3d::SurfaceMesh *, int, int)) &easy3d::SurfaceMeshPicker::pick_edge, "Pick an edge from a surface mesh given the cursor position.\n \n\n The cursor x-coordinate, relative to the left edge of the content area.\n \n\n The cursor y-coordinate, relative to the top edge of the content area.\n \n\n The screen point is expressed in the screen coordinate system with an origin in the upper left\n            corner. So it doesn't necessarily correspond to a pixel on High DPI devices, e.g. a Mac with\n            a Retina display.\n \n\n The picked halfedge.\n\nC++: easy3d::SurfaceMeshPicker::pick_edge(class easy3d::SurfaceMesh *, int, int) --> struct easy3d::SurfaceMesh::Halfedge", pybind11::arg("model"), pybind11::arg("x"), pybind11::arg("y"));
		cl.def("pick_vertex", (struct easy3d::SurfaceMesh::Vertex (easy3d::SurfaceMeshPicker::*)(class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Face, int, int)) &easy3d::SurfaceMeshPicker::pick_vertex, "Pick a vertex from a surface mesh given the cursor position and a known picked face.\n \n\n The cursor x-coordinate, relative to the left edge of the content area.\n \n\n The cursor y-coordinate, relative to the top edge of the content area.\n \n\n The picked face.\n \n\n The screen point is expressed in the screen coordinate system with an origin in the upper left\n            corner. So it doesn't necessarily correspond to a pixel on High DPI devices, e.g. a Mac with\n            a Retina display.\n \n\n The picked vertex.\n \n\n This method must be called after calling to pick_face(). The result is valid only if the\n            picked_face is valid.\n\nC++: easy3d::SurfaceMeshPicker::pick_vertex(class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Face, int, int) --> struct easy3d::SurfaceMesh::Vertex", pybind11::arg("model"), pybind11::arg("picked_face"), pybind11::arg("x"), pybind11::arg("y"));
		cl.def("pick_edge", (struct easy3d::SurfaceMesh::Halfedge (easy3d::SurfaceMeshPicker::*)(class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Face, int, int)) &easy3d::SurfaceMeshPicker::pick_edge, "Pick an edge from a surface mesh given the cursor position and a known picked face.\n \n\n The cursor x-coordinate, relative to the left edge of the content area.\n \n\n The cursor y-coordinate, relative to the top edge of the content area.\n \n\n The picked face.\n \n\n The screen point is expressed in the screen coordinate system with an origin in the upper left\n            corner. So it doesn't necessarily correspond to a pixel on High DPI devices, e.g. a Mac with\n            a Retina display.\n \n\n The picked halfedge.\n \n\n This method must be called after calling to pick_face(). The result is valid only if the\n            picked_face is valid.\n\nC++: easy3d::SurfaceMeshPicker::pick_edge(class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Face, int, int) --> struct easy3d::SurfaceMesh::Halfedge", pybind11::arg("model"), pybind11::arg("picked_face"), pybind11::arg("x"), pybind11::arg("y"));
		cl.def("picked_face", (struct easy3d::SurfaceMesh::Face (easy3d::SurfaceMeshPicker::*)() const) &easy3d::SurfaceMeshPicker::picked_face, "Query the previously picked face.\n \n\n The previously picked face.\n \n\n This method must be called after calling to one of the above pick element methods. The results is\n            valid only if a face has been picked.\n\nC++: easy3d::SurfaceMeshPicker::picked_face() const --> struct easy3d::SurfaceMesh::Face");
		cl.def("picked_point", (class easy3d::Vec<3, float> (easy3d::SurfaceMeshPicker::*)(class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Face, int, int) const) &easy3d::SurfaceMeshPicker::picked_point, "Query the coordinate of the previously picked position, which is the intersection between the picking\n line and the picked face.\n \n\n The cursor x-coordinate, relative to the left edge of the content area.\n \n\n The cursor y-coordinate, relative to the top edge of the content area.\n \n\n The picked face.\n \n\n The xyz coordinate of the picked position.\n \n\n This method must be called after calling to pick_face(). The result is valid only if the\n            picked_face is valid.\n\nC++: easy3d::SurfaceMeshPicker::picked_point(class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Face, int, int) const --> class easy3d::Vec<3, float>", pybind11::arg("model"), pybind11::arg("picked_face"), pybind11::arg("x"), pybind11::arg("y"));
		cl.def("assign", (class easy3d::SurfaceMeshPicker & (easy3d::SurfaceMeshPicker::*)(const class easy3d::SurfaceMeshPicker &)) &easy3d::SurfaceMeshPicker::operator=, "C++: easy3d::SurfaceMeshPicker::operator=(const class easy3d::SurfaceMeshPicker &) --> class easy3d::SurfaceMeshPicker &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}

}
