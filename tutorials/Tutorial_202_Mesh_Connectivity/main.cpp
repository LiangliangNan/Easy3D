/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work,
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan.
*           Easy3D: a lightweight, easy-to-use, and efficient C++
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
*
*	Easy3D is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Easy3D is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include <easy3d/core/surface_mesh.h>

using namespace easy3d;


// This example shows how to
//		- access the incident vertices of each vertex
//		- access the incident faces of each vertex
//		- access the incident vertices of each face
//		- access the incident half-edges of each face
//		- access the two end points of each edge;
//		- access the two faces connected by each edge



SurfaceMesh* old_mesh_from_previous_example() {
	// Create a surface mesh
	SurfaceMesh* mesh = new SurfaceMesh;

	// Add 4 vertices
	SurfaceMesh::Vertex v0 = mesh->add_vertex(vec3(0, 0, 0));
	SurfaceMesh::Vertex v1 = mesh->add_vertex(vec3(1, 0, 0));
	SurfaceMesh::Vertex v2 = mesh->add_vertex(vec3(0, 1, 0));
	SurfaceMesh::Vertex v3 = mesh->add_vertex(vec3(0, 0, 1));

	// Add 4 triangular faces
	mesh->add_triangle(v0, v1, v3);
	mesh->add_triangle(v1, v2, v3);
	mesh->add_triangle(v2, v0, v3);
	mesh->add_triangle(v0, v2, v1);

	return mesh;
}


int main(int /*argc*/, char** /*argv*/) {
	SurfaceMesh* mesh = old_mesh_from_previous_example();
	
    std::cout << "----------------------------------------\n";
    std::cout << "The incident vertices of each vertex" << std::endl;
    std::cout << "----------------------------------------\n";

	// loop over all vertices
	for (auto v : mesh->vertices()) {
		std::cout << "incident vertices of vertex " << v << ": ";
		// loop over all incident vertices
		for (auto vc : mesh->vertices(v))
			std::cout << vc << " ";
		std::cout << std::endl;
	}

    std::cout << "----------------------------------------\n";
    std::cout << "The incident faces of each vertex" << std::endl;
    std::cout << "----------------------------------------\n";

	// loop over all vertices
	for (auto v : mesh->vertices()) {
		std::cout << "incident faces of vertex " << v << ": ";
		// loop over all incident faces
		for (auto f : mesh->faces(v))
			std::cout << f << " ";
		std::cout << std::endl;
	}

    std::cout << "----------------------------------------\n";
    std::cout << "The incident vertices of each face" << std::endl;
    std::cout << "----------------------------------------\n";

	// loop over all faces
	for (auto f : mesh->faces()) {
		std::cout << "incident vertices of face " << f << ": ";
		// loop over all incident vertices
		for (auto v : mesh->vertices(f))
			std::cout << v << " ";
		std::cout << std::endl;
	}

    std::cout << "----------------------------------------\n";
    std::cout << "The incident half-edges of each face" << std::endl;
    std::cout << "----------------------------------------\n";

	// loop over all faces
	for (auto f : mesh->faces()) {
		std::cout << "half-edges around face " << f << ": ";
		// loop over all half-edges around the face
		SurfaceMesh::HalfedgeAroundFaceCirculator h_fit = mesh->halfedges(f);
		SurfaceMesh::HalfedgeAroundFaceCirculator h_end = h_fit;
		do {
			SurfaceMesh::Halfedge h = *h_fit;
			std::cout << h << " ";
			++h_fit;
		} while (h_fit != h_end);
		std::cout << std::endl;
	}

    std::cout << "----------------------------------------\n";
    std::cout << "The two end points of each edge" << std::endl;
    std::cout << "----------------------------------------\n";

	// loop over all edges
	for (auto e : mesh->edges()) {
		std::cout << "the two end points of edge " << e << ": ";
		SurfaceMesh::Vertex vs = mesh->vertex(e, 0);
		std::cout << vs << " ";
		SurfaceMesh::Vertex vt = mesh->vertex(e, 1);
		std::cout << vt << " " << std::endl;
	}

    std::cout << "----------------------------------------\n";
    std::cout << "The two faces connected by each edge" << std::endl;
    std::cout << "----------------------------------------\n";

	// loop over all edges
	for (auto e : mesh->edges()) {
		std::cout << "the two faces connected edge " << e << ": ";
		SurfaceMesh::Halfedge h0 = mesh->halfedge(e, 0);
		if (mesh->is_boundary(h0))
			std::cout << "NULL" << " ";
		else
			std::cout << mesh->face(h0) << " ";
	
		SurfaceMesh::Halfedge h1 = mesh->halfedge(e, 1);
		if (mesh->is_boundary(h1))
			std::cout << "NULL" << " ";
		else
			std::cout << mesh->face(h1) << " ";

		std::cout << std::endl;
	}

	// Delete the mesh (i.e., release memory)
	delete mesh;

    return EXIT_SUCCESS;
}

