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
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_304_VectorField/main.cpp
 *
 * \brief Visualizing a vector field on a surface mesh as line-segment glyphs (face normals).
 *
 * <b>Overview</b>
 * This tutorial renders a vector field defined on a surface mesh: the normal direction of
 * every face of a sphere model. Each vector is drawn as a short green line segment starting
 * at the face center and pointing along the normal — the classic "glyph" visualization of
 * direction fields. The mesh edges are shown as well, so the faceting is easy to read.
 *
 * <b>The technique</b>
 * A vector field assigns a direction (and possibly a magnitude) to each element of a domain;
 * here the domain is the mesh surface and the vectors are the per-face normals. The standard
 * way to display such a field is with glyphs: small geometric marks placed at the sample
 * locations whose orientation encodes the vector direction. A line segment from the sample
 * point s to s + v * length is the simplest glyph — its direction shows the vector, and a
 * common length keeps the display readable. Two practical details matter: (1) the glyph
 * length is chosen relative to the model size (5% of the bounding-box diagonal here) so the
 * arrows neither vanish nor dominate the shape; (2) the segment's start point is the face
 * centroid (average of its vertices), which places the glyph on the surface it describes.
 * The same pattern works for any vector data — velocities, displacements, gradients — by
 * substituting your own vectors for the normals. For thicker, arrow-like glyphs, the segments
 * can be rendered as cone imposters instead (see Tutorial_302_Imposters).
 *
 * <b>Easy3D API</b>
 * - SurfaceMesh: update_face_normals() computes the "f:normal" property; vertices(f) iterates
 *   the corners of a face; bounding_box() gives the model extents.
 * - Renderer: add_lines_drawable() creates an extra LinesDrawable attached to the model;
 *   get_lines_drawable("edges") retrieves the built-in edge drawable.
 * - LinesDrawable: update_vertex_buffer() uploads the segment endpoints (each consecutive
 *   pair of points is one line); set_uniform_coloring() and set_line_width() style them.
 * - DrawableTriangles: set_smooth_shading(false) keeps the faceted look so each face's normal
 *   glyph visibly matches its flat facet.
 *
 * <b>Running it</b>
 * The window shows the faceted sphere with a green normal segment sticking out of every face
 * and the mesh edges overlaid. Navigate with the standard Viewer controls ('h' for help).
 *
 * \see LinesDrawable, SurfaceMesh, Renderer, Tutorial_302_Imposters, Tutorial_303_ScalarField
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    // Create the default Easy3D viewer.
    // Note: a viewer must be created before creating any drawables.
    Viewer viewer(EXAMPLE_TITLE);

    // Load a surface mesh (a tessellated sphere) from a file. The 'true' flag centers and
    // scales the model to fit the scene.
    const std::string file_name = resource::directory() + "/data/sphere.obj";
    auto mesh = dynamic_cast<SurfaceMesh *>(viewer.add_model(file_name, true));
    if (!mesh) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    // Keep this face-normal example uniformly colored and visibly faceted,
    // regardless of texture coordinates stored in the example OBJ file.
    // Flat shading makes each facet's constant normal match its glyph direction.
    auto faces = mesh->renderer()->get_triangles_drawable("faces");
    faces->set_uniform_coloring(faces->color());
    faces->set_smooth_shading(false);

    // Get the bounding box of the model. Then we defined the length of the
    // normal vectors to be 5% of the bounding box diagonal.
    // A model-relative length keeps the glyphs visible without hiding the shape.
    const Box3 &box = mesh->bounding_box();
    float length = norm(box.max_point() - box.min_point()) * 0.05f;

    // Compute the face normals. They are stored in the per-face property "f:normal".
    mesh->update_face_normals();
    auto normals = mesh->get_face_property<vec3>("f:normal");

    // Every consecutive two points represent a normal vector: the glyph runs from the face
    // centroid s to the tip t = s + normal * length.
    std::vector<vec3> points;
    for (auto f : mesh->faces()) {
        // The centroid is the average of the face's corner positions; it anchors the glyph
        // on the surface, independent of how many corners the face has.
        vec3 center(0, 0, 0); // face center
        int count = 0;
        for (auto v : mesh->vertices(f)) {
            center += mesh->position(v);
            ++count;
        }

        const vec3 s = center / count;
        const vec3 t = s + normals[f] * length;
        points.push_back(s);
        points.push_back(t);
    }

    // Create a drawable for rendering the normal vectors.
    auto drawable = mesh->renderer()->add_lines_drawable("normals");
    // Upload the data to the GPU. No element buffer is needed: consecutive vertex pairs
    // are interpreted as line segments automatically.
    drawable->update_vertex_buffer(points);

    // We will draw the normal vectors in a uniform green color
    drawable->set_uniform_coloring(vec4(0.0f, 1.0f, 0.0f, 1.0f));

    // Set the line width (in pixels) so the glyphs stay visible at any zoom level.
    drawable->set_line_width(1.0f);

    // Also show the standard "edges" (hidden by default) to make the faceting explicit.
    mesh->renderer()->get_lines_drawable("edges")->set_visible(true);

    // Enter the interactive loop (returns when the window is closed).
    return viewer.run();
}
