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
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_302_Imposters/main.cpp
 *
 * \brief Rendering points as spheres/surfels and lines as cylinders/cones using imposters.
 *
 * <b>Overview</b>
 * This tutorial shows how to give simple primitives a richer appearance with imposters:
 * mesh vertices are drawn as shaded spheres, mesh edges as solid cylinders, and vertex
 * normals as cones. A second copy of the mesh, shifted sideways, shows the same vertices
 * rendered as surfels (oriented disks). All of this is done by switching a style flag on
 * PointsDrawable / LinesDrawable — no extra geometry is generated.
 *
 * <b>The technique</b>
 * An imposter (or "sprite") is a cheap primitive — a point or a line — that the fragment
 * shader makes look like a more complex 3D object. For sphere imposters, each point is
 * rasterized as a small screen-facing quad; the shader then computes, per pixel, the surface
 * of the sphere that the quad represents: it discards pixels outside the circular silhouette,
 * reconstructs the sphere's 3D normal and depth, and shades it with lighting. The result looks
 * like a real lit sphere at a fraction of the cost of tessellating one. The same idea turns a
 * line segment into a cylinder (constant radius, flat caps) or a cone (tapering from the first
 * endpoint to the second) — ideal for drawing edges and vector arrows. A surfel ("surface
 * element") is an oriented disk: it uses the point's normal to tilt the disk so it lies in the
 * local tangent plane, which makes point clouds look like sampled surface patches. Because
 * imposters are analytic, they stay perfectly smooth at any zoom level. See e.g. the classic
 * "GPU Gems" chapters on sprite/sphere rendering for the underlying ray-intersection math.
 *
 * <b>Easy3D API</b>
 * - PointsDrawable: set_impostor_type() chooses PLAIN, SPHERE, or SURFEL; set_point_size()
 *   sets the diameter in logical pixels (DPI-independent); set_uniform_coloring() sets one RGBA color.
 * - LinesDrawable: set_impostor_type() chooses PLAIN, CYLINDER, or CONE; set_line_width()
 *   sets the diameter in logical pixels (DPI-independent).
 * - Renderer: each model owns one; add_points_drawable()/add_lines_drawable() create extra
 *   drawables attached to the model (here "vertices", "edges", "normals").
 * - SurfaceMesh: update_vertex_normals() computes per-vertex normals; the "v:point" and
 *   "v:normal" properties give positions and normals used to build the normal segments.
 *
 * <b>Running it</b>
 * The window shows two spheres side by side: on the left, red sphere vertices, orange
 * cylinder edges, and green cone normals; on the right, the same mesh with red surfel
 * vertices. Navigate with the standard Viewer controls (left-drag rotate, scroll zoom,
 * 'h' for help).
 *
 * \see PointsDrawable, LinesDrawable, Renderer, SurfaceMesh, Tutorial_301_Drawables, Tutorial_304_VectorField
 */

using namespace easy3d;

// Render mesh vertices as spheres. The renderer pulls the vertex positions straight from the
// mesh, so no explicit buffer upload is needed — only the style is configured here.
void create_spheres(SurfaceMesh *mesh) {
    // Create (or fetch) the "vertices" points drawable attached to the mesh's renderer.
    PointsDrawable *drawable = mesh->renderer()->add_points_drawable("vertices");
    drawable->set_uniform_coloring(vec4(1.0f, 0.0f, 0.0f, 1.0f)); // opaque red
    drawable->set_point_size(16.0f); // diameter in logical pixels (DPI-independent)
    // SPHERE: each point is shaded as a lit sphere imposter instead of a flat square.
    drawable->set_impostor_type(PointsDrawable::SPHERE);
}

// Render mesh edges as cylinders: each edge becomes a solid tube of constant radius.
void create_cylinders(SurfaceMesh *mesh) {
    LinesDrawable *drawable = mesh->renderer()->add_lines_drawable("edges");
    drawable->set_uniform_coloring(vec4(1.0f, 0.67f, 0.5f, 1.0f)); // light orange
    drawable->set_impostor_type(LinesDrawable::CYLINDER);
    drawable->set_line_width(6); // tube diameter in logical pixels (DPI-independent)
}

// Render the vertex normals as cones: a tapering imposter that reads as an arrow/direction.
void create_cones(SurfaceMesh *mesh) {
    // Vertex positions and normals are stored as mesh properties ("v:point" / "v:normal").
    auto points = mesh->get_vertex_property<vec3>("v:point");
    // Compute per-vertex normals (averaged from the adjacent face normals).
    mesh->update_vertex_normals();
    auto normals = mesh->get_vertex_property<vec3>("v:normal");

    // Get the bounding box of the model. Then we defined the length of the
    // normal vectors to be 15% of the bounding box diagonal.
    // Scaling by the model size keeps the glyphs visible but not overwhelming.
    float length = mesh->bounding_box().diagonal_length() * 0.15f;

    // Now let collects the two end points of each normal vector. So from
    // these points we can create a drawable to visualize the normal vectors.

    // Every consecutive two points represent a normal vector: the segment starts at the
    // vertex and ends one "length" away along the normal. Cones taper from the first
    // endpoint (base) to the second (tip), so the tip points along the normal direction.
    std::vector<vec3> normal_points;
    for (auto v : mesh->vertices()) {
        const vec3 &s = points[v];
        const vec3 &t = points[v] + normals[v] * length;
        normal_points.push_back(s);
        normal_points.push_back(t);
    }

    LinesDrawable *drawable = mesh->renderer()->add_lines_drawable("normals");
    // This drawable is not one of the mesh's built-ins, so its segment endpoints must be
    // uploaded explicitly.
    drawable->update_vertex_buffer(normal_points);
    drawable->set_uniform_coloring(vec4(0.0f, 1.0f, 0.0f, 1.0f)); // opaque green
    drawable->set_impostor_type(LinesDrawable::CONE);
    drawable->set_line_width(8); // base diameter in logical pixels (DPI-independent)
}

// Render mesh vertices as surfels: oriented disks that lie in the local tangent plane.
// The disk orientation comes from each vertex's normal ("v:normal"); the mesh copy used in
// main() inherits the normals computed by create_cones(), so they are already available.
void create_surfels(SurfaceMesh *mesh) {
    PointsDrawable *drawable = mesh->renderer()->add_points_drawable("vertices");
    drawable->set_uniform_coloring(vec4(1.0f, 0.0f, 0.0f, 1.0f));
    drawable->set_point_size(16.0f);
    drawable->set_impostor_type(PointsDrawable::SURFEL);
}

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    // Create the default Easy3D viewer.
    // Note: a viewer must be created before creating any drawables.
    Viewer viewer(EXAMPLE_TITLE);

    // Load a surface mesh (a tessellated sphere) from a file.
    const std::string file_name = resource::directory() + "/data/sphere.obj";

    // Here the 'false' flag tells the viewer to skip creating the default drawables.
    // Instead, we will create them by ourselves.
    // add_model() returns a generic Model pointer; dynamic_cast checks it is a SurfaceMesh.
    auto mesh = dynamic_cast<SurfaceMesh *>(viewer.add_model(file_name, false));
    if (!mesh) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    //--------------------- render vertices as spheres ----------------

    create_spheres(mesh);

    //--------------------- render edges as cylinders -----------------

    create_cylinders(mesh);

    //--------------------- render normals as cones -------------------

    create_cones(mesh);

    //-------------------- render vertices as surfels -----------------

    // make a copy of the mesh (the copy also carries over the computed vertex normals)
    auto copy = new SurfaceMesh(*mesh);
    // translate the mesh a bit so we can see both: shift it along Y by 70% of the bounding
    // box diagonal, which places the surfel version next to the sphere/cylinder version.
    const vec3 trans = vec3(0, 1, 0) * mesh->bounding_box().diagonal_length() * 0.7f;
    auto points = copy->get_vertex_property<vec3>("v:point");
    for (auto v : copy->vertices())
        points[v] += trans;
    // Add the translated copy as a second model (again without default drawables).
    viewer.add_model(std::shared_ptr<SurfaceMesh>(copy), false);

    create_surfels(copy);

    // ----------------------------------------------------------------

    // run the viewer
    return viewer.run();
}
