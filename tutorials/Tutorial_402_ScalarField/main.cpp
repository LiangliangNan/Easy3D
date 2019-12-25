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
#include <easy3d/viewer/drawable_triangles.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/viewer/setting.h>


using namespace easy3d;


// This example shows how to
//		- rendering a scalar field defined on vertices of a mesh;
//		- use the viewer to visualize the drawable.


int main(int /*argc*/, char** /*argv*/) {
    const std::string file_name = setting::resource_directory() + "/data/building.off";

    try {
        // Create the default Easy3D viewer.
        // Note: a viewer must be created before creating any drawables.
        Viewer viewer("Tutorial_402_ScalarField");

        // Load a mesh model and create a drawable for the faces.
        SurfaceMesh* mesh = dynamic_cast<SurfaceMesh*>(viewer.open(file_name, true, true));
        if (!mesh) {
            std::cerr << "Error: failed to load model. Please make sure the file exists and format is correct." << std::endl;
            return EXIT_FAILURE;
        }

        // By default, Easy3D renders the model using a uniform color.
        // In this tutorial, let's define a scalar field on the mesh vertices: elevation.
        // This is actually the z-value of each vertex.
        // To visualize the scalar field, we assign each vertex a color according
        // its elevation. The idea is to map the elevation values to a color range
        // (e.g., here from blue to red. To do the mapping, we need to know the
        // min/max value range of the scalar field.
        auto elevation = mesh->add_vertex_property<float>("v:elevation");
        auto vertices = mesh->get_vertex_property<vec3>("v:point");
        float min_value = std::numeric_limits<float>::max();
        float max_value = -std::numeric_limits<float>::max();
        for (auto v : mesh->vertices()) {
            elevation[v] = vertices[v].z;
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

        // The faces drawable created by default.
        TrianglesDrawable* surface = mesh->triangles_drawable("surface");
        // Note we had already uploaded the vertex positions and the vertex indices
        // to the GPU. Now we only need to transfer the color data to the GPU.
        surface->update_color_buffer(scalar_field_colors);

        // Vertices have varying colors.
        surface->set_per_vertex_color(true);

        // Run the viewer
        viewer.run();
    }
    catch (const std::runtime_error &e) {
        std::string error_msg = std::string("Caught a fatal error: ") + std::string(e.what());
        std::cerr << error_msg << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

