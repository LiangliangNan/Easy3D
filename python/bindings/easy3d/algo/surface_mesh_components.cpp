#include <easy3d/algo/surface_mesh_components.h>
#include <easy3d/core/box.h>
#include <easy3d/core/surface_mesh.h>
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

void bind_easy3d_algo_surface_mesh_components(pybind11::module_& m)
{
	{ // easy3d::SurfaceMeshComponent file:easy3d/algo/surface_mesh_components.h line:27
		pybind11::class_<easy3d::SurfaceMeshComponent, std::shared_ptr<easy3d::SurfaceMeshComponent>> cl(m, "SurfaceMeshComponent", "A connected component of a SurfaceMesh.\n \n\n Internally, A connected component of a SurfaceMesh stores four lists of SurfaceMesh elements, i.e., vertices,\n faces, edges, and halfedges.\n \n\n\n     ");
		cl.def( pybind11::init<class easy3d::SurfaceMesh *>(), pybind11::arg("mesh") );

		cl.def( pybind11::init( [](easy3d::SurfaceMeshComponent const &o){ return new easy3d::SurfaceMeshComponent(o); } ) );
		cl.def_static("extract", [](class easy3d::SurfaceMesh * a0) -> std::vector<class easy3d::SurfaceMeshComponent> { return easy3d::SurfaceMeshComponent::extract(a0); }, "", pybind11::arg("mesh"));
		cl.def_static("extract", (class std::vector<class easy3d::SurfaceMeshComponent> (*)(class easy3d::SurfaceMesh *, bool)) &easy3d::SurfaceMeshComponent::extract, "Extracts connected components. The components are sorted in descending order if  is  (in terms of number of faces.\n\nC++: easy3d::SurfaceMeshComponent::extract(class easy3d::SurfaceMesh *, bool) --> class std::vector<class easy3d::SurfaceMeshComponent>", pybind11::arg("mesh"), pybind11::arg("descending"));
		cl.def_static("extract", (class easy3d::SurfaceMeshComponent (*)(class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Face)) &easy3d::SurfaceMeshComponent::extract, "Extracts a single connected component from the seed face \n\nC++: easy3d::SurfaceMeshComponent::extract(class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Face) --> class easy3d::SurfaceMeshComponent", pybind11::arg("mesh"), pybind11::arg("seed"));
		cl.def_static("extract", (class easy3d::SurfaceMeshComponent (*)(class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Vertex)) &easy3d::SurfaceMeshComponent::extract, "Extracts a single connected component from the seed vertex \n\nC++: easy3d::SurfaceMeshComponent::extract(class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Vertex) --> class easy3d::SurfaceMeshComponent", pybind11::arg("mesh"), pybind11::arg("seed"));
		cl.def("faces", (const class std::vector<struct easy3d::SurfaceMesh::Face> & (easy3d::SurfaceMeshComponent::*)() const) &easy3d::SurfaceMeshComponent::faces, "C++: easy3d::SurfaceMeshComponent::faces() const --> const class std::vector<struct easy3d::SurfaceMesh::Face> &", pybind11::return_value_policy::automatic);
		cl.def("vertices", (const class std::vector<struct easy3d::SurfaceMesh::Vertex> & (easy3d::SurfaceMeshComponent::*)() const) &easy3d::SurfaceMeshComponent::vertices, "C++: easy3d::SurfaceMeshComponent::vertices() const --> const class std::vector<struct easy3d::SurfaceMesh::Vertex> &", pybind11::return_value_policy::automatic);
		cl.def("edges", (const class std::vector<struct easy3d::SurfaceMesh::Edge> & (easy3d::SurfaceMeshComponent::*)() const) &easy3d::SurfaceMeshComponent::edges, "C++: easy3d::SurfaceMeshComponent::edges() const --> const class std::vector<struct easy3d::SurfaceMesh::Edge> &", pybind11::return_value_policy::automatic);
		cl.def("halfedges", (const class std::vector<struct easy3d::SurfaceMesh::Halfedge> & (easy3d::SurfaceMeshComponent::*)() const) &easy3d::SurfaceMeshComponent::halfedges, "C++: easy3d::SurfaceMeshComponent::halfedges() const --> const class std::vector<struct easy3d::SurfaceMesh::Halfedge> &", pybind11::return_value_policy::automatic);
		cl.def("n_faces", (std::size_t (easy3d::SurfaceMeshComponent::*)() const) &easy3d::SurfaceMeshComponent::n_faces, "C++: easy3d::SurfaceMeshComponent::n_faces() const --> std::size_t");
		cl.def("n_vertices", (std::size_t (easy3d::SurfaceMeshComponent::*)() const) &easy3d::SurfaceMeshComponent::n_vertices, "C++: easy3d::SurfaceMeshComponent::n_vertices() const --> std::size_t");
		cl.def("n_edges", (std::size_t (easy3d::SurfaceMeshComponent::*)() const) &easy3d::SurfaceMeshComponent::n_edges, "C++: easy3d::SurfaceMeshComponent::n_edges() const --> std::size_t");
		cl.def("n_halfedges", (std::size_t (easy3d::SurfaceMeshComponent::*)() const) &easy3d::SurfaceMeshComponent::n_halfedges, "C++: easy3d::SurfaceMeshComponent::n_halfedges() const --> std::size_t");
		cl.def("mesh", (class easy3d::SurfaceMesh * (easy3d::SurfaceMeshComponent::*)() const) &easy3d::SurfaceMeshComponent::mesh, "Returns the surface mesh to which this component belongs. \n\nC++: easy3d::SurfaceMeshComponent::mesh() const --> class easy3d::SurfaceMesh *", pybind11::return_value_policy::automatic);
		cl.def("area", (float (easy3d::SurfaceMeshComponent::*)() const) &easy3d::SurfaceMeshComponent::area, "Returns the surface area of this component.\n Internally it triangulates the face using the tessellator. So this method also works for concave faces.\n\nC++: easy3d::SurfaceMeshComponent::area() const --> float");
		cl.def("border_length", (float (easy3d::SurfaceMeshComponent::*)() const) &easy3d::SurfaceMeshComponent::border_length, "Returns the total border length of this component. \n\nC++: easy3d::SurfaceMeshComponent::border_length() const --> float");
		cl.def("bbox", (class easy3d::GenericBox<3, float> (easy3d::SurfaceMeshComponent::*)() const) &easy3d::SurfaceMeshComponent::bbox, "Returns the bounding box of this component. \n\nC++: easy3d::SurfaceMeshComponent::bbox() const --> class easy3d::GenericBox<3, float>");
		cl.def("translate", (void (easy3d::SurfaceMeshComponent::*)(const class easy3d::Vec<3, float> &)) &easy3d::SurfaceMeshComponent::translate, "Translates this component by an offset vector. \n\nC++: easy3d::SurfaceMeshComponent::translate(const class easy3d::Vec<3, float> &) --> void", pybind11::arg("offset"));
		cl.def("to_mesh", (class easy3d::SurfaceMesh * (easy3d::SurfaceMeshComponent::*)() const) &easy3d::SurfaceMeshComponent::to_mesh, "Constructs a surface mesh from this component. \n\nC++: easy3d::SurfaceMeshComponent::to_mesh() const --> class easy3d::SurfaceMesh *", pybind11::return_value_policy::automatic);
		cl.def("assign", (class easy3d::SurfaceMeshComponent & (easy3d::SurfaceMeshComponent::*)(const class easy3d::SurfaceMeshComponent &)) &easy3d::SurfaceMeshComponent::operator=, "C++: easy3d::SurfaceMeshComponent::operator=(const class easy3d::SurfaceMeshComponent &) --> class easy3d::SurfaceMeshComponent &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}

}
