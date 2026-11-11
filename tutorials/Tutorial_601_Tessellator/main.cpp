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
#include <easy3d/algo/tessellator.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_601_Tessellator/main.cpp
 *
 * \brief Triangulating general polygonal faces (concave, self-intersecting, and with holes) using the Tessellator.
 *
 * <b>Overview</b>
 * This tutorial builds a tiny SurfaceMesh whose three faces are deliberately "hard" to triangulate —
 * a concave quad, a self-intersecting star, and a quad with a rectangular hole — and then converts the
 * whole mesh into triangles with the Tessellator. It shows the general recipe for turning arbitrary
 * planar contours into a triangle soup that a GPU can actually rasterize.
 *
 * <b>The technique</b>
 * Graphics hardware renders triangles, so any polygon that is not already a triangle must be
 * decomposed. Simple convex polygons are easy (a fan from one vertex), but concave polygons, polygons
 * with holes, and self-intersecting contours break naive fans and ear-clipping. The robust solution,
 * used by the OpenGL Utility (GLU) tessellator and by Easy3D's Tessellator, is a sweep-line algorithm:
 * the contour edges are intersected to form a planar subdivision, and each resulting face is classified
 * as interior or exterior using a winding rule. The winding number of a point counts how many times the
 * contour loops around it; WINDING_NONZERO fills every region with a non-zero count (so a self-intersecting
 * star becomes a solid pentagram), while WINDING_ODD fills only regions encircled an odd number of times
 * (which is how a hole — a contour nested inside the outer boundary — is carved out). All input is first
 * projected onto the plane perpendicular to a supplied normal, and the output triangles are oriented
 * counter-clockwise with respect to that normal. See the OpenGL Programming Guide (the "Red Book"),
 * chapter 11, for the classic description of winding rules and the GLU tessellator.
 *
 * <b>Easy3D API</b>
 * - Tessellator: the workhorse. begin_polygon() sets the plane normal; begin_contour()/add_vertex()/
 *   end_contour() feed one closed loop; set_winding_rule() chooses the fill rule; end_polygon() runs the
 *   sweep. vertices() and elements() return the unique output vertices and triangle indices.
 * - SurfaceMesh: stores the input faces and, after clear(), is rebuilt from the tessellator output.
 *   Face properties "f:normal" and "f:holes" carry the per-face plane normal and optional hole contours.
 * - Viewer, PointsDrawable, LinesDrawable: display the resulting triangles together with their vertices,
 *   edges, and borders.
 *
 * <b>Running it</b>
 * A window opens showing the three tessellated shapes side by side, with spheres at the vertices and the
 * triangle edges and borders drawn. The star is filled solid (non-zero rule) and the third quad has a
 * rectangular hole (odd rule). For general rendering you can also let the tessellator build a
 * DrawableTriangles directly, without explicitly rewriting the mesh.
 *
 * \see Tessellator, SurfaceMesh, Viewer, Tutorial_602_ConvexPartition, Tutorial_603_CSG
 */

using namespace easy3d;

// A hole is stored as an extra closed contour (a list of 3D points) attached to a face.
typedef std::vector<vec3> Hole;

// Convert every face of the mesh into triangles using the tessellator, then rebuild the mesh
// from the tessellator's output vertices and triangle indices.
void triangulate(SurfaceMesh *mesh) {
    if (!mesh)
        return;

    // The tessellator projects each face onto a plane, so it needs a reliable per-face normal.
    mesh->update_face_normals();
    auto normals = mesh->face_property<vec3>("f:normal");
    // Optional property: faces may carry hole contours (absent for most meshes).
    auto holes = mesh->get_face_property<Hole>("f:holes");

    Tessellator tessellator;
    for (auto f : mesh->faces()) {
        // Start a new complex polygon lying in the plane with this normal. Output triangles
        // will be oriented counter-clockwise with respect to it.
        tessellator.begin_polygon(normals[f]);

        // The outer boundary: fill every region with non-zero winding number. This also fills
        // self-intersecting contours (e.g. the star below) into a solid shape.
        tessellator.set_winding_rule(Tessellator::WINDING_NONZERO);
        tessellator.begin_contour();
        for (auto h : mesh->halfedges(f)) {
            SurfaceMesh::Vertex v = mesh->target(h);
            // Passing v.idx() lets us map tessellator output vertices back to the original mesh
            // vertices; vertices created at edge intersections get index -1 instead.
            tessellator.add_vertex(mesh->position(v), v.idx());
        }
        tessellator.end_contour();

        if (holes && holes[f].size() >= 3) { // has a valid hole
            // A hole is just another contour of the same complex polygon. With the odd winding
            // rule, the region inside the hole is encircled twice (even) and stays unfilled.
            tessellator.set_winding_rule(Tessellator::WINDING_ODD);
            tessellator.begin_contour();
            for (const auto &p : holes[f])
                tessellator.add_vertex(p);
            tessellator.end_contour();
        }

        // Run the sweep-line tessellation for this face.
        tessellator.end_polygon();
    }

    // now the tessellation is done. We can clear the old mesh and
    // fill it will the new set of triangles

    mesh->clear();

    const auto &triangles = tessellator.elements();
    if (triangles.empty())
        return; // in degenerate cases num can be zero

    // The tessellator welds duplicate positions into unique vertices, so the element (index)
    // buffer can reference them without sending repeated coordinates to the GPU.
    const auto &vts = tessellator.vertices();
    for (auto v : vts)
        mesh->add_vertex(vec3(v->data()));

    // Each element is a triple of indices into the vertex list above.
    for (const auto &t : triangles) {
        mesh->add_triangle(SurfaceMesh::Vertex(static_cast<int>(t[0])), SurfaceMesh::Vertex(static_cast<int>(t[1])),
                           SurfaceMesh::Vertex(static_cast<int>(t[2])));
    }
}

