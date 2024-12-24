#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/surface_mesh_builder.h>
#include <easy3d/core/vec.h>

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

void bind_easy3d_core_surface_mesh_builder(pybind11::module_& m)
{
	{ // easy3d::SurfaceMeshBuilder file:easy3d/core/surface_mesh_builder.h line:56
		pybind11::class_<easy3d::SurfaceMeshBuilder, std::shared_ptr<easy3d::SurfaceMeshBuilder>> cl(m, "SurfaceMeshBuilder", "A helper class for constructing manifold surface mesh models.\n \n\n\n \n SurfaceMeshBuilder resolves non-manifoldness while building a surface mesh.\n      It is typically used to load a model from a file (because you don't know if the mesh is manifold or not).\n      For meshes guaranteed to be manifold, you can also use the built-in add_vertex() and\n      add_[face/triangle/quad]() functions of SurfaceMesh for their construction.\n Example use:\n \n\n\n\n\n\n\n\n\n\n     ");
		cl.def( pybind11::init<class easy3d::SurfaceMesh *>(), pybind11::arg("mesh") );

		cl.def( pybind11::init( [](easy3d::SurfaceMeshBuilder const &o){ return new easy3d::SurfaceMeshBuilder(o); } ) );
		cl.def("begin_surface", (void (easy3d::SurfaceMeshBuilder::*)()) &easy3d::SurfaceMeshBuilder::begin_surface, "Begin surface construction. Must be called at the beginning of the surface construction and used in\n        pair with end_surface() at the end of surface mesh construction.\n \n\n\n        \n\nC++: easy3d::SurfaceMeshBuilder::begin_surface() --> void");
		cl.def("add_vertex", (struct easy3d::SurfaceMesh::Vertex (easy3d::SurfaceMeshBuilder::*)(const class easy3d::Vec<3, float> &)) &easy3d::SurfaceMeshBuilder::add_vertex, "Add a vertex to the mesh.\n \n\n The 3D coordinates of the vertex.\n \n\n The added vertex on success.\n\nC++: easy3d::SurfaceMeshBuilder::add_vertex(const class easy3d::Vec<3, float> &) --> struct easy3d::SurfaceMesh::Vertex", pybind11::arg("p"));
		cl.def("add_face", (struct easy3d::SurfaceMesh::Face (easy3d::SurfaceMeshBuilder::*)(const class std::vector<struct easy3d::SurfaceMesh::Vertex> &)) &easy3d::SurfaceMeshBuilder::add_face, "Add a face to the mesh.\n \n\n The vertices of the face.\n \n\n The added face on success.\n \n\n\n         \n\nC++: easy3d::SurfaceMeshBuilder::add_face(const class std::vector<struct easy3d::SurfaceMesh::Vertex> &) --> struct easy3d::SurfaceMesh::Face", pybind11::arg("vertices"));
		cl.def("add_triangle", (struct easy3d::SurfaceMesh::Face (easy3d::SurfaceMeshBuilder::*)(struct easy3d::SurfaceMesh::Vertex, struct easy3d::SurfaceMesh::Vertex, struct easy3d::SurfaceMesh::Vertex)) &easy3d::SurfaceMeshBuilder::add_triangle, "Add a new triangle face connecting vertices v1, v2, and v3.\n \n\n The added face on success.\n \n\n\n         \n\nC++: easy3d::SurfaceMeshBuilder::add_triangle(struct easy3d::SurfaceMesh::Vertex, struct easy3d::SurfaceMesh::Vertex, struct easy3d::SurfaceMesh::Vertex) --> struct easy3d::SurfaceMesh::Face", pybind11::arg("v1"), pybind11::arg("v2"), pybind11::arg("v3"));
		cl.def("add_quad", (struct easy3d::SurfaceMesh::Face (easy3d::SurfaceMeshBuilder::*)(struct easy3d::SurfaceMesh::Vertex, struct easy3d::SurfaceMesh::Vertex, struct easy3d::SurfaceMesh::Vertex, struct easy3d::SurfaceMesh::Vertex)) &easy3d::SurfaceMeshBuilder::add_quad, "Add a new quad face connecting vertices v1, v2, v3, and v4.\n \n\n The added face on success.\n \n\n\n         \n\nC++: easy3d::SurfaceMeshBuilder::add_quad(struct easy3d::SurfaceMesh::Vertex, struct easy3d::SurfaceMesh::Vertex, struct easy3d::SurfaceMesh::Vertex, struct easy3d::SurfaceMesh::Vertex) --> struct easy3d::SurfaceMesh::Face", pybind11::arg("v1"), pybind11::arg("v2"), pybind11::arg("v3"), pybind11::arg("v4"));
		cl.def("end_surface", [](easy3d::SurfaceMeshBuilder &o) -> void { return o.end_surface(); }, "");
		cl.def("end_surface", (void (easy3d::SurfaceMeshBuilder::*)(bool)) &easy3d::SurfaceMeshBuilder::end_surface, "Finalize surface construction. Must be called at the end of the surface construction and used in\n        pair with begin_surface() at the beginning of surface mesh construction.\n \n\n True to log the issues detected and a report on the process of the issues to the log file.\n \n\n\n         \n\nC++: easy3d::SurfaceMeshBuilder::end_surface(bool) --> void", pybind11::arg("log_issues"));
		cl.def("face_vertices", (const class std::vector<struct easy3d::SurfaceMesh::Vertex> & (easy3d::SurfaceMeshBuilder::*)() const) &easy3d::SurfaceMeshBuilder::face_vertices, "The actual vertices of the previously added face. The order of the vertices are the same as those\n        provided to add_[face/triangle/quad]() for the construction of the face.\n \n\n The result is valid if the face was successfully added, and it will remain valid until the next\n            call to add_[face/triangle/quad]() and end_surface().\n\nC++: easy3d::SurfaceMeshBuilder::face_vertices() const --> const class std::vector<struct easy3d::SurfaceMesh::Vertex> &", pybind11::return_value_policy::automatic);
		cl.def("assign", (class easy3d::SurfaceMeshBuilder & (easy3d::SurfaceMeshBuilder::*)(const class easy3d::SurfaceMeshBuilder &)) &easy3d::SurfaceMeshBuilder::operator=, "C++: easy3d::SurfaceMeshBuilder::operator=(const class easy3d::SurfaceMeshBuilder &) --> class easy3d::SurfaceMeshBuilder &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}
}
