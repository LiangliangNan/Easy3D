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
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/util/resource.h>
#include <easy3d/core/types.h>
#include <easy3d/util/timer.h>

using namespace easy3d;


int test_drawables(int duration) {
    Viewer viewer("Drawables");

    const std::vector<vec3> &points = resource::bunny_vertices;
    const std::vector<unsigned int> &indices = resource::bunny_indices;

    // Create a TrianglesDrawable to visualize the surface of the "bunny".
    // For visualization, the point positions and the vertex indices of the faces have to be sent to the GPU.
    auto surface = new TrianglesDrawable("faces");
    // Upload the vertex positions of the surface to the GPU.
    surface->update_vertex_buffer(points);
    // Upload the vertex indices of the surface to the GPU.
    surface->update_element_buffer(indices);
    // Add the drawable to the viewer
    viewer.add_drawable(surface);

    // Create a PointsDrawable to visualize the vertices of the "bunny".
    // Only the vertex positions have to be sent to the GPU for visualization.
    auto vertices = new PointsDrawable("vertices");
    // Upload the vertex positions to the GPU.
    vertices->update_vertex_buffer(points);
    // Set a color for the vertices (here we want a red color).
    vertices->set_uniform_coloring(vec4(1.0f, 0.0f, 0.0f, 1.0f));  // r, g, b, a
    // Let's render the vertices as spheres.
    vertices->set_impostor_type(PointsDrawable::SPHERE);
    // Set the vertices size (here 10 pixels).
    vertices->set_point_size(10);
    // Add the drawable to the viewer
    viewer.add_drawable(vertices);

    // Create a LinesDrawable to visualize the bounding box of the "bunny".

    // Compute the bounding box.
    auto bbox_drawable = new LinesDrawable("bbox");
    const Box3 &box = geom::bounding_box<Box3, std::vector<vec3> >(points);
    float xmin = box.min_coord(0);
    float xmax = box.max_coord(0);
    float ymin = box.min_coord(1);
    float ymax = box.max_coord(1);
    float zmin = box.min_coord(2);
    float zmax = box.max_coord(2);
    // The eight vertices of the bounding box.
    const std::vector<vec3> bbox_points = {
            vec3(xmin, ymin, zmax), vec3(xmax, ymin, zmax),
            vec3(xmin, ymax, zmax), vec3(xmax, ymax, zmax),
            vec3(xmin, ymin, zmin), vec3(xmax, ymin, zmin),
            vec3(xmin, ymax, zmin), vec3(xmax, ymax, zmin)
    };
    // The vertex indices of the twelve line segments of the bounding box.
    const std::vector<unsigned int> bbox_indices = {
            0, 1, 2, 3, 4, 5, 6, 7,
            0, 2, 4, 6, 1, 3, 5, 7,
            0, 4, 2, 6, 1, 5, 3, 7
    };
    // Upload the vertex positions of the bounding box to the GPU.
    bbox_drawable->update_vertex_buffer(bbox_points);
    // Upload the vertex indices of the bounding box to the GPU.
    bbox_drawable->update_element_buffer(bbox_indices);
    // Set a color for the edges of the bounding box (here we want a blue color).
    bbox_drawable->set_uniform_coloring(vec4(0.0f, 0.0f, 1.0f, 1.0f));    // r, g, b, a
    // Set the width of the edges (here 5 pixels).
    bbox_drawable->set_line_width(5.0f);
    // Add the drawable to the viewer
    viewer.add_drawable(bbox_drawable);

    // Make sure everything is within the visible region of the viewer.
    viewer.fit_screen();

    viewer.set_usage("testing drawables...");

    Timer<>::single_shot(duration, (Viewer*)&viewer, &Viewer::exit);
    return viewer.run();
}

