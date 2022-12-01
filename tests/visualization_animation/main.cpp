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
#include <easy3d/renderer/vertex_array_object.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/timer.h>

using namespace easy3d;

int test_animation(int duration) {

    Viewer viewer("Animation");

    const std::vector<vec3> &points = resource::bunny_vertices;
    const std::vector<unsigned int> &indices = resource::bunny_indices;

    auto surface = new TrianglesDrawable("faces");
    surface->update_vertex_buffer(points, true);

    surface->update_element_buffer(indices);
    viewer.add_drawable(surface);
    viewer.fit_screen();

    viewer.set_animation(true);

    viewer.set_usage("testing animation...");

    // Define an animation function to specify how vertex positions are updated.
    // In this trivial example, we stretch the model along the Z-axis.
    viewer.animation_func_ = [&](easy3d::Viewer* v) -> bool {
        (void)v;

        // map the vertex buffer into the client's address space
        void* pointer = VertexArrayObject::map_buffer(GL_ARRAY_BUFFER, surface->vertex_buffer(), GL_WRITE_ONLY);

        vec3* vertices = reinterpret_cast<vec3*>(pointer);
        if (!vertices)
            return false;

        static float total_scale = 1.0f;
        float scale = 1.01f;
        if (total_scale > 1.5f) {
            scale = 1.0f / total_scale;
            total_scale = 1.0f;
        }
        else
            total_scale *= scale;

        for (std::size_t i=0; i<points.size(); ++i)
            vertices[i].z *= scale;

        // unmap the vertex buffer
        VertexArrayObject::unmap_buffer(GL_ARRAY_BUFFER, surface->vertex_buffer());

        viewer.update();
        return true;
    };

    Timer<>::single_shot(duration, (Viewer*)&viewer, &Viewer::exit);
    return viewer.run();
}

