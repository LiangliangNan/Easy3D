#include <easy3d/core/graph.h>
#include <easy3d/core/model.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/renderer.h>

#include <memory>
#include <utility>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>


#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_renderer_renderer(pybind11::module_& m)
{
	{ // easy3d::Renderer file:easy3d/renderer/renderer.h line:62
		pybind11::class_<easy3d::Renderer, std::shared_ptr<easy3d::Renderer>> cl(m, "Renderer", "A Renderer manages the drawables (and thus the rendering) of a model.\n \n\n Renderer also allow to create default drawables for most rendering purposes.\n \n\n\n \n Drawable, PointsDrawable, LinesDrawable, TrianglesDrawable\n\n The following code shows how to use renderer:\n \n\n\n\n\n \n\n\n\n     ");
		cl.def( pybind11::init( [](class easy3d::Model * a0){ return new easy3d::Renderer(a0); } ), "doc" , pybind11::arg("model"));
		cl.def( pybind11::init<class easy3d::Model *, bool>(), pybind11::arg("model"), pybind11::arg("create_drawables") );

		cl.def( pybind11::init( [](easy3d::Renderer const &o){ return new easy3d::Renderer(o); } ) );
		cl.def("model", (class easy3d::Model * (easy3d::Renderer::*)()) &easy3d::Renderer::model, "The model to which this renderer is attached.\n\nC++: easy3d::Renderer::model() --> class easy3d::Model *", pybind11::return_value_policy::automatic);
		cl.def("is_visible", (bool (easy3d::Renderer::*)() const) &easy3d::Renderer::is_visible, "Returns whether the model is currently visible.\n\nC++: easy3d::Renderer::is_visible() const --> bool");
		cl.def("set_visible", (void (easy3d::Renderer::*)(bool)) &easy3d::Renderer::set_visible, "Shows/Hides the model.\n\nC++: easy3d::Renderer::set_visible(bool) --> void", pybind11::arg("b"));
		cl.def("is_selected", (bool (easy3d::Renderer::*)() const) &easy3d::Renderer::is_selected, "Returns whether the model has been selected.\n\nC++: easy3d::Renderer::is_selected() const --> bool");
		cl.def("set_selected", (void (easy3d::Renderer::*)(bool)) &easy3d::Renderer::set_selected, "Select/Deselect the model. The state of all its drawables will change accordingly.\n\nC++: easy3d::Renderer::set_selected(bool) --> void", pybind11::arg("b"));
		cl.def("update", (void (easy3d::Renderer::*)()) &easy3d::Renderer::update, "Invalidates the rendering buffers of the model and thus updates the rendering (delayed in rendering).\n \n\n This method triggers an update of the rendering buffers of all the drawables of the model to which\n      this renderer is attached. The effect is equivalent to calling Drawable::update() functions for all\n      the drawables of this model.\n todo: for better performance, it is wise to update only the affected drawables and buffers.\n \n\n  Drawable::update()\n\nC++: easy3d::Renderer::update() --> void");
		cl.def("get_points_drawable", [](easy3d::Renderer const &o, const std::string & a0) -> easy3d::PointsDrawable * { return o.get_points_drawable(a0); }, "", pybind11::return_value_policy::reference_internal, pybind11::arg("name"));
		cl.def("get_points_drawable", (class easy3d::PointsDrawable * (easy3d::Renderer::*)(const std::string &, bool) const) &easy3d::Renderer::get_points_drawable, "Get the points drawable with a given name.\n Return nullptr if the drawable does not exist.\n\nC++: easy3d::Renderer::get_points_drawable(const std::string &, bool) const --> class easy3d::PointsDrawable *", pybind11::return_value_policy::reference_internal, pybind11::arg("name"), pybind11::arg("warning_not_found"));
		cl.def("get_lines_drawable", [](easy3d::Renderer const &o, const std::string & a0) -> easy3d::LinesDrawable * { return o.get_lines_drawable(a0); }, "", pybind11::return_value_policy::reference_internal, pybind11::arg("name"));
		cl.def("get_lines_drawable", (class easy3d::LinesDrawable * (easy3d::Renderer::*)(const std::string &, bool) const) &easy3d::Renderer::get_lines_drawable, "Get the lines drawable with a given name.\n Return nullptr if the drawable does not exist.\n\nC++: easy3d::Renderer::get_lines_drawable(const std::string &, bool) const --> class easy3d::LinesDrawable *", pybind11::return_value_policy::reference_internal, pybind11::arg("name"), pybind11::arg("warning_not_found"));
		cl.def("get_triangles_drawable", [](easy3d::Renderer const &o, const std::string & a0) -> easy3d::TrianglesDrawable * { return o.get_triangles_drawable(a0); }, "", pybind11::return_value_policy::reference_internal, pybind11::arg("name"));
		cl.def("get_triangles_drawable", (class easy3d::TrianglesDrawable * (easy3d::Renderer::*)(const std::string &, bool) const) &easy3d::Renderer::get_triangles_drawable, "Get the triangles drawable with a given name.\n Return nullptr if the drawable does not exist.\n\nC++: easy3d::Renderer::get_triangles_drawable(const std::string &, bool) const --> class easy3d::TrianglesDrawable *", pybind11::return_value_policy::reference_internal, pybind11::arg("name"), pybind11::arg("warning_not_found"));
		cl.def("add_points_drawable", (class easy3d::PointsDrawable * (easy3d::Renderer::*)(const std::string &)) &easy3d::Renderer::add_points_drawable, "Create a new points drawable and add it to this renderer.\n \n\n The name of the points drawable to be created.\n \n\n The created points drawable. If a points drawable with 'name' already exists, the creation will be\n         ignored and the existing drawable is returned.\n\nC++: easy3d::Renderer::add_points_drawable(const std::string &) --> class easy3d::PointsDrawable *", pybind11::return_value_policy::automatic, pybind11::arg("name"));
		cl.def("add_lines_drawable", (class easy3d::LinesDrawable * (easy3d::Renderer::*)(const std::string &)) &easy3d::Renderer::add_lines_drawable, "Create a new lines drawable and add it to this renderer.\n \n\n The name of the lines drawable to be created.\n \n\n The created lines drawable. If a lines drawable with 'name' already exists, the creation will be\n         ignored and the existing drawable is returned.\n\nC++: easy3d::Renderer::add_lines_drawable(const std::string &) --> class easy3d::LinesDrawable *", pybind11::return_value_policy::automatic, pybind11::arg("name"));
		cl.def("add_triangles_drawable", (class easy3d::TrianglesDrawable * (easy3d::Renderer::*)(const std::string &)) &easy3d::Renderer::add_triangles_drawable, "Create a new triangles drawable and add it to this renderer.\n \n\n The name of the triangles drawable to be created.\n \n\n The created triangles drawable. If a triangles drawable with 'name' already exists, the creation will\n         be ignored and the existing drawable is returned.\n\nC++: easy3d::Renderer::add_triangles_drawable(const std::string &) --> class easy3d::TrianglesDrawable *", pybind11::return_value_policy::automatic, pybind11::arg("name"));
		cl.def("points_drawables", (const class std::vector<class easy3d::PointsDrawable *> & (easy3d::Renderer::*)() const) &easy3d::Renderer::points_drawables, "All available points drawables managed by this renderer.\n\nC++: easy3d::Renderer::points_drawables() const --> const class std::vector<class easy3d::PointsDrawable *> &", pybind11::return_value_policy::reference_internal);
		cl.def("lines_drawables", (const class std::vector<class easy3d::LinesDrawable *> & (easy3d::Renderer::*)() const) &easy3d::Renderer::lines_drawables, "All available lines drawables managed by this renderer.\n\nC++: easy3d::Renderer::lines_drawables() const --> const class std::vector<class easy3d::LinesDrawable *> &", pybind11::return_value_policy::reference_internal);
		cl.def("triangles_drawables", (const class std::vector<class easy3d::TrianglesDrawable *> & (easy3d::Renderer::*)() const) &easy3d::Renderer::triangles_drawables, "All available triangles drawables managed by this renderer.\n\nC++: easy3d::Renderer::triangles_drawables() const --> const class std::vector<class easy3d::TrianglesDrawable *> &", pybind11::return_value_policy::reference_internal);
		cl.def("assign", (class easy3d::Renderer & (easy3d::Renderer::*)(const class easy3d::Renderer &)) &easy3d::Renderer::operator=, "C++: easy3d::Renderer::operator=(const class easy3d::Renderer &) --> class easy3d::Renderer &", pybind11::return_value_policy::automatic, pybind11::arg(""));

        cl.def("color_by_segmentation", [](easy3d::Renderer &self, const std::string &property_name) -> void {
                   if (dynamic_cast<easy3d::PointCloud *>(self.model())) {
                       auto cloud = dynamic_cast<easy3d::PointCloud *>(self.model());
                       auto segments = cloud->vertex_property<int>(property_name);
                       const std::string color_name = "v:color-segments";
                       auto coloring = cloud->vertex_property<easy3d::vec3>(color_name, easy3d::vec3(0, 0, 0));
                       easy3d::Renderer::color_from_segmentation(cloud, segments, coloring);
                       auto vertices = cloud->renderer()->get_points_drawable("vertices");
                       vertices->set_property_coloring(easy3d::State::VERTEX, color_name);
                   } else if (dynamic_cast<easy3d::SurfaceMesh *>(self.model())) {
                       auto mesh = dynamic_cast<easy3d::SurfaceMesh *>(self.model());
                       auto segments = mesh->face_property<int>(property_name);
                       const std::string color_name = "f:color-segments";
                       auto coloring = mesh->face_property<easy3d::vec3>(color_name, easy3d::vec3(0, 0, 0));
                       easy3d::Renderer::color_from_segmentation(mesh, segments, coloring);
                       auto faces = mesh->renderer()->get_triangles_drawable("faces");
                       faces->set_property_coloring(easy3d::State::FACE, color_name);
                   } else {
                       throw std::runtime_error(
                               "Current implementation of color_by_segmentation() only supports PointCloud and SurfaceMesh.");
                   }
               },
               "Colorize a model by its segmentation information stored as an integer property \"property_name\", e.g., \"v:primitive_index\" for point clouds, and \"f:chart\" for surface meshes.",
               pybind11::arg("property_name"));

    }

}
