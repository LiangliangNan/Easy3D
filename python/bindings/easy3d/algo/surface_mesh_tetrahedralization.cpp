#include <easy3d/algo/surface_mesh_tetrahedralization.h>

#include <memory>

#include <pybind11/pybind11.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_algo_surface_mesh_tetrahedralization(pybind11::module_& m)
{
	{ // easy3d::SurfaceMeshTetrehedralization file:easy3d/algo/surface_mesh_tetrahedralization.h line:26
		pybind11::class_<easy3d::SurfaceMeshTetrehedralization, std::shared_ptr<easy3d::SurfaceMeshTetrehedralization>> cl(m, "SurfaceMeshTetrehedralization", "Generate quality tetrahedralization from closed shells.\n \n");
		cl.def( pybind11::init( [](){ return new easy3d::SurfaceMeshTetrehedralization(); } ) );
		cl.def( pybind11::init( [](easy3d::SurfaceMeshTetrehedralization const &o){ return new easy3d::SurfaceMeshTetrehedralization(o); } ) );
		cl.def("set_allow_steiner_points_on_boundary", (void (easy3d::SurfaceMeshTetrehedralization::*)(bool)) &easy3d::SurfaceMeshTetrehedralization::set_allow_steiner_points_on_boundary, "Sets if Steiner points are allowed on the boundary edges and faces of the input surface. Default is true.\n \n\n Disable this will preserve the input boundary edges and faces. In this case, Steiner points (if\n      there exists any) will appear only in the interior space of the input surface.\n\nC++: easy3d::SurfaceMeshTetrehedralization::set_allow_steiner_points_on_boundary(bool) --> void", pybind11::arg("x"));
		cl.def("set_max_tet_shape", (void (easy3d::SurfaceMeshTetrehedralization::*)(double)) &easy3d::SurfaceMeshTetrehedralization::set_max_tet_shape, "Sets the maximum allowable radius-edge ratio. Default value is 2.0.\n The value controls how new points can be added to improve the mesh quality.\n More complicated constraints can be set by using set_command_line(). See the \"-q\" switch in tetgen manual.\n http://wias-berlin.de/software/tetgen/1.5/doc/manual/manual005.html#cmd-q\n \n\n set_max_dihedral_angle()\n\nC++: easy3d::SurfaceMeshTetrehedralization::set_max_tet_shape(double) --> void", pybind11::arg("x"));
		cl.def("set_min_dihedral_angle", (void (easy3d::SurfaceMeshTetrehedralization::*)(double)) &easy3d::SurfaceMeshTetrehedralization::set_min_dihedral_angle, "Sets the minimum allowable dihedral angle. Default value is 0.\n The value controls how new points can be added to improve the mesh quality.\n More complicated constraints can be set by using set_command_line(). See the \"-q\" switch in tetgen manual.\n http://wias-berlin.de/software/tetgen/1.5/doc/manual/manual005.html#cmd-q\n \n\n set_max_tet_shape()\n\nC++: easy3d::SurfaceMeshTetrehedralization::set_min_dihedral_angle(double) --> void", pybind11::arg("x"));
		cl.def("set_max_tet_volume", (void (easy3d::SurfaceMeshTetrehedralization::*)(double)) &easy3d::SurfaceMeshTetrehedralization::set_max_tet_volume, "Sets the maximum volume constraint on all tetrahedra. Default value is -1 (no max volume constraint).\n \n\n A positive value indicates that no tetrahedra is generated whose volume is larger than this value.\n \n\n A negative value indicates no such a constraint.\n More complicated constraints can be set by using set_command_line(). See the \"-a\" switch in tetgen manual.\n http://wias-berlin.de/software/tetgen/1.5/doc/manual/manual005.html#cmd-a\n\nC++: easy3d::SurfaceMeshTetrehedralization::set_max_tet_volume(double) --> void", pybind11::arg("x"));
		cl.def("set_tag_regions", (void (easy3d::SurfaceMeshTetrehedralization::*)(bool)) &easy3d::SurfaceMeshTetrehedralization::set_tag_regions, "If enabled, assigns an additional attribute (an integer number) to each tetrahedron that identifies to what\n facet-bounded region it belongs. In the output mesh, all tetrahedra in the same region will get a\n corresponding non-zero attribute.\n\nC++: easy3d::SurfaceMeshTetrehedralization::set_tag_regions(bool) --> void", pybind11::arg("x"));
		cl.def("set_command_line", (void (easy3d::SurfaceMeshTetrehedralization::*)(const std::string &)) &easy3d::SurfaceMeshTetrehedralization::set_command_line, "If specified, overrides all other options. \n\nC++: easy3d::SurfaceMeshTetrehedralization::set_command_line(const std::string &) --> void", pybind11::arg("x"));
		cl.def("assign", (class easy3d::SurfaceMeshTetrehedralization & (easy3d::SurfaceMeshTetrehedralization::*)(const class easy3d::SurfaceMeshTetrehedralization &)) &easy3d::SurfaceMeshTetrehedralization::operator=, "C++: easy3d::SurfaceMeshTetrehedralization::operator=(const class easy3d::SurfaceMeshTetrehedralization &) --> class easy3d::SurfaceMeshTetrehedralization &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}
}