std::shared_ptr<SurfaceMesh> create_tessellation_mesh() {
    //-------- create a simple mesh with 3 complex faces ---------
    // All geometry lies in the z = 0 plane so the tessellator works in a well-defined 2D plane.

    auto mesh = std::make_shared<SurfaceMesh>();

    { // face 1: a concave quad
        // v3 dents inward, so a naive triangle fan would produce triangles outside the face.
        auto v0 = mesh->add_vertex(vec3(0, 0, 0));
        auto v1 = mesh->add_vertex(vec3(800, 0, 0));
        auto v2 = mesh->add_vertex(vec3(800, 800, 0));
        auto v3 = mesh->add_vertex(vec3(600, 300, 0));
        mesh->add_quad(v0, v1, v2, v3);
    }

    { // face 2: a self-intersecting face representing a star
        // The vertex order makes the boundary cross itself (a pentagram). The non-zero winding
        // rule fills the whole star, including the central pentagon.
        auto vertices = {mesh->add_vertex(vec3(1500, 0, 0)), mesh->add_vertex(vec3(1300, 800, 0)),
                         mesh->add_vertex(vec3(1100, 0, 0)), mesh->add_vertex(vec3(1700, 500, 0)),
                         mesh->add_vertex(vec3(900, 500, 0))};
        mesh->add_face(vertices);
    }

    { // face 3: a quad face with a hole
        auto vertices = {mesh->add_vertex(vec3(1800, 0, 0)), mesh->add_vertex(vec3(2200, 0, 0)),
                         mesh->add_vertex(vec3(2200, 700, 0)), mesh->add_vertex(vec3(1800, 700, 0))};
        auto f = mesh->add_face(vertices);

        // let's create a hole (also a quad shape) in this face
        // SurfaceMesh has no native hole representation, so we attach the hole contour as a
        // custom face property; triangulate() feeds it to the tessellator as a second contour.
        auto holes = mesh->add_face_property<Hole>("f:holes");
        holes[f] = {vec3(1900, 100, 0), vec3(2100, 100, 0), vec3(2100, 600, 0), vec3(1900, 600, 0)};
    }

    //-------- triangulate the mesh using the tessellator ---------

    triangulate(mesh.get());

    // ------------------------------------------------------------

    return mesh;
}

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();
    Viewer viewer(EXAMPLE_TITLE);

    // Look straight down onto the XY plane: y is up on screen, the camera looks along -z.
    viewer.camera()->setUpVector(vec3(0, 1, 0));
    viewer.camera()->setViewDirection(vec3(0, 0, -1));
    auto mesh = create_tessellation_mesh();
    if (!mesh || mesh->empty())
        return EXIT_FAILURE;
    // add the model to the viewer (true: center and scale it to fit the scene).
    viewer.add_model(mesh, true);

    // show the vertices (as sphere impostors, so they stay round at any zoom level)
    mesh->renderer()->get_points_drawable("vertices")->set_visible(true);
    mesh->renderer()->get_points_drawable("vertices")->set_impostor_type(easy3d::PointsDrawable::SPHERE);
    mesh->renderer()->get_points_drawable("vertices")->set_point_size(8);
    // show the edges (the triangulation produced by the tessellator)
    mesh->renderer()->get_lines_drawable("edges")->set_visible(true);
    // also show the borders (the outer silhouette, including the hole boundary)
    mesh->renderer()->get_lines_drawable("borders")->set_visible(true);

    // Enter the interactive loop (returns when the window is closed).
    return viewer.run();
}
