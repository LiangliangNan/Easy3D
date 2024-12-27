#include <easy3d/algo/surface_mesh_hole_filling.h>
#include <easy3d/core/surface_mesh.h>

#include <memory>

#include <pybind11/pybind11.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_algo_surface_mesh_hole_filling(pybind11::module_& m)
{
	{ // easy3d::SurfaceMeshHoleFilling file:easy3d/algo/surface_mesh_hole_filling.h line:31
		pybind11::class_<easy3d::SurfaceMeshHoleFilling, std::shared_ptr<easy3d::SurfaceMeshHoleFilling>> cl(m, "SurfaceMeshHoleFilling", "This class closes simple holes in a surface mesh.\n \n\n\n \n It closes simple holes (boundary loops of manifold vertices) by first filling the hole with an\n angle/area-minimizing triangulation, followed by isometric remeshing, and finished by curvature-minimizing\n fairing of the filled-in patch. See the following paper for more details:\n  - Peter Liepa. Filling holes in meshes. SGP, pages 200–205, 2003.");
		cl.def( pybind11::init<class easy3d::SurfaceMesh *>(), pybind11::arg("mesh") );

        cl.def(pybind11::init(
                [](easy3d::SurfaceMeshHoleFilling const &o) { return new easy3d::SurfaceMeshHoleFilling(o); }));

        cl.def("fill_hole", (bool (easy3d::SurfaceMeshHoleFilling::*)(
                       struct easy3d::SurfaceMesh::Halfedge)) &easy3d::SurfaceMeshHoleFilling::fill_hole,
               "fill the hole specified by halfedge h\n\nC++: easy3d::SurfaceMeshHoleFilling::fill_hole(struct easy3d::SurfaceMesh::Halfedge) --> bool",
               pybind11::arg("h"));

        cl.def("fill_holes", (int (easy3d::SurfaceMeshHoleFilling::*)(int)) &easy3d::SurfaceMeshHoleFilling::fill_holes,
               "fill all holes with size smaller than specified size. The size of a hole is defined as the number of boundary edges in the boundary loop of the hole.\n\nC++: easy3d::SurfaceMeshHoleFilling::fill_holes(int size = 500) --> int",
               pybind11::arg("size"));

        cl.def("assign", (class easy3d::SurfaceMeshHoleFilling & (easy3d::SurfaceMeshHoleFilling::*)(const class easy3d::SurfaceMeshHoleFilling &)) &easy3d::SurfaceMeshHoleFilling::operator=, "C++: easy3d::SurfaceMeshHoleFilling::operator=(const class easy3d::SurfaceMeshHoleFilling &) --> class easy3d::SurfaceMeshHoleFilling &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}

}
