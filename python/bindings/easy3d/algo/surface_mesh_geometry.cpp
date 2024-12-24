#include <easy3d/algo/surface_mesh_geometry.h>
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

void bind_easy3d_algo_surface_mesh_geometry(pybind11::module_& m)
{
	// easy3d::geom::triangle_area(const class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Face) file:easy3d/algo/surface_mesh_geometry.h line:25
	m.def("triangle_area", (float (*)(const class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Face)) &easy3d::geom::triangle_area, "compute area of triangle f    \n\nC++: easy3d::geom::triangle_area(const class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Face) --> float", pybind11::arg("mesh"), pybind11::arg("f"));

	// easy3d::geom::surface_area(const class easy3d::SurfaceMesh *) file:easy3d/algo/surface_mesh_geometry.h line:28
	m.def("surface_area", (float (*)(const class easy3d::SurfaceMesh *)) &easy3d::geom::surface_area, "surface area of the mesh (assumes triangular faces)    \n\nC++: easy3d::geom::surface_area(const class easy3d::SurfaceMesh *) --> float", pybind11::arg("mesh"));

	// easy3d::geom::volume(const class easy3d::SurfaceMesh *) file:easy3d/algo/surface_mesh_geometry.h line:33
	m.def("volume", (float (*)(const class easy3d::SurfaceMesh *)) &easy3d::geom::volume, "Compute the volume of a mesh\n \n\n See  for details.\n \n\n Input mesh needs to be a pure triangle mesh.\n\nC++: easy3d::geom::volume(const class easy3d::SurfaceMesh *) --> float", pybind11::arg("mesh"));

	// easy3d::geom::centroid(const class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Face) file:easy3d/algo/surface_mesh_geometry.h line:36
	m.def("centroid", (class easy3d::Vec<3, float> (*)(const class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Face)) &easy3d::geom::centroid, "barycenter/centroid of a face    \n\nC++: easy3d::geom::centroid(const class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Face) --> class easy3d::Vec<3, float>", pybind11::arg("mesh"), pybind11::arg("f"));

	// easy3d::geom::centroid(const class easy3d::SurfaceMesh *) file:easy3d/algo/surface_mesh_geometry.h line:40
	m.def("centroid", (class easy3d::Vec<3, float> (*)(const class easy3d::SurfaceMesh *)) &easy3d::geom::centroid, "barycenter/centroid of mesh, computed as area-weighted mean of vertices.    \n\n \n assumes triangular faces.    \n\nC++: easy3d::geom::centroid(const class easy3d::SurfaceMesh *) --> class easy3d::Vec<3, float>", pybind11::arg("mesh"));

	// easy3d::geom::dual(class easy3d::SurfaceMesh *) file:easy3d/algo/surface_mesh_geometry.h line:44
	m.def("dual", (void (*)(class easy3d::SurfaceMesh *)) &easy3d::geom::dual, "Compute dual of a mesh.\n \n\n Changes the mesh in place. All properties are cleared.\n\nC++: easy3d::geom::dual(class easy3d::SurfaceMesh *) --> void", pybind11::arg("mesh"));

	// easy3d::geom::cotan_weight(const class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Edge) file:easy3d/algo/surface_mesh_geometry.h line:47
	m.def("cotan_weight", (double (*)(const class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Edge)) &easy3d::geom::cotan_weight, "compute the cotangent weight for edge e    \n\nC++: easy3d::geom::cotan_weight(const class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Edge) --> double", pybind11::arg("mesh"), pybind11::arg("e"));

	// easy3d::geom::voronoi_area(const class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Vertex) file:easy3d/algo/surface_mesh_geometry.h line:50
	m.def("voronoi_area", (double (*)(const class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Vertex)) &easy3d::geom::voronoi_area, "compute (mixed) Voronoi area of vertex v    \n\nC++: easy3d::geom::voronoi_area(const class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Vertex) --> double", pybind11::arg("mesh"), pybind11::arg("v"));

	// easy3d::geom::voronoi_area_barycentric(const class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Vertex) file:easy3d/algo/surface_mesh_geometry.h line:53
	m.def("voronoi_area_barycentric", (double (*)(const class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Vertex)) &easy3d::geom::voronoi_area_barycentric, "compute barycentric Voronoi area of vertex v    \n\nC++: easy3d::geom::voronoi_area_barycentric(const class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Vertex) --> double", pybind11::arg("mesh"), pybind11::arg("v"));

	// easy3d::geom::laplace(const class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Vertex) file:easy3d/algo/surface_mesh_geometry.h line:56
	m.def("laplace", (class easy3d::Vec<3, float> (*)(const class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Vertex)) &easy3d::geom::laplace, "compute Laplace vector for vertex v (normalized by Voronoi area)    \n\nC++: easy3d::geom::laplace(const class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Vertex) --> class easy3d::Vec<3, float>", pybind11::arg("mesh"), pybind11::arg("v"));

	// easy3d::geom::angle_sum(const class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Vertex) file:easy3d/algo/surface_mesh_geometry.h line:59
	m.def("angle_sum", (double (*)(const class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Vertex)) &easy3d::geom::angle_sum, "compute the sum of angles around vertex v (used for Gaussian curvature)    \n\nC++: easy3d::geom::angle_sum(const class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Vertex) --> double", pybind11::arg("mesh"), pybind11::arg("v"));

	{ // easy3d::geom::VertexCurvature file:easy3d/algo/surface_mesh_geometry.h line:62
		pybind11::class_<easy3d::geom::VertexCurvature, std::shared_ptr<easy3d::geom::VertexCurvature>> cl(m, "VertexCurvature", "discrete curvature information for a vertex. used for vertex_curvature()    ");
		cl.def( pybind11::init( [](){ return new easy3d::geom::VertexCurvature(); } ) );
		cl.def_readwrite("mean", &easy3d::geom::VertexCurvature::mean);
		cl.def_readwrite("gauss", &easy3d::geom::VertexCurvature::gauss);
		cl.def_readwrite("max", &easy3d::geom::VertexCurvature::max);
		cl.def_readwrite("min", &easy3d::geom::VertexCurvature::min);
	}
	// easy3d::geom::vertex_curvature(const class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Vertex) file:easy3d/algo/surface_mesh_geometry.h line:73
	m.def("vertex_curvature", (struct easy3d::geom::VertexCurvature (*)(const class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Vertex)) &easy3d::geom::vertex_curvature, "compute min, max, mean, and Gaussian curvature for vertex v. \n\n \n This will not give reliable values for boundary vertices.    \n\nC++: easy3d::geom::vertex_curvature(const class easy3d::SurfaceMesh *, struct easy3d::SurfaceMesh::Vertex) --> struct easy3d::geom::VertexCurvature", pybind11::arg("mesh"), pybind11::arg("v"));

}
