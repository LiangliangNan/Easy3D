/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
 * Easy3D is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3
 * as published by the Free Software Foundation.
 *
 * Easy3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ********************************************************************/

#include <easy3d/core/surface_mesh.h>
#include <easy3d/util/initializer.h>


using namespace easy3d;


// This example shows how to create and access properties defined on a surface mesh.
// We use per-face properties as example, you should be able to do similarly for per-edge/vertex properties also.


SurfaceMesh* old_mesh_from_previous_example() {
	// Create a surface mesh
	auto mesh = new SurfaceMesh;

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

int main(int argc, char** argv) {
    // initialize Easy3D.
    initialize();

    // The mesh created from the previous tutorial.
	SurfaceMesh* mesh = old_mesh_from_previous_example();

	// We add a per-face property "f:normal" storing the normal of each face
	SurfaceMesh::FaceProperty<vec3> normals = mesh->add_face_property<vec3>("f:normal");

	// for each face, we access the face normal and print it.
	for (auto f : mesh->faces()) {
		// We use the built-in function of SurfaceMesh compute_face_normal(). 
		// Of course, you can write your own function to compute the normal of
		// a face (the normalized cross product of two consecutive edge vectors). 
		normals[f] = mesh->compute_face_normal(f);
		std::cout << "normal of face " << f << ": " << normals[f] << std::endl;
	}

	delete mesh;

    return EXIT_SUCCESS;
}

