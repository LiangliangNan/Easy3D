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
#include <easy3d/algo/polygon_partition.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_602_ConvexPartition/main.cpp
 *
 * \brief Partitioning a simple polygon with holes into convex pieces using PolygonPartition.
 *
 * <b>Overview</b>
 * This tutorial takes a rectangle with two rectangular holes and splits it into a set of convex
 * polygons with PolygonPartition::apply(). Convex pieces are the "safe" currency of computational
 * geometry: they are trivial to triangulate, rasterize, and test for point containment or collision,
 * so many pipelines first decompose a complicated region into convex parts.
 *
 * <b>The technique</b>
 * A polygon is convex when every interior angle is at most 180 degrees, i.e. the segment between any
 * two interior points stays inside. A concave (or multiply-connected) polygon can always be cut along
 * diagonals into convex pieces. Easy3D's apply() uses the Hertel-Mehlhorn algorithm: triangulate the
 * polygon, then repeatedly merge two adjacent triangles across a diagonal whenever the merged piece is
 * still convex (equivalently, whenever the diagonal is "inessential" — removing it does not create a
 * reflex vertex). Hertel and Mehlhorn proved this yields at most four times the optimal number of parts,
 * and in practice it is usually optimal; it runs in O(n^2) time and O(n) space. The truly optimal
 * partition (fewest convex pieces) is available as apply_OPT() via the Keil-Snoeyink algorithm, at a
 * higher O(n^3) cost. Holes are handled by treating each hole as an interior contour. The input must be
 * a simple polygon (no self-intersections); for self-intersecting or overlapping contours, first run the
 * CSG operators in easy3d/algo/tessellator.h to obtain simple contours.
 * References: S. Hertel and K. Mehlhorn, "Fast triangulation of simple polygons", 1983;
 * M. Keil and J. Snoeyink, "On the time bound for convex decomposition of simple polygons", 1998.
 *
 * <b>Easy3D API</b>
 * - PolygonPartition::apply(points, polys, holes, parts): the all-in-one entry point. points is the shared
 *   vertex pool; polys are the outer contours (counter-clockwise); holes are interior contours
 *   (clockwise); parts receives the resulting convex polygons as index lists. PolygonPartition::Polygon
 *   is just a std::vector of vertex indices.
 * - SurfaceMesh: the convex parts are stored as faces for display.
 * - Viewer, PointsDrawable, LinesDrawable: show the vertices, the partition edges, and the borders.
 *
 * <b>Running it</b>
 * A window opens showing the rectangle-with-two-holes cut into convex faces; the added diagonals are
 * visible as edges and the hole boundaries as borders. Note the orientation convention in the code:
 * outer contours must be counter-clockwise and holes clockwise, or apply() will fail.
 *
 * \see PolygonPartition, SurfaceMesh, Viewer, Tutorial_601_Tessellator, Tutorial_603_CSG
 */

using namespace easy3d;

std::shared_ptr<SurfaceMesh> create_partition_mesh() {
    // the vertices of the polygon (including the holes).
    // All contours share one vertex pool; each contour below refers to these points by index.
    std::vector<vec2> points = {
        vec2(0, 0),     // 0: start of outer contour, in counterclockwise order
        vec2(500, 0),   // 1
        vec2(500, 700), // 2
        vec2(0, 700),   // 3
        vec2(100, 100), // 4: start of 1st hole, also in counterclockwise order
        vec2(400, 100), // 5
        vec2(400, 300), // 6
        vec2(100, 300), // 7
        vec2(100, 400), // 8: start of 2nd hole, also in counterclockwise order
        vec2(400, 400), // 9
        vec2(400, 600), // 10
        vec2(100, 600), // 11
    };

    // the outer contour represented by the vertex indices. Note: must be in counterclockwise order.
    std::vector<PolygonPartition::Polygon> polygons = {
        {0, 1, 2, 3} // outer contour
    };

    // the interior contours representing holes. Note: hole vertices must be in clockwise order.
    std::vector<PolygonPartition::Polygon> holes = {
        {7, 6, 5, 4},   // {4, 5, 6, 7} won't work
        {11, 10, 9, 8}, // {8, 9, 10, 11} won't work
    };

    // convex partition
    // apply() runs Hertel-Mehlhorn over the outer contour plus the hole contours and returns
    // the convex pieces as index lists into the shared 'points' pool.
    std::vector<PolygonPartition::Polygon> parts;
    if (!PolygonPartition::apply(points, polygons, holes, parts))
        return {};

    //--------------------------------------------------------

    // Visualizing the result.
    // we create a mesh to store the resulted convex polygons.
    auto mesh = std::make_shared<SurfaceMesh>();
    // Lift the 2D points into the z = 0 plane so the 3D viewer can display them.
    for (const auto &p : points)
        mesh->add_vertex({p.x, p.y, 0});
    // Each convex part becomes one face; the indices already refer to the vertices added above.
    for (const auto &poly : parts) {
        std::vector<SurfaceMesh::Vertex> vts;
        for (const auto &id : poly)
            vts.push_back(SurfaceMesh::Vertex(static_cast<int>(id)));
        mesh->add_face(vts);
    }
    return mesh;
}

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();
    Viewer viewer(EXAMPLE_TITLE);

    // Look straight down onto the XY plane: y is up on screen, the camera looks along -z.
    viewer.camera()->set_up_vector(vec3(0, 1, 0));
    viewer.camera()->set_view_direction(vec3(0, 0, -1));
    auto mesh = create_partition_mesh();
    if (!mesh || mesh->empty())
        return EXIT_FAILURE;
    // add the mesh to the viewer.
    viewer.add_model(mesh);

    // show the vertices (as sphere impostors, so they stay round at any zoom level)
    mesh->renderer()->get_points_drawable("vertices")->set_visible(true);
    mesh->renderer()->get_points_drawable("vertices")->set_impostor_type(easy3d::PointsDrawable::SPHERE);
    mesh->renderer()->get_points_drawable("vertices")->set_point_size(8);
    // show the edges (including the diagonals added by the partition)
    mesh->renderer()->get_lines_drawable("edges")->set_visible(true);
    // also show the borders (the outer boundary and the hole boundaries)
    mesh->renderer()->get_lines_drawable("borders")->set_visible(true);

    // Enter the interactive loop (returns when the window is closed).
    return viewer.run();
}
