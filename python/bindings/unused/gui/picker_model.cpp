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
	{ // easy3d::Model file:easy3d/core/model.h line:48
		pybind11::class_<easy3d::Model, std::shared_ptr<easy3d::Model>> cl(m, "Model", "The base class of renderable 3D models.\n \n\n\n \n A model can represent a SurfaceMesh, a PointCloud, or a Graph. A model can have multiple drawables,\n e.g., faces, vertices, edges.");
		cl.def("set_name", (void (easy3d::Model::*)(const std::string &)) &easy3d::Model::set_name, "Sets/Changes the name of a model.\n Assigning a name to a model is optional, but it is useful for handling multiple models with the same viewer.\n\nC++: easy3d::Model::set_name(const std::string &) --> void", pybind11::arg("n"));
		cl.def("name", (const std::string & (easy3d::Model::*)() const) &easy3d::Model::name, "The name of a model.\n\nC++: easy3d::Model::name() const --> const std::string &", pybind11::return_value_policy::automatic);
		cl.def("bounding_box", [](easy3d::Model const &o) -> const easy3d::GenericBox<3, float> & { return o.bounding_box(); }, "", pybind11::return_value_policy::automatic);
		cl.def("bounding_box", (const class easy3d::GenericBox<3, float> & (easy3d::Model::*)(bool) const) &easy3d::Model::bounding_box, "The bounding box of the model.\n \n\n If  or if the bounding box is not known, it computes and returns the bounding\n      box of the model. Otherwise, it returns the known bounding box.\n \n\n Manipulation transformation is not handled.\n \n\n invalidate_bounding_box().\n\nC++: easy3d::Model::bounding_box(bool) const --> const class easy3d::GenericBox<3, float> &", pybind11::return_value_policy::automatic, pybind11::arg("recompute"));
		cl.def("invalidate_bounding_box", (void (easy3d::Model::*)()) &easy3d::Model::invalidate_bounding_box, "Invalidates the bounding box of the model. So when bounding_box() is called, the bounding box will\n be re-computed. This function is typically called when the geometry of a model is changed.\n\nC++: easy3d::Model::invalidate_bounding_box() --> void");
		cl.def("points", (class std::vector<class easy3d::Vec<3, float> > & (easy3d::Model::*)()) &easy3d::Model::points, "The vertices of the model. \n\nC++: easy3d::Model::points() --> class std::vector<class easy3d::Vec<3, float> > &", pybind11::return_value_policy::automatic);
		cl.def("empty", (bool (easy3d::Model::*)() const) &easy3d::Model::empty, "Tests if the model is empty. \n\nC++: easy3d::Model::empty() const --> bool");
		cl.def("property_stats", (void (easy3d::Model::*)(std::ostream &) const) &easy3d::Model::property_stats, "Prints the names of all properties to an output stream (e.g., std::cout). \n\nC++: easy3d::Model::property_stats(std::ostream &) const --> void", pybind11::arg("output"));
		cl.def("assign", (class easy3d::Model & (easy3d::Model::*)(const class easy3d::Model &)) &easy3d::Model::operator=, "C++: easy3d::Model::operator=(const class easy3d::Model &) --> class easy3d::Model &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}
	{ // easy3d::BasePropertyArray file:easy3d/core/property.h line:40
		pybind11::class_<easy3d::BasePropertyArray, std::shared_ptr<easy3d::BasePropertyArray>, PyCallBack_easy3d_BasePropertyArray> cl(m, "BasePropertyArray", "Base class for a property array.\n \n");
		cl.def( pybind11::init<const std::string &>(), pybind11::arg("name") );

		cl.def(pybind11::init<PyCallBack_easy3d_BasePropertyArray const &>());
		cl.def("reserve", (void (easy3d::BasePropertyArray::*)(unsigned long)) &easy3d::BasePropertyArray::reserve, "Reserve memory for n elements.\n\nC++: easy3d::BasePropertyArray::reserve(unsigned long) --> void", pybind11::arg("n"));
		cl.def("resize", (void (easy3d::BasePropertyArray::*)(unsigned long)) &easy3d::BasePropertyArray::resize, "Resize storage to hold n elements.\n\nC++: easy3d::BasePropertyArray::resize(unsigned long) --> void", pybind11::arg("n"));
		cl.def("shrink_to_fit", (void (easy3d::BasePropertyArray::*)()) &easy3d::BasePropertyArray::shrink_to_fit, "Free unused memory.\n\nC++: easy3d::BasePropertyArray::shrink_to_fit() --> void");
		cl.def("push_back", (void (easy3d::BasePropertyArray::*)()) &easy3d::BasePropertyArray::push_back, "Extend the number of elements by one.\n\nC++: easy3d::BasePropertyArray::push_back() --> void");
		cl.def("reset", (void (easy3d::BasePropertyArray::*)(unsigned long)) &easy3d::BasePropertyArray::reset, "Reset element to default value\n\nC++: easy3d::BasePropertyArray::reset(unsigned long) --> void", pybind11::arg("idx"));
		cl.def("transfer", (bool (easy3d::BasePropertyArray::*)(const class easy3d::BasePropertyArray &)) &easy3d::BasePropertyArray::transfer, "Copy the entire properties from \n\nC++: easy3d::BasePropertyArray::transfer(const class easy3d::BasePropertyArray &) --> bool", pybind11::arg("other"));
		cl.def("transfer", (bool (easy3d::BasePropertyArray::*)(const class easy3d::BasePropertyArray &, std::size_t, std::size_t)) &easy3d::BasePropertyArray::transfer, "Copy the property[from] of  to this->property[to].\n\nC++: easy3d::BasePropertyArray::transfer(const class easy3d::BasePropertyArray &, std::size_t, std::size_t) --> bool", pybind11::arg("other"), pybind11::arg("from"), pybind11::arg("to"));
		cl.def("swap", (void (easy3d::BasePropertyArray::*)(unsigned long, unsigned long)) &easy3d::BasePropertyArray::swap, "Let two elements swap their storage place.\n\nC++: easy3d::BasePropertyArray::swap(unsigned long, unsigned long) --> void", pybind11::arg("i0"), pybind11::arg("i1"));
		cl.def("copy", (void (easy3d::BasePropertyArray::*)(unsigned long, unsigned long)) &easy3d::BasePropertyArray::copy, "Let copy 'from' -> 'to'.\n\nC++: easy3d::BasePropertyArray::copy(unsigned long, unsigned long) --> void", pybind11::arg("from"), pybind11::arg("to"));
		cl.def("clone", (class easy3d::BasePropertyArray * (easy3d::BasePropertyArray::*)() const) &easy3d::BasePropertyArray::clone, "Return a deep copy of self.\n\nC++: easy3d::BasePropertyArray::clone() const --> class easy3d::BasePropertyArray *", pybind11::return_value_policy::automatic);
		cl.def("empty_clone", (class easy3d::BasePropertyArray * (easy3d::BasePropertyArray::*)() const) &easy3d::BasePropertyArray::empty_clone, "Return a empty copy of self.\n\nC++: easy3d::BasePropertyArray::empty_clone() const --> class easy3d::BasePropertyArray *", pybind11::return_value_policy::automatic);
		cl.def("type", (const class std::type_info & (easy3d::BasePropertyArray::*)() const) &easy3d::BasePropertyArray::type, "Return the type_info of the property\n\nC++: easy3d::BasePropertyArray::type() const --> const class std::type_info &", pybind11::return_value_policy::automatic);
		cl.def("name", (const std::string & (easy3d::BasePropertyArray::*)() const) &easy3d::BasePropertyArray::name, "Return the name of the property\n\nC++: easy3d::BasePropertyArray::name() const --> const std::string &", pybind11::return_value_policy::automatic);
		cl.def("set_name", (void (easy3d::BasePropertyArray::*)(const std::string &)) &easy3d::BasePropertyArray::set_name, "Set the name of the property\n\nC++: easy3d::BasePropertyArray::set_name(const std::string &) --> void", pybind11::arg("n"));
		cl.def("is_same", (bool (easy3d::BasePropertyArray::*)(const class easy3d::BasePropertyArray &) const) &easy3d::BasePropertyArray::is_same, "Test if two properties are the same.\n \n\n true only if their names and types are both identical.\n\nC++: easy3d::BasePropertyArray::is_same(const class easy3d::BasePropertyArray &) const --> bool", pybind11::arg("other"));
		cl.def("assign", (class easy3d::BasePropertyArray & (easy3d::BasePropertyArray::*)(const class easy3d::BasePropertyArray &)) &easy3d::BasePropertyArray::operator=, "C++: easy3d::BasePropertyArray::operator=(const class easy3d::BasePropertyArray &) --> class easy3d::BasePropertyArray &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}
	{ // easy3d::PropertyArray file:easy3d/core/property.h line:110
		pybind11::class_<easy3d::PropertyArray<bool>, std::shared_ptr<easy3d::PropertyArray<bool>>, PyCallBack_easy3d_PropertyArray_bool_t, easy3d::BasePropertyArray> cl(m, "PropertyArray_bool_t", "");
		cl.def( pybind11::init( [](const std::string & a0){ return new easy3d::PropertyArray<bool>(a0); }, [](const std::string & a0){ return new PyCallBack_easy3d_PropertyArray_bool_t(a0); } ), "doc");
		cl.def( pybind11::init<const std::string &, bool>(), pybind11::arg("name"), pybind11::arg("t") );

		cl.def( pybind11::init( [](PyCallBack_easy3d_PropertyArray_bool_t const &o){ return new PyCallBack_easy3d_PropertyArray_bool_t(o); } ) );
		cl.def( pybind11::init( [](easy3d::PropertyArray<bool> const &o){ return new easy3d::PropertyArray<bool>(o); } ) );
		cl.def("reserve", (void (easy3d::PropertyArray<bool>::*)(unsigned long)) &easy3d::PropertyArray<bool>::reserve, "C++: easy3d::PropertyArray<bool>::reserve(unsigned long) --> void", pybind11::arg("n"));
		cl.def("resize", (void (easy3d::PropertyArray<bool>::*)(unsigned long)) &easy3d::PropertyArray<bool>::resize, "C++: easy3d::PropertyArray<bool>::resize(unsigned long) --> void", pybind11::arg("n"));
		cl.def("push_back", (void (easy3d::PropertyArray<bool>::*)()) &easy3d::PropertyArray<bool>::push_back, "C++: easy3d::PropertyArray<bool>::push_back() --> void");
		cl.def("reset", (void (easy3d::PropertyArray<bool>::*)(unsigned long)) &easy3d::PropertyArray<bool>::reset, "C++: easy3d::PropertyArray<bool>::reset(unsigned long) --> void", pybind11::arg("idx"));
		cl.def("transfer", (bool (easy3d::PropertyArray<bool>::*)(const class easy3d::BasePropertyArray &)) &easy3d::PropertyArray<bool>::transfer, "C++: easy3d::PropertyArray<bool>::transfer(const class easy3d::BasePropertyArray &) --> bool", pybind11::arg("other"));
		cl.def("transfer", (bool (easy3d::PropertyArray<bool>::*)(const class easy3d::BasePropertyArray &, std::size_t, std::size_t)) &easy3d::PropertyArray<bool>::transfer, "C++: easy3d::PropertyArray<bool>::transfer(const class easy3d::BasePropertyArray &, std::size_t, std::size_t) --> bool", pybind11::arg("other"), pybind11::arg("from"), pybind11::arg("to"));
		cl.def("shrink_to_fit", (void (easy3d::PropertyArray<bool>::*)()) &easy3d::PropertyArray<bool>::shrink_to_fit, "C++: easy3d::PropertyArray<bool>::shrink_to_fit() --> void");
		cl.def("swap", (void (easy3d::PropertyArray<bool>::*)(unsigned long, unsigned long)) &easy3d::PropertyArray<bool>::swap, "C++: easy3d::PropertyArray<bool>::swap(unsigned long, unsigned long) --> void", pybind11::arg("i0"), pybind11::arg("i1"));
		cl.def("copy", (void (easy3d::PropertyArray<bool>::*)(unsigned long, unsigned long)) &easy3d::PropertyArray<bool>::copy, "C++: easy3d::PropertyArray<bool>::copy(unsigned long, unsigned long) --> void", pybind11::arg("from"), pybind11::arg("to"));
		cl.def("clone", (class easy3d::BasePropertyArray * (easy3d::PropertyArray<bool>::*)() const) &easy3d::PropertyArray<bool>::clone, "C++: easy3d::PropertyArray<bool>::clone() const --> class easy3d::BasePropertyArray *", pybind11::return_value_policy::automatic);
		cl.def("empty_clone", (class easy3d::BasePropertyArray * (easy3d::PropertyArray<bool>::*)() const) &easy3d::PropertyArray<bool>::empty_clone, "C++: easy3d::PropertyArray<bool>::empty_clone() const --> class easy3d::BasePropertyArray *", pybind11::return_value_policy::automatic);
		cl.def("type", (const class std::type_info & (easy3d::PropertyArray<bool>::*)() const) &easy3d::PropertyArray<bool>::type, "C++: easy3d::PropertyArray<bool>::type() const --> const class std::type_info &", pybind11::return_value_policy::automatic);
		cl.def("data", (const bool * (easy3d::PropertyArray<bool>::*)() const) &easy3d::PropertyArray<bool>::data, "C++: easy3d::PropertyArray<bool>::data() const --> const bool *", pybind11::return_value_policy::automatic);
		cl.def("vector", (class std::vector<bool> & (easy3d::PropertyArray<bool>::*)()) &easy3d::PropertyArray<bool>::vector, "C++: easy3d::PropertyArray<bool>::vector() --> class std::vector<bool> &", pybind11::return_value_policy::automatic);
		cl.def("assign", (class easy3d::PropertyArray<bool> & (easy3d::PropertyArray<bool>::*)(const class easy3d::PropertyArray<bool> &)) &easy3d::PropertyArray<bool>::operator=, "C++: easy3d::PropertyArray<bool>::operator=(const class easy3d::PropertyArray<bool> &) --> class easy3d::PropertyArray<bool> &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		cl.def("reserve", (void (easy3d::BasePropertyArray::*)(unsigned long)) &easy3d::BasePropertyArray::reserve, "Reserve memory for n elements.\n\nC++: easy3d::BasePropertyArray::reserve(unsigned long) --> void", pybind11::arg("n"));
		cl.def("resize", (void (easy3d::BasePropertyArray::*)(unsigned long)) &easy3d::BasePropertyArray::resize, "Resize storage to hold n elements.\n\nC++: easy3d::BasePropertyArray::resize(unsigned long) --> void", pybind11::arg("n"));
		cl.def("shrink_to_fit", (void (easy3d::BasePropertyArray::*)()) &easy3d::BasePropertyArray::shrink_to_fit, "Free unused memory.\n\nC++: easy3d::BasePropertyArray::shrink_to_fit() --> void");
		cl.def("push_back", (void (easy3d::BasePropertyArray::*)()) &easy3d::BasePropertyArray::push_back, "Extend the number of elements by one.\n\nC++: easy3d::BasePropertyArray::push_back() --> void");
		cl.def("reset", (void (easy3d::BasePropertyArray::*)(unsigned long)) &easy3d::BasePropertyArray::reset, "Reset element to default value\n\nC++: easy3d::BasePropertyArray::reset(unsigned long) --> void", pybind11::arg("idx"));
		cl.def("transfer", (bool (easy3d::BasePropertyArray::*)(const class easy3d::BasePropertyArray &)) &easy3d::BasePropertyArray::transfer, "Copy the entire properties from \n\nC++: easy3d::BasePropertyArray::transfer(const class easy3d::BasePropertyArray &) --> bool", pybind11::arg("other"));
		cl.def("transfer", (bool (easy3d::BasePropertyArray::*)(const class easy3d::BasePropertyArray &, std::size_t, std::size_t)) &easy3d::BasePropertyArray::transfer, "Copy the property[from] of  to this->property[to].\n\nC++: easy3d::BasePropertyArray::transfer(const class easy3d::BasePropertyArray &, std::size_t, std::size_t) --> bool", pybind11::arg("other"), pybind11::arg("from"), pybind11::arg("to"));
		cl.def("swap", (void (easy3d::BasePropertyArray::*)(unsigned long, unsigned long)) &easy3d::BasePropertyArray::swap, "Let two elements swap their storage place.\n\nC++: easy3d::BasePropertyArray::swap(unsigned long, unsigned long) --> void", pybind11::arg("i0"), pybind11::arg("i1"));
		cl.def("copy", (void (easy3d::BasePropertyArray::*)(unsigned long, unsigned long)) &easy3d::BasePropertyArray::copy, "Let copy 'from' -> 'to'.\n\nC++: easy3d::BasePropertyArray::copy(unsigned long, unsigned long) --> void", pybind11::arg("from"), pybind11::arg("to"));
		cl.def("clone", (class easy3d::BasePropertyArray * (easy3d::BasePropertyArray::*)() const) &easy3d::BasePropertyArray::clone, "Return a deep copy of self.\n\nC++: easy3d::BasePropertyArray::clone() const --> class easy3d::BasePropertyArray *", pybind11::return_value_policy::automatic);
		cl.def("empty_clone", (class easy3d::BasePropertyArray * (easy3d::BasePropertyArray::*)() const) &easy3d::BasePropertyArray::empty_clone, "Return a empty copy of self.\n\nC++: easy3d::BasePropertyArray::empty_clone() const --> class easy3d::BasePropertyArray *", pybind11::return_value_policy::automatic);
		cl.def("type", (const class std::type_info & (easy3d::BasePropertyArray::*)() const) &easy3d::BasePropertyArray::type, "Return the type_info of the property\n\nC++: easy3d::BasePropertyArray::type() const --> const class std::type_info &", pybind11::return_value_policy::automatic);
		cl.def("name", (const std::string & (easy3d::BasePropertyArray::*)() const) &easy3d::BasePropertyArray::name, "Return the name of the property\n\nC++: easy3d::BasePropertyArray::name() const --> const std::string &", pybind11::return_value_policy::automatic);
		cl.def("set_name", (void (easy3d::BasePropertyArray::*)(const std::string &)) &easy3d::BasePropertyArray::set_name, "Set the name of the property\n\nC++: easy3d::BasePropertyArray::set_name(const std::string &) --> void", pybind11::arg("n"));
		cl.def("is_same", (bool (easy3d::BasePropertyArray::*)(const class easy3d::BasePropertyArray &) const) &easy3d::BasePropertyArray::is_same, "Test if two properties are the same.\n \n\n true only if their names and types are both identical.\n\nC++: easy3d::BasePropertyArray::is_same(const class easy3d::BasePropertyArray &) const --> bool", pybind11::arg("other"));
		cl.def("assign", (class easy3d::BasePropertyArray & (easy3d::BasePropertyArray::*)(const class easy3d::BasePropertyArray &)) &easy3d::BasePropertyArray::operator=, "C++: easy3d::BasePropertyArray::operator=(const class easy3d::BasePropertyArray &) --> class easy3d::BasePropertyArray &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}
}
