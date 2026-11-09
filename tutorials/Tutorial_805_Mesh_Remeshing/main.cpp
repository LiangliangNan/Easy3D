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
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/algo/surface_mesh_remeshing.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_805_Mesh_Remeshing/main.cpp
 *
 * \brief Improving triangle quality with uniform and curvature-adaptive incremental remeshing.
 *
 * <b>Overview</b>
 * This tutorial demonstrates surface remeshing with SurfaceMeshRemeshing. Remeshing rebuilds the
 * triangulation of a surface so that the triangles are well-shaped and appropriately sized, while
 * staying close to the original geometry. Two schemes are shown on the bunny: uniform remeshing toward
 * a single target edge length, and adaptive remeshing that puts smaller triangles where the surface
 * curves sharply. The edges are drawn so the new triangulation is visible. Remeshing is a key
 * preprocessing step for simulation, deformation, and multiresolution modeling, where poor or uneven
 * triangles cause artifacts.
 *
 * <b>The technique</b>
 * Easy3D uses incremental remeshing (Botsch and Kobbelt, "A remeshing approach to multiresolution
 * modeling", SGP 2004): it repeatedly applies four local operations — split edges that are too long,
 * collapse edges that are too short, flip edges to improve valence/shape (toward the Delaunay-like
 * optimum), and relax vertices tangentially to equalize triangle areas — followed by back-projection
 * onto the original surface so the shape is preserved. Uniform remeshing drives every edge toward one
 * target length, giving an isotropic mesh. Adaptive remeshing (Dunyach et al., "Adaptive remeshing for
 * real-time mesh deformation", EG 2013) instead uses a curvature-based sizing field: the local target
 * edge length shrinks where curvature is high, bounded by a minimum/maximum edge length and a maximum
 * approximation error, so detail is concentrated where it matters and flat regions stay coarse.
 *
 * <b>Easy3D API</b>
 * - SurfaceMeshRemeshing::uniform_remeshing(edge_length): isotropic remeshing toward a target length.
 * - SurfaceMeshRemeshing::adaptive_remeshing(min_len, max_len, approx_error): curvature-driven remeshing
 *   within the given edge-length bounds and approximation tolerance (lengths here are fractions of the
 *   bounding-box diagonal so they scale with the model).
 * - DrawableLines ("edges"): made visible to reveal the triangulation.
 * - Renderer::update(): re-extracts the rendering buffers after the connectivity changes.
 * - Viewer::bind() / set_usage(): register the Ctrl shortcuts and the help text.
 *
 * <b>Running it</b>
 * Press Ctrl+u for uniform remeshing (target = 1.5 x the current average edge length) and Ctrl+a for
 * adaptive remeshing; the console prints the resulting vertex/face counts after each operation.
 *
 * \see SurfaceMeshRemeshing, SurfaceMesh, DrawableLines, Tutorial_806_Mesh_Simplification, Tutorial_807_Mesh_Subdivision
 */

using namespace easy3d;

namespace {

    // Compute the mean edge length of the mesh; used to derive a scale-relative target for uniform
    // remeshing so the result is neither too coarse nor too fine for this particular model.
    float average_edge_length(const SurfaceMesh *mesh) {
        float len = 0.0f;
        for (auto e : mesh->edges())
            len += distance(mesh->position(mesh->vertex(e, 0)), mesh->position(mesh->vertex(e, 1)));
        return mesh->n_edges() > 0 ? len / static_cast<float>(mesh->n_edges()) : 0.0f;
    }

    // Shortcut handler: isotropic remeshing toward 1.5 x the current average edge length.
    bool uniform_remesh(Viewer *viewer, Model *model) {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        if (!mesh)
            return false;
        const float target = average_edge_length(mesh) * 1.5f;
        SurfaceMeshRemeshing remeshing(mesh);
        remeshing.uniform_remeshing(target); // split/collapse/flip/relax + back-projection
        std::cout << "uniform remeshing: " << mesh->n_vertices() << " vertices / " << mesh->n_faces() << " faces"
                  << std::endl;
        mesh->renderer()->update(); // connectivity changed: refresh the rendering buffers
        viewer->update();           // request a redraw
        return true;
    }

    // Shortcut handler: curvature-adaptive remeshing. The three lengths are fractions of the
    // bounding-box diagonal: minimum edge length, maximum edge length, and max approximation error.
    bool adaptive_remesh(Viewer *viewer, Model *model) {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        if (!mesh)
            return false;
        const float bb = mesh->bounding_box().diagonal_length();
        SurfaceMeshRemeshing remeshing(mesh);
        remeshing.adaptive_remeshing(0.001f * bb, 0.100f * bb, 0.001f * bb);
        std::cout << "adaptive remeshing: " << mesh->n_vertices() << " vertices / " << mesh->n_faces() << " faces"
                  << std::endl;
        mesh->renderer()->update(); // connectivity changed: refresh the rendering buffers
        viewer->update();
        return true;
    }

} // namespace

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    const std::string file = resource::directory() + "/data/bunny.ply";

    // Create the viewer and load the mesh (centered/scaled to fit the scene).
    Viewer viewer(EXAMPLE_TITLE);
    auto mesh = dynamic_cast<SurfaceMesh *>(viewer.add_model(file, true));
    if (!mesh) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    viewer.camera()->setViewDirection(vec3(0, 1, 0));

    // Show the edges so the new triangulation is visible.
    mesh->renderer()->get_lines_drawable("edges")->set_visible(true);

    // Register the remeshing shortcuts and the on-screen help text.
    viewer.bind([](Viewer *v, Model *m) { return uniform_remesh(v, m); }, mesh, Viewer::KEY_U, Viewer::MODIF_CTRL);
    viewer.bind([](Viewer *v, Model *m) { return adaptive_remesh(v, m); }, mesh, Viewer::KEY_A, Viewer::MODIF_CTRL);
    viewer.set_usage(
        "default",
        "Ctrl + u: uniform remeshing (target = 1.5 x average edge length)\n" "Ctrl + a: adaptive remeshing (curvature-driven)");

    // Enter the interactive loop (returns when the window is closed).
    return viewer.run();
}