/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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
 */

#include <easy3d/viewer/viewer.h>
#include <easy3d/renderer/vertex_array_object.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/fileio/resources.h>
#include <easy3d/util/logging.h>

using namespace easy3d;

// This example shows how to map an array buffer for dynamic buffer update (useful for animation).

int main(int argc, char **argv) {
    // Initialize logging.
    logging::initialize();

    //-------------------------------------------------------------

    // Create the default Easy3D viewer.
    // Note: a viewer must be created before creating any drawables.
    Viewer viewer("Tutorial_311_Animation");

    //-------------------------------------------------------------

    // We use the points and indices of the "bunny".
    const std::vector<vec3> &points = resource::bunny_vertices;
    // Each consecutive 3 indices represent a triangle.
    const std::vector<unsigned int> &indices = resource::bunny_indices;

    //-------------------------------------------------------------
    // Create a TrianglesDrawable to visualize the surface of the "bunny".
    auto surface = new TrianglesDrawable("faces");

    // Upload the vertex positions of the surface to the GPU.
    // The second argument is 'true' for dynamic update of the buffer data.
    surface->update_vertex_buffer(points, true);

    // Upload the vertex indices of the surface to the GPU. The indices represent how the vertices are connected to
    // form triangles.
    surface->update_element_buffer(indices);
    // Add the drawable to the viewer
    viewer.add_drawable(surface);

    //-------------------------------------------------------------

    viewer.fit_screen();

    // Create an animation function to define how the vertex positions are updated.
    // This is a trivial example that stretches the model along the Z-axis.
    viewer.animation_func_ = [&](easy3d::Viewer& v) -> bool {
        static vec3* vertices = nullptr;
        if (!vertices)
            vertices = (vec3*)VertexArrayObject::map_buffer(GL_ARRAY_BUFFER, surface->vertex_buffer(), GL_WRITE_ONLY);

        static double total_scale = 1.0;
        float scale = 1.01f;
        if (total_scale > 2) {
            scale = static_cast<float>(1.0 / total_scale);
            total_scale = 1.0;
        }
        else
            total_scale *= scale;

        for (int i=0; i<points.size(); ++i)
            vertices[i].z *= scale;

        viewer.update();
        return true;
    };

    // Run the viewer
    return viewer.run();
}

