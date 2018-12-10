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
#include <easy3d/surface_mesh.h>
#include <easy3d/drawable.h>


using namespace easy3d;

// This example shows how to
//		- rendering a scalar field defined on vertices of a mesh;
//		- use the viewer to visualize the drawable.


// You have seen the creation of this model and it drawable in the
// previous tutorial. Just skip it and go to the main() function.
SurfaceMesh* old_mesh_from_previous_example() {
	// Create a surface mesh
	SurfaceMesh* mesh = new SurfaceMesh;

	// Read a mesh specified by its file name
    std::string file_name = "../../../Easy3D/data/building.off";
	bool success = mesh->read(file_name);
	if (!success) {
		std::cerr << "file does not exist or invalid file format" << std::endl;
		delete mesh;
		return nullptr;
	}

	// Create the face drawable and attach it to the mesh.
	FacesDrawable* surfaces_drawable = mesh->add_faces_drawable("surface");

	// All the vertices.
	auto vertices = mesh->get_vertex_property<vec3>("v:point");
	const auto& points = vertices.vector();
	// Upload the vertex positions to the GPU.
	surfaces_drawable->update_vertex_buffer(points);

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
	surfaces_drawable->update_index_buffer(indices);

	return mesh;
}


void main() {
	// Create the default Easy3D viewer.
	// Note: a viewer must be created before creating any drawables. 
    Viewer viewer("Tutorial_10_ScalarFields");

	// Load a mesh model and create a drawable for the faces. 
	SurfaceMesh* mesh = old_mesh_from_previous_example();
	if (!mesh)
		return;

	// In the previous tutorial, we render the model using a uniform color.
	// In this tutorial, let's define a scalar field on the mesh vertices: elevation. 
	// This is actually the z-value of each vertex.
	auto elevation = mesh->add_vertex_property<float>("v:elevation");
	auto vertices = mesh->get_vertex_property<vec3>("v:point");
	for (auto v : mesh->vertices())
		elevation[v] = vertices[v].z;

	// To visualize the scalar field, we assign each vertex a color according 
	// its elevation. The idea is to map the elevation values to a color range
	// from blue to red. To do the mapping, we need to know the min/max value
	// range of the scalar field.
	float min_value = std::numeric_limits<float>::max();
	float max_value = -std::numeric_limits<float>::max();
	for (auto v : mesh->vertices()) {
		min_value = std::min(min_value, elevation[v]);
		max_value = std::max(max_value, elevation[v]);
	}

	// With the value range of the scalar field, we assign each vertex a color
	// according to its scalar value. We can use a vertex property or an array 
	// to store the colors. Here we use the std::vector array.
	std::vector<vec3> scalar_field_colors;
	for (auto v : mesh->vertices()) {
		float value = elevation[v];
		float r = (value - min_value) / (max_value - min_value);
		scalar_field_colors.push_back(vec3(r, 0.0f, 1.0f -r));
	}

	// The faces drawable we created before.
	FacesDrawable* surface = mesh->faces_drawable("surface");
	// Note we had already uploaded the vertex positions and the vertex indices
	// to the GPU. Now we only need to transfer the color data to the GPU.
	surface->update_color_buffer(scalar_field_colors);

	// Vertices have varying colors. 
	surface->set_per_vertex_color(true);

	// Add the model to the viewer
	viewer.add_model(mesh);

	// Run the viewer
	viewer.run();
}
