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
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>


using namespace easy3d;


// This example shows how to render imposters, e.g.,
//		- points as spheres, surfels
//		- lines as cylinders;
//      - lines as cones.


// render mesh vertices as spheres
void create_spheres(SurfaceMesh *mesh) {
    PointsDrawable *drawable = mesh->renderer()->add_points_drawable("vertices");
    drawable->set_uniform_coloring(vec4(1.0f, 0.0f, 0.0f, 1.0f));
    drawable->set_point_size(24.0f);
    drawable->set_impostor_type(PointsDrawable::SPHERE);
}


// render mesh edges as cylinders
void create_cylinders(SurfaceMesh *mesh) {
    LinesDrawable *drawable = mesh->renderer()->add_lines_drawable("edges");
    drawable->set_uniform_coloring(vec4(1.0f, 0.67f, 0.5f, 1.0f));
    drawable->set_impostor_type(LinesDrawable::CYLINDER);
    drawable->set_line_width(6);
}


// render the vertex normals as cones
void create_cones(SurfaceMesh *mesh) {
    auto points = mesh->get_vertex_property<vec3>("v:point");
    mesh->update_vertex_normals();
    auto normals = mesh->get_vertex_property<vec3>("v:normal");

    // Get the bounding box of the model. Then we defined the length of the
    // normal vectors to be 15% of the bounding box diagonal.
    float length = mesh->bounding_box().diagonal_length() * 0.15f;

    // Now let collects the two end points of each normal vector. So from
    // these points we can create a drawable to visualize the normal vectors.

    // Every consecutive two points represent a normal vector.
    std::vector<vec3> normal_points;
    for (auto v : mesh->vertices()) {
        const vec3 &s = points[v];
        const vec3 &t = points[v] + normals[v] * length;
        normal_points.push_back(s);
        normal_points.push_back(t);
    }

    LinesDrawable *drawable = mesh->renderer()->add_lines_drawable("normals");
    drawable->update_vertex_buffer(normal_points);
    drawable->set_uniform_coloring(vec4(0.0f, 1.0f, 0.0f, 1.0f));
    drawable->set_impostor_type(LinesDrawable::CONE);
    drawable->set_line_width(8);
}


// render mesh vertices as surfels
void create_surfels(SurfaceMesh *mesh) {
    PointsDrawable *drawable = mesh->renderer()->add_points_drawable("vertices");
    drawable->set_uniform_coloring(vec4(1.0f, 0.0f, 0.0f, 1.0f));
    drawable->set_point_size(24.0f);
    drawable->set_impostor_type(PointsDrawable::SURFEL);
}


int main(int argc, char **argv) {
    // initialize Easy3D.
    initialize();

    // Create the default Easy3D viewer.
    // Note: a viewer must be created before creating any drawables.
    Viewer viewer(EXAMPLE_TITLE);

    // Load point cloud data from a file
    const std::string file_name = resource::directory() + "/data/sphere.obj";
    auto mesh = dynamic_cast<SurfaceMesh *>(viewer.add_model(file_name, false));
    if (!mesh) {
        LOG(ERROR) << "failed to load model. Please make sure the file exists and format is correct.";
        return EXIT_FAILURE;
    }

    //--------------------- render vertices as spheres ----------------

    create_spheres(mesh);

    //--------------------- render edges as cylinders -----------------

    create_cylinders(mesh);

    //--------------------- render normals as cones -------------------

    create_cones(mesh);

    //-------------------- render vertices as surfels -----------------

    // make a copy of the mesh
    auto copy = new SurfaceMesh(*mesh);
    // translate the mesh a bit so we can see both
    const vec3 trans = vec3(0, 1, 0) * mesh->bounding_box().diagonal_length() * 0.7f;
    auto points = copy->get_vertex_property<vec3>("v:point");
    for (auto v : copy->vertices())
        points[v] += trans;
    viewer.add_model(copy, false);

    create_surfels(copy);

    // ----------------------------------------------------------------

    // run the viewer
    return viewer.run();
}

