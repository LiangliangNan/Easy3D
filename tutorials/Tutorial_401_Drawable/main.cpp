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

#include <easy3d/viewer/viewer.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/viewer/drawable.h>
#include <easy3d/fileio/surface_mesh_io.h>


using namespace easy3d;


// This example shows how to
//		- create a drawable for a specific rendering purpose;
//		- use the viewer to visualize the drawable.


int main(int /*argc*/, char** /*argv*/) {
	// Create the default Easy3D viewer.
	// Note: a viewer must be created before creating any drawables. 
    Viewer viewer("Tuturial_302_Drawables");

	// Read a mesh specified by its file name
    const std::string file_name = "../../Easy3D/data/building.off";

    // There are multiple options for loading a model and creating drawables.
    // This example shows how to customize the drawables. You can change the
    // "#if 0" to "#if 1" to see the differences.
#if 0
    // Use the built-in open() function of the viewer to load the model.
    // This funnction will do the following:
    //  - load the model to the viewer.
    //  - create necessary default drawables for visualization.
    easy3d::Model* model = viewer.open(file_name);
    if (!model)
        std::cerr << "failed loading model from \'" << file_name << "\'" << std::endl;

#else
    SurfaceMesh* mesh = SurfaceMeshIO::load(file_name);
    if (!mesh) {
        std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct." << std::endl;
        return EXIT_FAILURE;
	}

    // Drawables can be used to visualize either a set of points, lines, or faces.
	// In this example, let's create two drawables:
    //    - a "TrianglesDrawable" vasualizing the surface of the model;
    //    - a "LinesDrawable" vasualizing the bounding box of the model.

    // Create the drawable and attach it to the mesh.
    TrianglesDrawable* surface_drawable = mesh->add_triangles_drawable("surface");

    // To visualize the surface, there are two options to collect data and feed
    // them to the GPU.
    // One option is to collect three points for every single triangle. This is
    // simple, but not memory friendly because vertices can be shared by their
    // incident faces. So it is not necessary to duplicate the vertices.
    // Another option is to send each vertex only once, but we need to know the
    // connectivities of these vertices. This is simple: it is sufficient to let
    // the GPU know the vertex indices of each face. To do so, we use an index buffer.

    // The "point" property
    auto vertices = mesh->get_vertex_property<vec3>("v:point");
    // All the XYZ coordinates
    const auto& points = vertices.vector();
    // Upload the vertex positions to the GPU.
    surface_drawable->update_vertex_buffer(points);

    // computer vertex normals for each vertex
    mesh->update_vertex_normals();
    // The "normal" property
    auto normals = mesh->get_vertex_property<vec3>("v:normal");
    // Upload the vertex positions to the GPU.
    surface_drawable->update_normal_buffer(normals.vector());

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
	LinesDrawable* bbox_drawable = mesh->add_lines_drawable("bbox");
	Box3 box;
	// Compute the bounding box.
	for (const auto& p : points)
		box.add_point(p);
    float xmin = box.min(0);	float xmax = box.max(0);
    float ymin = box.min(1);	float ymax = box.max(1);
    float zmin = box.min(2);	float zmax = box.max(2);
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
#endif

	// Run the viewer
	viewer.run();

    // Delete the mesh (i.e., release memory)? No. The viewer will do this.
    // delete mesh;

    return EXIT_SUCCESS;
}

