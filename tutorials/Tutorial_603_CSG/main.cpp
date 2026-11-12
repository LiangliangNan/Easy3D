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
#include <easy3d/core/types.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/algo/tessellator.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_603_CSG/main.cpp
 *
 * \brief Computing 2D constructive solid geometry (union, intersection, difference) on polygons with the csg operators.
 *
 * <b>Overview</b>
 * This tutorial computes the three classic 2D boolean operations on two overlapping squares using the
 * csg operators provided in easy3d/algo/tessellator.h. The result is shown in a 2 x 2 MultiViewer so the
 * inputs and the three operations can be compared side by side:
 *      - top-left:     the two input polygons A (red) and B (blue);
 *      - top-right:    union (A | B);
 *      - bottom-left:  intersection (A & B);
 *      - bottom-right: difference (A - B).
 *
 * <b>The technique</b>
 * Constructive solid geometry builds complex shapes by combining simpler ones with set operations.
 * In 2D these are exactly the boolean operations on regions: union (A | B) is the area covered by either,
 * intersection (A & B) the area covered by both, and difference (A - B) the part of A not covered by B.
 * The csg operators accept polygons of unknown structure (e.g., self-intersecting or overlapping contours)
 * and return simple contours: counter-clockwise contours define the outer boundary and clockwise contours
 * define holes. Internally they reuse the same sweep-line + winding-number machinery as the Tessellator
 * (see Tutorial_601_Tessellator): each operation corresponds to a different winding rule for classifying
 * the interior of the combined contours — union keeps regions with non-zero winding, intersection keeps
 * regions covered by both inputs, and difference subtracts one region's winding from the other's. The
 * returned simple contours are then filled into a triangle mesh with the Tessellator for display.
 * See the OpenGL Programming Guide (the "Red Book"), chapter 11, for the winding-rule formulation of CSG.
 *
 * <b>Easy3D API</b>
 * - csg::union_of(), csg::intersection_of(), csg::difference_of(): the boolean operators. They take/return
 *   Polygon2 contours (a GenericPolygon of vec2). union_of() merges a whole vector in place.
 * - Tessellator: fills the resulting simple contours (CCW outer, CW holes) into triangles.
 * - SurfaceMesh, DrawableTriangles, LinesDrawable: store and display the filled result and its boundary.
 * - MultiViewer: a grid of synchronized views; assign(row, col, model) places a model in one cell.
 *
 * <b>Running it</b>
 * A 2 x 2 window opens comparing the inputs (top-left) with union (top-right), intersection (bottom-left),
 * and difference (bottom-right). All views share navigation. See Tutorial_602_ConvexPartition for
 * partitioning the resulting simple polygons into convex pieces.
 *
 * \see Tessellator, MultiViewer, SurfaceMesh, Tutorial_601_Tessellator, Tutorial_602_ConvexPartition
 */

using namespace easy3d;

namespace {

    // Two overlapping squares (both in counter-clockwise order).
    const std::vector<vec2> square_a = {vec2(0, 0), vec2(400, 0), vec2(400, 400), vec2(0, 400)};
    const std::vector<vec2> square_b = {vec2(200, 200), vec2(600, 200), vec2(600, 600), vec2(200, 600)};

    enum Operation { CSG_UNION, CSG_INTERSECTION, CSG_DIFFERENCE };

    // Run one boolean operation on the two squares and return the resulting simple contours.
    std::vector<Polygon2> compute(Operation op) {
        Polygon2 a(square_a.begin(), square_a.end());
        Polygon2 b(square_b.begin(), square_b.end());
        std::vector<Polygon2> result;
        switch (op) {
        case CSG_UNION:
            // Union = tessellate all contours together with the non-zero winding rule:
            // any region covered by at least one input polygon is kept.
            result = {a, b};
            csg::union_of(result);
            break;
        case CSG_INTERSECTION:
            // Intersection = keep only regions covered by both inputs (winding number >= 2
            // in absolute value, i.e. WINDING_ABS_GEQ_TWO).
            csg::intersection_of(a, b, result);
            break;
        case CSG_DIFFERENCE:
            // Difference = A with B subtracted: the operator reverses one contour's orientation
            // and extracts the regions with positive winding number.
            csg::difference_of(a, b, result);
            break;
        }
        return result;
    }

