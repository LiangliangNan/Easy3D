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

#include <easy3d/core/viewer.h>
#include <easy3d/model/surface_mesh.h>
#include <easy3d/core/drawable.h>


using namespace easy3d;

// This example shows how to
//		- create a drawable for a specific rendering purpose;
//		- use the viewer to visualize the drawable.


void main() {
	// Create the default Easy3D viewer.
	// Note: a viewer must be created before creating any drawables. 
	Viewer viewer("Example_09_Drawables");

	// Create a surface mesh
	Surface_mesh* mesh = new Surface_mesh;

	// Read a mesh specified by its file name
	std::string file_name = "../../../data/building.off";
	bool success = mesh->read(file_name);
	if (!success) {
		std::cerr << "file does not exist or invalid file format" << std::endl;
		delete mesh;
		return;
	}

	// A drawable can represent either a set of points, or lines, or faces. 
	// In "Example_08_Viewer", you have learned how to create drawable for 
	// rendering point cloud (a set of points).
	// In this example, let's create two drawables:
	//    - a "FacesDrawable" representing the surface of the model;
	//    - a "LinesDrawable" representing the bounding box of the model.

	// Create the face drawable and attach it to the mesh.
	FacesDrawable* surface_drawable = mesh->add_face_drawable("surface");

	// To visualize the surface, there are two options to collect data and feed
	// them to the GPU. 
	// One option is to collect three points for every single 
	// triangle. This is simple, but not memory friendly because vertices can 
	// be shared by of their incident faces. So it is not necessary to duplicate
	// the vertices.
	// Another option is to send each vertex only once, but we need to know the
	// connectivities of these vertices. This is simple: it is sufficient to let
	// the GPU know the vertex indices of each face. We call it index buffer.

	// All the vertices.
	auto vertices = mesh->get_vertex_property<vec3>("v:point");
	const auto& points = vertices.vector();
	// Upload the vertex positions to the GPU.
	surface_drawable->update_vertex_buffer(points);

	// Now the vertex indices for all the triangles.
	std::vector<unsigned int> indices;
	std::size_t non_triangles = 0;
	for (auto f : mesh->faces()) {
		std::vector<unsigned int> vts;
		for (auto v : mesh->vertices(f))
			vts.push_back(v.idx());
		if (vts.size() == 3)
			indices.insert(indices.end(), vts.begin(), vts.end());
		else
			++non_triangles;
	}
	if (non_triangles > 0) {
		std::cerr << "Warning: the default Easy3D viewer can only render triangles and"
			" non-triangle faces are ignored" << std::endl;
	}
	// Upload the vertex indices to the index buffer.
	// After this, the data for rendering the surface drawable is complete.
	surface_drawable->update_index_buffer(indices);
	surface_drawable->set_default_color(vec3(0.4f, 0.8f, 0.8f)); // give it an color

	// Now let's create the drawable for the bounding box of the model.
	// We also attached it to the model.
	LinesDrawable* bbox_drawable = mesh->add_line_drawable("bbox");
	Box3 box;
	// Compute the bounding box.
	for (const auto& p : points)
		box.add_point(p);
	float xmin = box.x_min();	float xmax = box.x_max();
	float ymin = box.y_min();	float ymax = box.y_max();
	float zmin = box.z_min();	float zmax = box.z_max();
	// The eight vertices of the bounding box.
	const std::vector<vec3> bbox_points = {
		vec3(xmin, ymin, zmax), vec3(xmax, ymin, zmax),
		vec3(xmin, ymax, zmax),	vec3(xmax, ymax, zmax),
		vec3(xmin, ymin, zmin),	vec3(xmax, ymin, zmin),
		vec3(xmin, ymax, zmin),	vec3(xmax, ymax, zmin)
	};
	const std::vector<unsigned int> bbox_indices = {
		0, 1, 2, 3, 4, 5, 6, 7,
		0, 2, 4, 6, 1, 3, 5, 7,
		0, 4, 2, 6, 1, 5, 3, 7
	};

	// Upload the vertex positions of the bounding box to the GPU.
	bbox_drawable->update_vertex_buffer(bbox_points);
	// Upload the vertex indices of the bounding box to the GPU.
	bbox_drawable->update_index_buffer(bbox_indices);
	bbox_drawable->set_default_color(vec3(1.0f, 0.0f, 0.0f));	// red color

	// Add the model to the viewer
	viewer.add_model(mesh);

	// Run the viewer
	viewer.run();
}