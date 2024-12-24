#include <easy3d/algo/point_cloud_ransac.h>

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


void bind_easy3d_algo_point_cloud_ransac(pybind11::module_& m)
{
	{ // easy3d::PrimitivesRansac file:easy3d/algo/point_cloud_ransac.h line:52
		pybind11::class_<easy3d::PrimitivesRansac, std::shared_ptr<easy3d::PrimitivesRansac>> cl(m, "PrimitivesRansac", "Extract primitives from point clouds using RANSAC.\n \n\n\n Usage example:\n  \n\n\n\n\n\n     ");
		cl.def( pybind11::init( [](){ return new easy3d::PrimitivesRansac(); } ) );
		cl.def( pybind11::init( [](easy3d::PrimitivesRansac const &o){ return new easy3d::PrimitivesRansac(o); } ) );

		pybind11::enum_<easy3d::PrimitivesRansac::PrimType>(cl, "PrimType", pybind11::arithmetic(), "")
			.value("PLANE", easy3d::PrimitivesRansac::PLANE)
			.value("SPHERE", easy3d::PrimitivesRansac::SPHERE)
			.value("CYLINDER", easy3d::PrimitivesRansac::CYLINDER)
			.value("CONE", easy3d::PrimitivesRansac::CONE)
			.value("TORUS", easy3d::PrimitivesRansac::TORUS)
			.value("UNKNOWN", easy3d::PrimitivesRansac::UNKNOWN)
			.export_values();

		cl.def("add_primitive_type", (void (easy3d::PrimitivesRansac::*)(enum easy3d::PrimitivesRansac::PrimType)) &easy3d::PrimitivesRansac::add_primitive_type, "Setup the primitive types to be extracted.\n \n\n This is done by adding the interested primitive types one by one.\n\nC++: easy3d::PrimitivesRansac::add_primitive_type(enum easy3d::PrimitivesRansac::PrimType) --> void", pybind11::arg("t"));
		cl.def("remove_primitive_type", (void (easy3d::PrimitivesRansac::*)(enum easy3d::PrimitivesRansac::PrimType)) &easy3d::PrimitivesRansac::remove_primitive_type, "Exclude a primitive types to be extracted.\n \n\n This is done by removing the primitive type from the existing list.\n\nC++: easy3d::PrimitivesRansac::remove_primitive_type(enum easy3d::PrimitivesRansac::PrimType) --> void", pybind11::arg("t"));
		cl.def("get_planes", (const class std::vector<struct easy3d::PrimitivesRansac::PlanePrim> & (easy3d::PrimitivesRansac::*)() const) &easy3d::PrimitivesRansac::get_planes, "C++: easy3d::PrimitivesRansac::get_planes() const --> const class std::vector<struct easy3d::PrimitivesRansac::PlanePrim> &", pybind11::return_value_policy::automatic);
		cl.def("get_cylinders", (const class std::vector<struct easy3d::PrimitivesRansac::CylinderPrim> & (easy3d::PrimitivesRansac::*)() const) &easy3d::PrimitivesRansac::get_cylinders, "C++: easy3d::PrimitivesRansac::get_cylinders() const --> const class std::vector<struct easy3d::PrimitivesRansac::CylinderPrim> &", pybind11::return_value_policy::automatic);
		cl.def("assign", (class easy3d::PrimitivesRansac & (easy3d::PrimitivesRansac::*)(const class easy3d::PrimitivesRansac &)) &easy3d::PrimitivesRansac::operator=, "C++: easy3d::PrimitivesRansac::operator=(const class easy3d::PrimitivesRansac &) --> class easy3d::PrimitivesRansac &", pybind11::return_value_policy::automatic, pybind11::arg(""));

		{ // easy3d::PrimitivesRansac::PlanePrim file:easy3d/algo/point_cloud_ransac.h line:126
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::PrimitivesRansac::PlanePrim, std::shared_ptr<easy3d::PrimitivesRansac::PlanePrim>> cl(enclosing_class, "PlanePrim", "");
			cl.def( pybind11::init( [](){ return new easy3d::PrimitivesRansac::PlanePrim(); } ) );
			cl.def( pybind11::init( [](easy3d::PrimitivesRansac::PlanePrim const &o){ return new easy3d::PrimitivesRansac::PlanePrim(o); } ) );
			cl.def_readwrite("primitive_index", &easy3d::PrimitivesRansac::PlanePrim::primitive_index);
			cl.def_readwrite("vertices", &easy3d::PrimitivesRansac::PlanePrim::vertices);
			cl.def_readwrite("plane", &easy3d::PrimitivesRansac::PlanePrim::plane);
			cl.def_readwrite("position", &easy3d::PrimitivesRansac::PlanePrim::position);
			cl.def_readwrite("normal", &easy3d::PrimitivesRansac::PlanePrim::normal);
			cl.def("assign", (struct easy3d::PrimitivesRansac::PlanePrim & (easy3d::PrimitivesRansac::PlanePrim::*)(const struct easy3d::PrimitivesRansac::PlanePrim &)) &easy3d::PrimitivesRansac::PlanePrim::operator=, "C++: easy3d::PrimitivesRansac::PlanePrim::operator=(const struct easy3d::PrimitivesRansac::PlanePrim &) --> struct easy3d::PrimitivesRansac::PlanePrim &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}

		{ // easy3d::PrimitivesRansac::CylinderPrim file:easy3d/algo/point_cloud_ransac.h line:135
			auto & enclosing_class = cl;
			pybind11::class_<easy3d::PrimitivesRansac::CylinderPrim, std::shared_ptr<easy3d::PrimitivesRansac::CylinderPrim>> cl(enclosing_class, "CylinderPrim", "");
			cl.def( pybind11::init( [](){ return new easy3d::PrimitivesRansac::CylinderPrim(); } ) );
			cl.def( pybind11::init( [](easy3d::PrimitivesRansac::CylinderPrim const &o){ return new easy3d::PrimitivesRansac::CylinderPrim(o); } ) );
			cl.def_readwrite("primitive_index", &easy3d::PrimitivesRansac::CylinderPrim::primitive_index);
			cl.def_readwrite("vertices", &easy3d::PrimitivesRansac::CylinderPrim::vertices);
			cl.def_readwrite("radius", &easy3d::PrimitivesRansac::CylinderPrim::radius);
			cl.def_readwrite("position", &easy3d::PrimitivesRansac::CylinderPrim::position);
			cl.def_readwrite("direction", &easy3d::PrimitivesRansac::CylinderPrim::direction);
			cl.def("assign", (struct easy3d::PrimitivesRansac::CylinderPrim & (easy3d::PrimitivesRansac::CylinderPrim::*)(const struct easy3d::PrimitivesRansac::CylinderPrim &)) &easy3d::PrimitivesRansac::CylinderPrim::operator=, "C++: easy3d::PrimitivesRansac::CylinderPrim::operator=(const struct easy3d::PrimitivesRansac::CylinderPrim &) --> struct easy3d::PrimitivesRansac::CylinderPrim &", pybind11::return_value_policy::automatic, pybind11::arg(""));
		}
	}
}
