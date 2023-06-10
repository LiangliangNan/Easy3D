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
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/algo/polygon_partition.h>
#include <easy3d/util/initializer.h>

#include <easy3d/util/timer.h>

using namespace easy3d;


int test_convex_partition(int duration) {
    Viewer viewer("ConvexPartition");
    viewer.camera()->setUpVector(vec3(0, 1, 0));
    viewer.camera()->setViewDirection(vec3(0, 0, -1));

    // the vertices of the polygon (including the holes).
    std::vector<vec2> points = {
            vec2(  0,   0), // 0: start of outer contour, in counter-clockwise order
            vec2(500,   0), // 1
            vec2(500, 700), // 2
            vec2(  0, 700), // 3
            vec2(100, 100), // 4: start of 1st hole, in counter-clockwise order
            vec2(400, 100), // 5
            vec2(400, 300), // 6
            vec2(100, 300), // 7
            vec2(100, 400), // 8: start of 2nd hole, in counter-clockwise order
            vec2(400, 400), // 9
            vec2(400, 600), // 10
            vec2(100, 600), // 11
    };

    // the outer contour represented by the vertex indices. Note: must be in counter-clock order.
    std::vector<PolygonPartition::Polygon> polygons = {
            {0, 1, 2, 3}    // outer contour
    };

    // the interior contours representing holes. Note: hole vertices must be in counter-clock order.
    std::vector<PolygonPartition::Polygon> holes = {
            {7,  6,  5, 4}, // {4, 5, 6, 7} won't work
            {11, 10, 9, 8}, // {8, 9, 10, 11} won't work
    };

    // convex partition
    std::vector<PolygonPartition::Polygon> parts;
    if (!PolygonPartition::apply(points, polygons, holes, parts))
        return EXIT_FAILURE;

    //--------------------------------------------------------

    // Visualizing the result.
    // we create a mesh to store the resulted convex polygons.
    SurfaceMesh* mesh = new SurfaceMesh;
    for (const auto& p : points)
        mesh->add_vertex({p.x, p.y, 0});
    for (auto& poly : parts) {
        std::vector<SurfaceMesh::Vertex> vts;
        for (const auto& id : poly)
            vts.push_back(SurfaceMesh::Vertex(id));
        mesh->add_face(vts);
    }
    // add the mesh to the viewer.
    viewer.add_model(mesh);

    // show the vertices
    mesh->renderer()->get_points_drawable("vertices")->set_visible(true);
    mesh->renderer()->get_points_drawable("vertices")->set_impostor_type(easy3d::PointsDrawable::SPHERE);
    mesh->renderer()->get_points_drawable("vertices")->set_point_size(12);
    // show the edges
    mesh->renderer()->get_lines_drawable("edges")->set_visible(true);
    // also show the borders
    mesh->renderer()->get_lines_drawable("borders")->set_visible(true);

    viewer.set_usage("testing convex partition...");

    Timer<>::single_shot(duration, (Viewer*)&viewer, &Viewer::exit);

    // run the viewer
    return viewer.run();
}