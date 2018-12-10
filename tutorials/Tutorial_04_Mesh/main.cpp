/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D: software for processing and rendering
*   meshes and point clouds.
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

#include <easy3d/surface_mesh.h>


using namespace easy3d;


// This example shows how to
//		- construct a mesh from its vertices and known connectivities


void main() {
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

	std::cout << "vertices: " << mesh->n_vertices() << std::endl;
	std::cout << "edges: " << mesh->n_edges() << std::endl;
	std::cout << "faces: " << mesh->n_faces() << std::endl;

	// Delete the mesh (i.e., release memory)
	delete mesh;
}
