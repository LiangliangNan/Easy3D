#include <easy3d/algo/extrusion.h>
#include <easy3d/core/polygon.h>
#include <easy3d/core/surface_mesh.h>

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


void bind_easy3d_algo_extrusion(pybind11::module_& m)
{
	// easy3d::extrude(const class std::vector<class easy3d::GenericPolygon<float> > &, float) file:easy3d/algo/extrusion.h line:49
	m.def("extrude", (class easy3d::SurfaceMesh * (*)(const class std::vector<class easy3d::GenericPolygon<float> > &, float)) &easy3d::extrude, "Extrudes a 3D surface mesh from a set of simple contours.\n \n\n The input contours, which must be simple, i.e.,\n      - free of intersections,\n      - CCW contours defining the outer boundary and CW contours defining holes.\n      Simple contours can be obtained using the tessellator.\n \n\n The height (in the Z direction) of the extruded 3D model.\n \n\n The extruded surface mesh model. NULL on failure.\n\nC++: easy3d::extrude(const class std::vector<class easy3d::GenericPolygon<float> > &, float) --> class easy3d::SurfaceMesh *", pybind11::return_value_policy::automatic, pybind11::arg("contours"), pybind11::arg("height"));

	// easy3d::extrude(class easy3d::SurfaceMesh *, const class std::vector<class easy3d::GenericPolygon<float> > &, float) file:easy3d/algo/extrusion.h line:61
	m.def("extrude", (bool (*)(class easy3d::SurfaceMesh *, const class std::vector<class easy3d::GenericPolygon<float> > &, float)) &easy3d::extrude, "Extrudes a 3D surface mesh from a set of simple contours.\n \n\n The output mesh model. Must be allocated before hand.\n \n\n The input contours, which must be simple, i.e.,\n      - free of intersections,\n      - CCW contours defining the outer boundary and CW contours defining holes.\n      Simple contours can be obtained using the tessellator.\n \n\n The height (in the Z direction) of the extruded 3D model.\n \n\n True on success and false on failure.\n\nC++: easy3d::extrude(class easy3d::SurfaceMesh *, const class std::vector<class easy3d::GenericPolygon<float> > &, float) --> bool", pybind11::arg("mesh"), pybind11::arg("contours"), pybind11::arg("height"));
}
