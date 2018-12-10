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

#include <easy3d/viewer.h>
#include <easy3d/drawable.h>
#include <easy3d/resources.h>
#include <easy3d/surface_mesh.h>


using namespace easy3d;


// This example shows how to
//		- create a surface model from a set of triangles;
//      - create a drawable for rendering mesh surfaces;
//		- use the viewer to visualize the surface.

void main() {
	// Create the default Easy3D viewer.
	// Note: a viewer must be created before creating any drawables. 
    Viewer viewer("Tutorial_08_Viewer");

	// Create a mesh model.
	SurfaceMesh* mesh = new SurfaceMesh;

	// In this example, we use the Easy3D example data (a building model) to
	// create the model. 
	//   - demodata::vertices:  an array of 3D points storing the model vertices
	//   - demodata::colors: an array of colors storing the vertex colors 
	for (std::size_t i = 0; i < demodata::vertices.size(); i += 3) {
		const vec3& p0 = demodata::vertices[i];
		const vec3& p1 = demodata::vertices[i + 1];
		const vec3& p2 = demodata::vertices[i + 2];
		std::vector<SurfaceMesh::Vertex> face_vertices = {
			mesh->add_vertex(p0),
			mesh->add_vertex(p1),
			mesh->add_vertex(p2)
		};
		mesh->add_face(face_vertices);
	}

	// Create a drawable for rendering the surface of this model.
	FacesDrawable* drawable = mesh->add_faces_drawable("surface");

	// Transfer vertex coordinates and colors to the GPU. 
	drawable->update_vertex_buffer(demodata::vertices);
	drawable->update_color_buffer(demodata::colors); // an array of colors

	drawable->set_per_vertex_color(true);	// vertices have different colors

	// Add the model to the viewer
	viewer.add_model(mesh);

	// Run the viewer
	viewer.run();
}
