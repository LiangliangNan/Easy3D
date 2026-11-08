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
#include <easy3d/algo/delaunay_2d.h>
#include <easy3d/util/initializer.h>

#include <random>

/**
 * \example{lineno} Tutorial_605_Delaunay2D/main.cpp
 *
 * This example shows how to compute the 2D Delaunay triangulation of a set of 2D points
 * using the Delaunay2 class of Easy3D, and how to visualize the resulting triangulation.
 *
 * The input is a set of 2D points. Delaunay2 computes the triangulation; we then read back
 * the vertices (vertex()) and triangles (tri_vertex()) and store them in a SurfaceMesh for
 * rendering.
 */

using namespace easy3d;


std::shared_ptr<SurfaceMesh> create_delaunay_mesh() {
    // A set of 2D points (a deterministic random sample so the result is reproducible).
    std::mt19937 gen(42);
    std::uniform_real_distribution<float> dis_x(0.0f, 800.0f);
    std::uniform_real_distribution<float> dis_y(0.0f, 600.0f);
    std::vector<vec2> points;
    for (int i = 0; i < 80; ++i)
        points.emplace_back(dis_x(gen), dis_y(gen));

    // Compute the 2D Delaunay triangulation.
    Delaunay2 delaunay;
    delaunay.set_vertices(points);

    // Store the result in a SurfaceMesh for rendering.
    auto mesh = std::make_shared<SurfaceMesh>();
    for (unsigned int i = 0; i < delaunay.nb_vertices(); ++i) {
        const vec2 &p = delaunay.vertex(i);
        mesh->add_vertex(vec3(p, 0.0));
    }
    for (unsigned int t = 0; t < delaunay.nb_triangles(); ++t) {
        mesh->add_triangle(
                SurfaceMesh::Vertex(delaunay.tri_vertex(t, 0)),
                SurfaceMesh::Vertex(delaunay.tri_vertex(t, 1)),
                SurfaceMesh::Vertex(delaunay.tri_vertex(t, 2))
        );
    }
    return mesh;
}


int main(int argc, char **argv) {
    initialize();
    Viewer viewer(EXAMPLE_TITLE);

    viewer.camera()->setUpVector(vec3(0, 1, 0));
    viewer.camera()->setViewDirection(vec3(0.2, 0.2, -1));

    auto mesh = create_delaunay_mesh();
    if (!mesh || mesh->empty()) return EXIT_FAILURE;
    viewer.add_model(mesh, true);

    // show the vertices
    mesh->renderer()->get_points_drawable("vertices")->set_visible(true);
    mesh->renderer()->get_points_drawable("vertices")->set_impostor_type(easy3d::PointsDrawable::SPHERE);
    mesh->renderer()->get_points_drawable("vertices")->set_point_size(10);
    // show the edges (the Delaunay triangulation)
    mesh->renderer()->get_lines_drawable("edges")->set_visible(true);
    // also show the borders (the convex hull of the point set)
    mesh->renderer()->get_lines_drawable("borders")->set_visible(true);

    return viewer.run();
}