    // Fill the resulting contours (CCW outer boundaries, CW holes) into a triangle mesh.
    void build_mesh(const std::vector<Polygon2> &contours, SurfaceMesh *mesh) {
        mesh->clear();

        // All contours belong to one complex polygon in the z = 0 plane (normal +z). The CSG
        // output is already simple, so the non-zero rule fills outer boundaries and leaves
        // the clockwise hole contours empty.
        Tessellator tessellator;
        tessellator.begin_polygon(vec3(0, 0, 1));
        tessellator.set_winding_rule(Tessellator::WINDING_NONZERO);
        for (const auto &contour : contours) {
            tessellator.begin_contour();
            for (const auto &p : contour)
                tessellator.add_vertex(vec3(p, 0.0));
            tessellator.end_contour();
        }
        tessellator.end_polygon();

        // Rebuild the mesh from the tessellator's unique vertices and triangle indices.
        for (auto v : tessellator.vertices())
            mesh->add_vertex(vec3(v->data()));
        for (const auto &t : tessellator.elements()) {
            mesh->add_triangle(SurfaceMesh::Vertex(static_cast<int>(t[0])), SurfaceMesh::Vertex(static_cast<int>(t[1])),
                               SurfaceMesh::Vertex(static_cast<int>(t[2])));
        }
    }

    // Show the boundary of a CSG result so its shape is easy to read.
    void set_boundary_visible(SurfaceMesh *mesh) {
        auto borders = mesh->renderer()->get_lines_drawable("borders");
        borders->set_line_width(2.0f);
        borders->set_visible(true);
    }

} // namespace

// The default (union) result, also used by the geometry tutorial test.
std::shared_ptr<SurfaceMesh> create_csg_mesh() {
    auto mesh = std::make_shared<SurfaceMesh>();
    build_mesh(compute(CSG_UNION), mesh.get());
    return mesh;
}

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    // A 2 x 2 grid of views sharing one camera, so the inputs and the three boolean
    // results can be compared side by side under identical navigation.
    MultiViewer viewer(2, 2, EXAMPLE_TITLE);
    // Look straight down onto the XY plane: y is up on screen, the camera looks along -z.
    viewer.camera()->set_up_vector(vec3(0, 1, 0));
    viewer.camera()->set_view_direction(vec3(0, 0, -1));

    // -------------------------------------------------------------------------
    // view (0, 0): the two input polygons, in distinct colors.
    auto input_a = std::make_shared<SurfaceMesh>();
    auto input_b = std::make_shared<SurfaceMesh>();
    build_mesh({Polygon2(square_a.begin(), square_a.end())}, input_a.get());
    build_mesh({Polygon2(square_b.begin(), square_b.end())}, input_b.get());
    viewer.add_model(input_a);
    viewer.add_model(input_b);
    set_boundary_visible(input_a.get());
    set_boundary_visible(input_b.get());

    // Lift B slightly toward the camera so the overlapping region does not z-fight
    // and B is drawn on top of A there (the intersection is shown explicitly below).
    auto points_b = input_b->get_vertex_property<vec3>("v:point");
    for (auto v : input_b->vertices())
        points_b[v].z += 1.0f;

    // Color A red and B blue so the two inputs are easy to tell apart in the top-left view.
    input_a->renderer()->get_triangles_drawable("faces")->set_uniform_coloring(vec4(0.90f, 0.30f, 0.25f, 1.0f));
    input_b->renderer()->get_triangles_drawable("faces")->set_uniform_coloring(vec4(0.25f, 0.45f, 0.90f, 1.0f));
    // assign(row, col, model) pins a model to one cell of the grid; both inputs share cell (0, 0).
    viewer.assign(0, 0, input_a.get());
    viewer.assign(0, 0, input_b.get());

    // -------------------------------------------------------------------------
    // view (0, 1): union; view (1, 0): intersection; view (1, 1): difference (A - B).
    auto union_mesh = create_csg_mesh();
    auto intersection_mesh = std::make_shared<SurfaceMesh>();
    build_mesh(compute(CSG_INTERSECTION), intersection_mesh.get());
    auto difference_mesh = std::make_shared<SurfaceMesh>();
    build_mesh(compute(CSG_DIFFERENCE), difference_mesh.get());
    viewer.add_model(union_mesh);
    viewer.add_model(intersection_mesh);
    viewer.add_model(difference_mesh);
    set_boundary_visible(union_mesh.get());
    set_boundary_visible(intersection_mesh.get());
    set_boundary_visible(difference_mesh.get());
    // Each boolean result is pinned to its own cell for the side-by-side comparison.
    viewer.assign(0, 1, union_mesh.get());
    viewer.assign(1, 0, intersection_mesh.get());
    viewer.assign(1, 1, difference_mesh.get());

    // Explain the four cells in the viewer's on-screen help.
    viewer.set_usage(
        "default",
        "Top-left: the two input polygons A (red) and B (blue)\n" "Top-right: union (A | B)\n" "Bottom-left: intersection (A & B)\n" "Bottom-right: difference (A - B)");

    // Enter the interactive loop (returns when the window is closed).
    return viewer.run();
}