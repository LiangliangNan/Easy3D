#include <easy3d/core/box.h>
#include <easy3d/core/model.h>
#include <easy3d/core/vec.h>
#include <easy3d/renderer/renderer.h>

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


void bind_easy3d_core_model(pybind11::module_& m)
{
	{ // easy3d::Model file:easy3d/core/model.h line:48
		pybind11::class_<easy3d::Model, std::shared_ptr<easy3d::Model>> cl(m, "Model", "The base class of renderable 3D models.\n \n\n\n \n A model can represent a SurfaceMesh, a PointCloud, or a Graph. A model can have multiple drawables,\n e.g., faces, vertices, edges.");

        cl.def("name", [](easy3d::Model& self) { return self.name(); }, pybind11::return_value_policy::copy, "Get the name of the model.");
        cl.def("set_name", [](easy3d::Model& self, const std::string& name) { self.set_name(name); }, "Set the name of the model.");

        cl.def("renderer", (class easy3d::Renderer* (easy3d::Model::*)()) &easy3d::Model::renderer, "Returns the renderer of this model", pybind11::return_value_policy::reference_internal);
        cl.def("renderer", (const class easy3d::Renderer* (easy3d::Model::*)() const) &easy3d::Model::renderer, "Returns the renderer of this model", pybind11::return_value_policy::reference_internal);

        cl.def("bounding_box", [](easy3d::Model const &o) -> const easy3d::GenericBox<3, float> & { return o.bounding_box(); }, "", pybind11::return_value_policy::automatic);
		cl.def("bounding_box", (const class easy3d::GenericBox<3, float> & (easy3d::Model::*)(bool) const) &easy3d::Model::bounding_box, "The bounding box of the model.\n \n\n If  or if the bounding box is not known, it computes and returns the bounding\n      box of the model. Otherwise, it returns the known bounding box.\n \n\n Manipulation transformation is not handled.\n \n\n invalidate_bounding_box().\n\nC++: easy3d::Model::bounding_box(bool) const --> const class easy3d::GenericBox<3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("recompute"));
		cl.def("invalidate_bounding_box", (void (easy3d::Model::*)()) &easy3d::Model::invalidate_bounding_box, "Invalidates the bounding box of the model. So when bounding_box() is called, the bounding box will\n be re-computed. This function is typically called when the geometry of a model is changed.\n\nC++: easy3d::Model::invalidate_bounding_box() --> void");
		cl.def("points", (class std::vector<class easy3d::Vec<3, float> > & (easy3d::Model::*)()) &easy3d::Model::points, "The vertices of the model. \n\nC++: easy3d::Model::points() --> class std::vector<class easy3d::Vec<3, float> > &", pybind11::return_value_policy::automatic);
        cl.def("points", (const class std::vector<class easy3d::Vec<3, float> > & (easy3d::Model::*)() const) &easy3d::Model::points, "The vertices of the model. \n\nC++: easy3d::Model::points() --> class std::vector<class easy3d::Vec<3, float> > &", pybind11::return_value_policy::automatic);
		cl.def("empty", (bool (easy3d::Model::*)() const) &easy3d::Model::empty, "Tests if the model is empty. \n\nC++: easy3d::Model::empty() const --> bool");
		cl.def("property_stats", (void (easy3d::Model::*)(std::ostream &) const) &easy3d::Model::property_stats, "Prints the names of all properties to an output stream (e.g., std::cout). \n\nC++: easy3d::Model::property_stats(std::ostream &) const --> void", pybind11::arg("output"));
		cl.def("assign", (class easy3d::Model & (easy3d::Model::*)(const class easy3d::Model &)) &easy3d::Model::operator=, "C++: easy3d::Model::operator=(const class easy3d::Model &) --> class easy3d::Model &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}
}
