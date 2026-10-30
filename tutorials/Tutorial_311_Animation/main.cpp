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

#include <easy3d/viewer/viewer.h>
#include "animation.h"
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>


/**
 * \example{lineno} Tutorial_311_Animation/main.cpp
 * This example shows portable dynamic buffer updates for animation on Vulkan and OpenGL.
 */

using namespace easy3d;


int main(int argc, char **argv) {
    // initialize Easy3D.
    initialize();

    //-------------------------------------------------------------

    // Create the default Easy3D viewer.
    // Note: a viewer must be created before creating any drawables.
    Viewer viewer(EXAMPLE_TITLE);

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
    viewer.add_drawable(std::shared_ptr<TrianglesDrawable>(surface));

    viewer.fit_screen();

    // allow animation
    viewer.set_animation(true);

    // Define an animation function to specify how vertex positions are updated.
    // In this trivial example, we stretch the model along the Z-axis.
    BunnyAnimation animation;
    viewer.animation_func_ = [surface, &animation](Viewer*) {
        // Keep editable geometry on the CPU. Updating the drawable publishes a
        // new snapshot; each backend uploads it before rendering the next frame.
        return animation.update(*surface);
    };

    // run the viewer
    return viewer.run();
}

