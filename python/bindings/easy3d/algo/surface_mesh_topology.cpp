#include <easy3d/algo/surface_mesh_topology.h>

#include <memory>

#include <pybind11/pybind11.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_algo_surface_mesh_topology(pybind11::module_& m)
{
	{ // easy3d::SurfaceMeshTopology file:easy3d/algo/surface_mesh_topology.h line:23
		pybind11::class_<easy3d::SurfaceMeshTopology, std::shared_ptr<easy3d::SurfaceMeshTopology>> cl(m, "SurfaceMeshTopology", "Compute various topological characteristics of a surface mesh component.\n \n");
		cl.def("euler_poincare", (int (easy3d::SurfaceMeshTopology::*)() const) &easy3d::SurfaceMeshTopology::euler_poincare, "returns the Euler-Poincare characteristic,\n   1: a disc\n   2: a sphere\n\nC++: easy3d::SurfaceMeshTopology::euler_poincare() const --> int");
		cl.def("number_of_borders", (std::size_t (easy3d::SurfaceMeshTopology::*)() const) &easy3d::SurfaceMeshTopology::number_of_borders, "returns 0 for a closed surface. \n\nC++: easy3d::SurfaceMeshTopology::number_of_borders() const --> std::size_t");
		cl.def("largest_border_size", (std::size_t (easy3d::SurfaceMeshTopology::*)() const) &easy3d::SurfaceMeshTopology::largest_border_size, "returns the number of edges in the largest border. \n\nC++: easy3d::SurfaceMeshTopology::largest_border_size() const --> std::size_t");
		cl.def("is_closed", (bool (easy3d::SurfaceMeshTopology::*)() const) &easy3d::SurfaceMeshTopology::is_closed, "returns if the surface is closed. \n\nC++: easy3d::SurfaceMeshTopology::is_closed() const --> bool");
		cl.def("is_sphere", (bool (easy3d::SurfaceMeshTopology::*)() const) &easy3d::SurfaceMeshTopology::is_sphere, "returns if the surface is topologically equivalent to a sphere. \n\nC++: easy3d::SurfaceMeshTopology::is_sphere() const --> bool");
		cl.def("is_disc", (bool (easy3d::SurfaceMeshTopology::*)() const) &easy3d::SurfaceMeshTopology::is_disc, "returns if the surface is topologically equivalent to a disk. \n\nC++: easy3d::SurfaceMeshTopology::is_disc() const --> bool");
		cl.def("is_cylinder", (bool (easy3d::SurfaceMeshTopology::*)() const) &easy3d::SurfaceMeshTopology::is_cylinder, "returns if the surface is topologically equivalent to a cylinder. \n\nC++: easy3d::SurfaceMeshTopology::is_cylinder() const --> bool");
		cl.def("is_torus", (bool (easy3d::SurfaceMeshTopology::*)() const) &easy3d::SurfaceMeshTopology::is_torus, "returns if the surface is topologically equivalent to a torus. \n\nC++: easy3d::SurfaceMeshTopology::is_torus() const --> bool");
	}

}
