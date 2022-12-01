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
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/timer.h>

using namespace easy3d;


int test_vector_field(int duration) {
    Viewer viewer("VectorField");

    // Load point cloud data from a file
    const std::string file_name = resource::directory() + "/data/sphere.obj";
    SurfaceMesh *model = dynamic_cast<SurfaceMesh *>(viewer.add_model(file_name, true));
    if (!model) {
        LOG(ERROR) << "failed to load model. Please make sure the file exists and format is correct.";
        return EXIT_FAILURE;
    }

    // Get the bounding box of the model. Then we defined the length of the
    // normal vectors to be 5% of the bounding box diagonal.
    const Box3 &box = model->bounding_box();
    float length = norm(box.max_point() - box.min_point()) * 0.05f;

    // Compute the face normals.
    model->update_face_normals();
    auto normals = model->get_face_property<vec3>("f:normal");

    // Every consecutive two points represent a normal vector.
    std::vector<vec3> points;
    for (auto f : model->faces()) {
        vec3 center(0, 0, 0); // face center
        int count = 0;
        for (auto v : model->vertices(f)) {
            center += model->position(v);
            ++count;
        }

        const vec3 s = center / count;
        const vec3 t = s + normals[f] * length;
        points.push_back(s);
        points.push_back(t);
    }

    // Create a drawable for rendering the normal vectors.
    auto drawable = model->renderer()->add_lines_drawable("normals");
    // Upload the data to the GPU.
    drawable->update_vertex_buffer(points);

    // We will draw the normal vectors in a uniform green color
    drawable->set_uniform_coloring(vec4(0.0f, 1.0f, 0.0f, 1.0f));

    // Set the line width
    drawable->set_line_width(3.0f);

    // Also show the standard "edges"
    model->renderer()->get_lines_drawable("edges")->set_visible(true);

    viewer.set_usage("testing vector field...");

    Timer<>::single_shot(duration, (Viewer*)&viewer, &Viewer::exit);
    return viewer.run();
}

