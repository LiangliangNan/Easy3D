/** \cond LICENSE
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
 \endcond */

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
 * \brief Computing and visualizing the 2D Delaunay triangulation of a point set with Delaunay2.
 *
 * <b>Overview</b>
 * This tutorial scatters 80 random 2D points, computes their Delaunay triangulation with Delaunay2,
 * and reads the result back into a SurfaceMesh for display. It is the canonical example of turning an
 * unstructured point set into a well-shaped triangle mesh — a building block for terrain modeling,
 * mesh generation, and spatial interpolation.
 *
 * <b>The technique</b>
 * A Delaunay triangulation of a point set is a triangulation such that no input point lies inside the
 * circumcircle of any triangle (the "empty circumcircle" property). Among all possible triangulations
 * of the same points, the Delaunay triangulation maximizes the minimum angle, so it avoids skinny
 * "sliver" triangles and produces the most equilateral, numerically well-behaved mesh. It is also the
 * geometric dual of the Voronoi diagram: two points are connected by a Delaunay edge exactly when their
 * Voronoi cells share a boundary. The triangulation covers the convex hull of the point set, which is
 * why the outer border of the result is the convex hull. Easy3D's Delaunay2 wraps Jonathan Richard
 * Shewchuk's robust "Triangle" implementation.
 * References: B. Delaunay, "Sur la sphere vide", 1934; J. R. Shewchuk, "Triangle: Engineering a 2D
 * Quality Mesh Generator and Delaunay Triangulator", 1996.
 *
 * <b>Easy3D API</b>
 * - Delaunay2: set_vertices() computes the triangulation; nb_vertices()/vertex(i) read back the points;
 *   nb_triangles()/tri_vertex(t, lv) read back each triangle's three vertex indices.
 * - SurfaceMesh: stores the vertices (lifted to z = 0) and triangles for rendering.
 * - Viewer, PointsDrawable, LinesDrawable: show the points, the triangulation edges, and the convex-hull
 *   border.
 *
 * <b>Running it</b>
 * A window opens showing the 80 points (as spheres) connected by the Delaunay edges, with the convex hull
 * highlighted as the border. The camera looks almost straight down (with a slight tilt) so the flat
 * triangulation is easy to read. The random sample is seeded, so the result is reproducible.
 *
 * \see Delaunay2, SurfaceMesh, Viewer, Tutorial_601_Tessellator
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
        mesh->add_triangle(SurfaceMesh::Vertex(delaunay.tri_vertex(t, 0)),
                           SurfaceMesh::Vertex(delaunay.tri_vertex(t, 1)),
                           SurfaceMesh::Vertex(delaunay.tri_vertex(t, 2)));
    }
    return mesh;
}

int main(int argc, char **argv) {
    initialize();
    Viewer viewer(EXAMPLE_TITLE);

    viewer.camera()->setUpVector(vec3(0, 1, 0));
    viewer.camera()->setViewDirection(vec3(0.2, 0.2, -1));

    auto mesh = create_delaunay_mesh();
    if (!mesh || mesh->empty())
        return EXIT_FAILURE;
    viewer.add_model(mesh, true);

    // show the vertices
    mesh->renderer()->get_points_drawable("vertices")->set_visible(true);
    mesh->renderer()->get_points_drawable("vertices")->set_impostor_type(easy3d::PointsDrawable::SPHERE);
    mesh->renderer()->get_points_drawable("vertices")->set_point_size(8);
    // show the edges (the Delaunay triangulation)
    mesh->renderer()->get_lines_drawable("edges")->set_visible(true);
    // also show the borders (the convex hull of the point set)
    mesh->renderer()->get_lines_drawable("borders")->set_visible(true);

    return viewer.run();
}