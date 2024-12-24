#include <easy3d/algo/surface_mesh_factory.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/vec.h>

#include <memory>

#include <pybind11/pybind11.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_algo_surface_mesh_factory(pybind11::module_& m)
{
	{ // easy3d::SurfaceMeshFactory file:easy3d/algo/surface_mesh_factory.h line:23
		pybind11::class_<easy3d::SurfaceMeshFactory, std::shared_ptr<easy3d::SurfaceMeshFactory>> cl(m, "SurfaceMeshFactory", "Factory class to generate different types of basic shapes.\n \n\n\n     ");
		cl.def( pybind11::init( [](){ return new easy3d::SurfaceMeshFactory(); } ) );
		cl.def_static("tetrahedron", (class easy3d::SurfaceMesh (*)()) &easy3d::SurfaceMeshFactory::tetrahedron, "Generate tetrahedron.\n\nC++: easy3d::SurfaceMeshFactory::tetrahedron() --> class easy3d::SurfaceMesh");
		cl.def_static("hexahedron", (class easy3d::SurfaceMesh (*)()) &easy3d::SurfaceMeshFactory::hexahedron, "Generate hexahedron.\n\nC++: easy3d::SurfaceMeshFactory::hexahedron() --> class easy3d::SurfaceMesh");
		cl.def_static("octahedron", (class easy3d::SurfaceMesh (*)()) &easy3d::SurfaceMeshFactory::octahedron, "Generate octahedron.\n\nC++: easy3d::SurfaceMeshFactory::octahedron() --> class easy3d::SurfaceMesh");
		cl.def_static("dodecahedron", (class easy3d::SurfaceMesh (*)()) &easy3d::SurfaceMeshFactory::dodecahedron, "Generate dodecahedron.\n\nC++: easy3d::SurfaceMeshFactory::dodecahedron() --> class easy3d::SurfaceMesh");
		cl.def_static("icosahedron", (class easy3d::SurfaceMesh (*)()) &easy3d::SurfaceMeshFactory::icosahedron, "Generate icosahedron.\n\nC++: easy3d::SurfaceMeshFactory::icosahedron() --> class easy3d::SurfaceMesh");
		cl.def_static("icosphere", []() -> easy3d::SurfaceMesh { return easy3d::SurfaceMeshFactory::icosphere(); }, "");
		cl.def_static("icosphere", (class easy3d::SurfaceMesh (*)(unsigned long)) &easy3d::SurfaceMeshFactory::icosphere, "Generate icosphere refined by  .\n \n\n Uses Loop subdivision to refine the initial icosahedron.\n \n\n SurfaceSubdivision\n\nC++: easy3d::SurfaceMeshFactory::icosphere(unsigned long) --> class easy3d::SurfaceMesh", pybind11::arg("n_subdivisions"));
		cl.def_static("quad_sphere", []() -> easy3d::SurfaceMesh { return easy3d::SurfaceMeshFactory::quad_sphere(); }, "");
		cl.def_static("quad_sphere", (class easy3d::SurfaceMesh (*)(unsigned long)) &easy3d::SurfaceMeshFactory::quad_sphere, "Generate quad sphere refined by  .\n \n\n Uses Catmull-Clark subdivision to refine the initial hexahedron.\n \n\n SurfaceMeshSubdivision\n\nC++: easy3d::SurfaceMeshFactory::quad_sphere(unsigned long) --> class easy3d::SurfaceMesh", pybind11::arg("n_subdivisions"));
		cl.def_static("uv_sphere", []() -> easy3d::SurfaceMesh { return easy3d::SurfaceMeshFactory::uv_sphere(); }, "");
		cl.def_static("uv_sphere", [](const class easy3d::Vec<3, float> & a0) -> easy3d::SurfaceMesh { return easy3d::SurfaceMeshFactory::uv_sphere(a0); }, "", pybind11::arg("center"));
		cl.def_static("uv_sphere", [](const class easy3d::Vec<3, float> & a0, float const & a1) -> easy3d::SurfaceMesh { return easy3d::SurfaceMeshFactory::uv_sphere(a0, a1); }, "", pybind11::arg("center"), pybind11::arg("radius"));
		cl.def_static("uv_sphere", [](const class easy3d::Vec<3, float> & a0, float const & a1, unsigned long const & a2) -> easy3d::SurfaceMesh { return easy3d::SurfaceMeshFactory::uv_sphere(a0, a1, a2); }, "", pybind11::arg("center"), pybind11::arg("radius"), pybind11::arg("n_slices"));
		cl.def_static("uv_sphere", (class easy3d::SurfaceMesh (*)(const class easy3d::Vec<3, float> &, float, unsigned long, unsigned long)) &easy3d::SurfaceMeshFactory::uv_sphere, "Generate UV sphere with given    and \n\nC++: easy3d::SurfaceMeshFactory::uv_sphere(const class easy3d::Vec<3, float> &, float, unsigned long, unsigned long) --> class easy3d::SurfaceMesh", pybind11::arg("center"), pybind11::arg("radius"), pybind11::arg("n_slices"), pybind11::arg("n_stacks"));
		cl.def_static("plane", []() -> easy3d::SurfaceMesh { return easy3d::SurfaceMeshFactory::plane(); }, "");
		cl.def_static("plane", (class easy3d::SurfaceMesh (*)(unsigned long)) &easy3d::SurfaceMeshFactory::plane, "Generate a plane mesh.\n \n\n Generates a pure quad mesh in the x-y plane with origin (0,0,0)\n and side length 1.\n \n\n Number of faces in each direction. Needs to be >= 1. Default: 4.\n\nC++: easy3d::SurfaceMeshFactory::plane(unsigned long) --> class easy3d::SurfaceMesh", pybind11::arg("resolution"));
		cl.def_static("cone", []() -> easy3d::SurfaceMesh { return easy3d::SurfaceMeshFactory::cone(); }, "");
		cl.def_static("cone", [](unsigned long const & a0) -> easy3d::SurfaceMesh { return easy3d::SurfaceMeshFactory::cone(a0); }, "", pybind11::arg("n_subdivisions"));
		cl.def_static("cone", [](unsigned long const & a0, float const & a1) -> easy3d::SurfaceMesh { return easy3d::SurfaceMeshFactory::cone(a0, a1); }, "", pybind11::arg("n_subdivisions"), pybind11::arg("radius"));
		cl.def_static("cone", (class easy3d::SurfaceMesh (*)(unsigned long, float, float)) &easy3d::SurfaceMeshFactory::cone, "Generate a cone mesh.\n \n\n Generates a polygonal mesh of a cone. The circular base lies in\n the x-y-plane and the tip points in positive z-direction.\n \n\n Number of subdivisions of the base circle. Needs to be >= 3. Default: 30.\n \n\n Radius of the base circle. Default: 1.\n \n\n Height of the the cone. Default: 2.5.\n\nC++: easy3d::SurfaceMeshFactory::cone(unsigned long, float, float) --> class easy3d::SurfaceMesh", pybind11::arg("n_subdivisions"), pybind11::arg("radius"), pybind11::arg("height"));
		cl.def_static("cylinder", []() -> easy3d::SurfaceMesh { return easy3d::SurfaceMeshFactory::cylinder(); }, "");
		cl.def_static("cylinder", [](unsigned long const & a0) -> easy3d::SurfaceMesh { return easy3d::SurfaceMeshFactory::cylinder(a0); }, "", pybind11::arg("n_subdivisions"));
		cl.def_static("cylinder", [](unsigned long const & a0, float const & a1) -> easy3d::SurfaceMesh { return easy3d::SurfaceMeshFactory::cylinder(a0, a1); }, "", pybind11::arg("n_subdivisions"), pybind11::arg("radius"));
		cl.def_static("cylinder", (class easy3d::SurfaceMesh (*)(unsigned long, float, float)) &easy3d::SurfaceMeshFactory::cylinder, "Generate a cylinder mesh.\n \n\n Generates a polygonal mesh of a cylinder. The cylinder is\n oriented in z-direction.\n \n\n Number of subdivisions of the cylinder. Needs to be >= 3. Default: 30.\n \n\n Radius of the cylinder. Default: 1.\n \n\n Height of the cylinder. Default: 2.5.\n\nC++: easy3d::SurfaceMeshFactory::cylinder(unsigned long, float, float) --> class easy3d::SurfaceMesh", pybind11::arg("n_subdivisions"), pybind11::arg("radius"), pybind11::arg("height"));
		cl.def_static("torus", []() -> easy3d::SurfaceMesh { return easy3d::SurfaceMeshFactory::torus(); }, "");
		cl.def_static("torus", [](unsigned long const & a0) -> easy3d::SurfaceMesh { return easy3d::SurfaceMeshFactory::torus(a0); }, "", pybind11::arg("radial_resolution"));
		cl.def_static("torus", [](unsigned long const & a0, unsigned long const & a1) -> easy3d::SurfaceMesh { return easy3d::SurfaceMeshFactory::torus(a0, a1); }, "", pybind11::arg("radial_resolution"), pybind11::arg("tubular_resolution"));
		cl.def_static("torus", [](unsigned long const & a0, unsigned long const & a1, float const & a2) -> easy3d::SurfaceMesh { return easy3d::SurfaceMeshFactory::torus(a0, a1, a2); }, "", pybind11::arg("radial_resolution"), pybind11::arg("tubular_resolution"), pybind11::arg("radius"));
		cl.def_static("torus", (class easy3d::SurfaceMesh (*)(unsigned long, unsigned long, float, float)) &easy3d::SurfaceMeshFactory::torus, "Generate a torus mesh.\n \n\n Generates a quad mesh of a torus with its major circle in the x-y plane.\n \n\n Number of subdivisions of the major circle. Needs to be >= 3. Default: 20.\n \n\n Number of subdivisions of along the tube. Needs to be >= 3. Default: 40.\n \n\n Radius of the major circle. Default: 1.\n \n\n Thickness of the tube. Default: 0.4.\n\nC++: easy3d::SurfaceMeshFactory::torus(unsigned long, unsigned long, float, float) --> class easy3d::SurfaceMesh", pybind11::arg("radial_resolution"), pybind11::arg("tubular_resolution"), pybind11::arg("radius"), pybind11::arg("thickness"));
	}

}
