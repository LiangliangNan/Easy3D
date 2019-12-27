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
#include <easy3d/viewer/drawable_lines.h>
#include <easy3d/viewer/drawable_triangles.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/viewer/setting.h>


using namespace easy3d;


// This example shows how to
//		- create a drawable for a specific rendering purpose;
//		- use the viewer to visualize the drawable.


int main(int /*argc*/, char** /*argv*/) {
	// Read a mesh specified by its file name
    const std::string file_name = setting::resource_directory() + "/data/building.off";

    try {

        // Create the default Easy3D viewer.
        // Note: a viewer must be created before creating any drawables.
        Viewer viewer("Tuturial_302_Drawables");

        // Use the built-in open() function of the viewer to load the model.
        // This funnction will do the following:
        //  - load the model to the viewer.
        //  - create necessary default drawables for visualization.
        Model* mesh = viewer.open(file_name, true);
        if (!mesh) {
            std::cerr << "failed loading model from \'" << file_name << "\'" << std::endl;
            return EXIT_FAILURE;
        }

        // Now let's create the drawable for the bounding box of the model.
        // We also attached it to the model.
        LinesDrawable* bbox_drawable = mesh->add_lines_drawable("bbox");
        const Box3& box = mesh->bounding_box();
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
        bbox_drawable->set_line_width(5.0f);

        // Run the viewer
        viewer.run();

        // Delete the mesh (i.e., release memory)? No. The viewer will do this.
        // delete mesh;
    } catch (const std::runtime_error &e) {
        const std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
        std::cerr << error_msg << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

