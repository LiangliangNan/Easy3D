#include <__functional/operations.h>
#include <easy3d/core/box.h>
#include <easy3d/core/graph.h>
#include <easy3d/core/mat.h>
#include <easy3d/core/model.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/poly_mesh.h>
#include <easy3d/core/quat.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/vec.h>
#include <easy3d/renderer/buffer.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/frame.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/manipulated_frame.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/state.h>
#include <easy3d/renderer/texture.h>
#include <easy3d/renderer/vertex_array_object.h>
#include <functional>
#include <ios>
#include <iterator>
#include <memory>
#include <ostream>
#include <set>
#include <streambuf>
#include <string>
#include <string_view>
#include <typeinfo>
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

void bind_easy3d_renderer_buffer(pybind11::module_& m)
{
	// easy3d::buffer::update(class easy3d::Model *, class easy3d::Drawable *) file:easy3d/renderer/buffer.h line:60
	M("easy3d::buffer").def("update", (void (*)(class easy3d::Model *, class easy3d::Drawable *)) &easy3d::buffer::update, "Update render buffers of a drawable. Coloring determined by the drawable's coloring scheme.\n \n\n     The model.\n \n\n  The drawable.\n\nC++: easy3d::buffer::update(class easy3d::Model *, class easy3d::Drawable *) --> void", pybind11::arg("model"), pybind11::arg("drawable"));

	// easy3d::buffer::update(class easy3d::PointCloud *, class easy3d::PointsDrawable *) file:easy3d/renderer/buffer.h line:73
	M("easy3d::buffer").def("update", (void (*)(class easy3d::PointCloud *, class easy3d::PointsDrawable *)) &easy3d::buffer::update, "Update render buffers for the default \"vertices\" drawable of a point cloud.\n Coloring determined by the drawable's coloring scheme.\n \n\n     The model.\n \n\n  The drawable.\n\nC++: easy3d::buffer::update(class easy3d::PointCloud *, class easy3d::PointsDrawable *) --> void", pybind11::arg("model"), pybind11::arg("drawable"));

	// easy3d::buffer::update(class easy3d::PointCloud *, class easy3d::LinesDrawable *, const std::string &, float) file:easy3d/renderer/buffer.h line:82
	M("easy3d::buffer").def("update", (void (*)(class easy3d::PointCloud *, class easy3d::LinesDrawable *, const std::string &, float)) &easy3d::buffer::update, "Update render buffers for a vector field defined on a point cloud.\n \n\n     The model.\n \n\n  The drawable.\n \n\n The name of the vector field.\n \n\n The length scale of the vectors is w.r.t. (0.01 * radius) of the model's bounding sphere.\n\nC++: easy3d::buffer::update(class easy3d::PointCloud *, class easy3d::LinesDrawable *, const std::string &, float) --> void", pybind11::arg("model"), pybind11::arg("drawable"), pybind11::arg("field"), pybind11::arg("scale"));

	// easy3d::buffer::update(class easy3d::SurfaceMesh *, class easy3d::PointsDrawable *) file:easy3d/renderer/buffer.h line:96
	M("easy3d::buffer").def("update", (void (*)(class easy3d::SurfaceMesh *, class easy3d::PointsDrawable *)) &easy3d::buffer::update, "Update render buffers for the default \"vertices\" drawable of a surface mesh.\n Coloring determined by the drawable's coloring scheme.\n \n\n     The model.\n \n\n  The drawable.\n\nC++: easy3d::buffer::update(class easy3d::SurfaceMesh *, class easy3d::PointsDrawable *) --> void", pybind11::arg("model"), pybind11::arg("drawable"));

	// easy3d::buffer::update(class easy3d::SurfaceMesh *, class easy3d::LinesDrawable *) file:easy3d/renderer/buffer.h line:104
	M("easy3d::buffer").def("update", (void (*)(class easy3d::SurfaceMesh *, class easy3d::LinesDrawable *)) &easy3d::buffer::update, "Update render buffers for the default \"edges\" drawable of a surface mesh.\n Coloring determined by the drawable's coloring scheme.\n \n\n     The model.\n \n\n  The drawable.\n\nC++: easy3d::buffer::update(class easy3d::SurfaceMesh *, class easy3d::LinesDrawable *) --> void", pybind11::arg("model"), pybind11::arg("drawable"));

	// easy3d::buffer::update(class easy3d::SurfaceMesh *, class easy3d::TrianglesDrawable *) file:easy3d/renderer/buffer.h line:112
	M("easy3d::buffer").def("update", (void (*)(class easy3d::SurfaceMesh *, class easy3d::TrianglesDrawable *)) &easy3d::buffer::update, "Update render buffers for the default \"faces\" drawable of a surface mesh.\n Coloring determined by the drawable's coloring scheme.\n \n\n     The model.\n \n\n  The drawable.\n\nC++: easy3d::buffer::update(class easy3d::SurfaceMesh *, class easy3d::TrianglesDrawable *) --> void", pybind11::arg("model"), pybind11::arg("drawable"));

	// easy3d::buffer::update(class easy3d::SurfaceMesh *, class easy3d::LinesDrawable *, const std::string &, enum easy3d::State::Location, float) file:easy3d/renderer/buffer.h line:122
	M("easy3d::buffer").def("update", (void (*)(class easy3d::SurfaceMesh *, class easy3d::LinesDrawable *, const std::string &, enum easy3d::State::Location, float)) &easy3d::buffer::update, "Update render buffers for a vector field defined on a surface mesh.\n \n\n     The model.\n \n\n  The The drawable for visualizing the vector field.\n \n\n The name of the vector field.\n \n\n The location where the vector field is defined.\n \n\n The scale of the vector length w.r.t. half of the average edge length of the surface mesh.\n\nC++: easy3d::buffer::update(class easy3d::SurfaceMesh *, class easy3d::LinesDrawable *, const std::string &, enum easy3d::State::Location, float) --> void", pybind11::arg("model"), pybind11::arg("drawable"), pybind11::arg("field"), pybind11::arg("location"), pybind11::arg("scale"));

	// easy3d::buffer::update(class easy3d::Graph *, class easy3d::PointsDrawable *) file:easy3d/renderer/buffer.h line:136
	M("easy3d::buffer").def("update", (void (*)(class easy3d::Graph *, class easy3d::PointsDrawable *)) &easy3d::buffer::update, "Update render buffers for the default \"vertices\" drawable of a graph.\n Coloring determined by the drawable's coloring scheme.\n \n\n     The model.\n \n\n  The drawable.\n\nC++: easy3d::buffer::update(class easy3d::Graph *, class easy3d::PointsDrawable *) --> void", pybind11::arg("model"), pybind11::arg("drawable"));

	// easy3d::buffer::update(class easy3d::Graph *, class easy3d::LinesDrawable *) file:easy3d/renderer/buffer.h line:145
	M("easy3d::buffer").def("update", (void (*)(class easy3d::Graph *, class easy3d::LinesDrawable *)) &easy3d::buffer::update, "Update render buffers for the default \"edges\" drawable of a graph.\n Coloring determined by the drawable's coloring scheme.\n \n\n     The model.\n \n\n  The drawable.\n\nC++: easy3d::buffer::update(class easy3d::Graph *, class easy3d::LinesDrawable *) --> void", pybind11::arg("model"), pybind11::arg("drawable"));

	// easy3d::buffer::update(class easy3d::PolyMesh *, class easy3d::PointsDrawable *) file:easy3d/renderer/buffer.h line:160
	M("easy3d::buffer").def("update", (void (*)(class easy3d::PolyMesh *, class easy3d::PointsDrawable *)) &easy3d::buffer::update, "Update render buffers for the default \"vertices\" drawable of a polyhedral mesh.\n Coloring determined by the drawable's coloring scheme.\n \n\n     The model.\n \n\n  The drawable.\n\nC++: easy3d::buffer::update(class easy3d::PolyMesh *, class easy3d::PointsDrawable *) --> void", pybind11::arg("model"), pybind11::arg("drawable"));

	// easy3d::buffer::update(class easy3d::PolyMesh *, class easy3d::LinesDrawable *) file:easy3d/renderer/buffer.h line:168
	M("easy3d::buffer").def("update", (void (*)(class easy3d::PolyMesh *, class easy3d::LinesDrawable *)) &easy3d::buffer::update, "Update render buffers for the default \"edges\" drawable of a polyhedral mesh.\n Coloring determined by the drawable's coloring scheme.\n \n\n     The model.\n \n\n  The drawable.\n\nC++: easy3d::buffer::update(class easy3d::PolyMesh *, class easy3d::LinesDrawable *) --> void", pybind11::arg("model"), pybind11::arg("drawable"));

	// easy3d::buffer::update(class easy3d::PolyMesh *, class easy3d::TrianglesDrawable *, bool) file:easy3d/renderer/buffer.h line:178
	M("easy3d::buffer").def("update", (void (*)(class easy3d::PolyMesh *, class easy3d::TrianglesDrawable *, bool)) &easy3d::buffer::update, "Update render buffers for the default \"faces\" drawables of a polyhedral mesh.\n \n\n Interior and boundary faces are rendered using two drawables. Thus, this function has an extra\n      parameter to specify for which drawable the renderer buffers are be updated.\n \n\n     The model.\n \n\n  The drawable.\n \n\n   for the boundary drawable and  for the interior drawable.\n\nC++: easy3d::buffer::update(class easy3d::PolyMesh *, class easy3d::TrianglesDrawable *, bool) --> void", pybind11::arg("model"), pybind11::arg("drawable"), pybind11::arg("border"));

	// easy3d::buffer::update(class easy3d::PolyMesh *, class easy3d::LinesDrawable *, const std::string &, enum easy3d::State::Location, float) file:easy3d/renderer/buffer.h line:189
	M("easy3d::buffer").def("update", (void (*)(class easy3d::PolyMesh *, class easy3d::LinesDrawable *, const std::string &, enum easy3d::State::Location, float)) &easy3d::buffer::update, "Update render buffers for a vector field defined on a polyhedral mesh.\n \n\n     The model.\n \n\n  The drawable for visualizing the vector field.\n \n\n The name of the vector field.\n \n\n The location where the vector field is defined.\n \n\n The scale of the vector length w.r.t. half of the average edge length of the polyhedral mesh.\n \n\n Only the vector fields on border faces, border vertices, and border edges are supported.\n\nC++: easy3d::buffer::update(class easy3d::PolyMesh *, class easy3d::LinesDrawable *, const std::string &, enum easy3d::State::Location, float) --> void", pybind11::arg("model"), pybind11::arg("drawable"), pybind11::arg("field"), pybind11::arg("location"), pybind11::arg("scale"));

}
