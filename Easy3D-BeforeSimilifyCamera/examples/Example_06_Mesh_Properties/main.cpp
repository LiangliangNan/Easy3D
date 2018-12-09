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

#include <easy3d/model/surface_mesh.h>

using namespace easy3d;


// This example shows how to
//		- add per-face properties to a mesh;
//		- access existing properties.
//
// You should be able to add/access per-edge/vertex properties also.

Surface_mesh* old_mesh_from_previous_example() {
	// Create a surface mesh
	Surface_mesh* mesh = new Surface_mesh;

	// Add 4 vertices
	Surface_mesh::Vertex v0 = mesh->add_vertex(vec3(0, 0, 0));
	Surface_mesh::Vertex v1 = mesh->add_vertex(vec3(1, 0, 0));
	Surface_mesh::Vertex v2 = mesh->add_vertex(vec3(0, 1, 0));
	Surface_mesh::Vertex v3 = mesh->add_vertex(vec3(0, 0, 1));

	// Add 4 triangular faces
	mesh->add_triangle(v0, v1, v3);
	mesh->add_triangle(v1, v2, v3);
	mesh->add_triangle(v2, v0, v3);
	mesh->add_triangle(v0, v2, v1);

	return mesh;
}

void main() {
	Surface_mesh* mesh = old_mesh_from_previous_example();

	// We add a per-face property "f:normal" storing the normal of each face

	Surface_mesh::Face_property<vec3> normals = mesh->add_face_property<vec3>("v:normal");
	for (auto f : mesh->faces()) {
		// We use the built-in function of Surface_mesh compute_face_normal(). 
		// Of course you can write your own function to compute the normal of 
		// a face (the normalized cross product of two consecutive edge vectors). 
		normals[f] = mesh->compute_face_normal(f);
		std::cout << "normal of face " << f << ": " << normals[f] << std::endl;
	}

	delete mesh;
}
