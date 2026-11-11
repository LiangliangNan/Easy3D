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
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/algo/surface_mesh_subdivision.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_807_Mesh_Subdivision/main.cpp
 *
 * \brief Refining a coarse mesh into a smooth surface with Catmull-Clark, Loop, and sqrt(3) subdivision.
 *
 * <b>Overview</b>
 * This tutorial demonstrates subdivision surfaces with SurfaceMeshSubdivision. Starting from a coarse
 * cube, each shortcut applies one subdivision step (and can be repeated), turning the faceted control
 * mesh into an increasingly smooth limit surface. Three classic schemes are available: Catmull-Clark
 * (for polygon meshes), Loop and sqrt(3) (for triangle meshes). The edges are drawn so the refinement is
 * visible. Subdivision is the standard way to model smooth organic shapes from a coarse control cage and
 * is widely used in animation and CAD.
 *
 * <b>The technique</b>
 * Subdivision repeatedly refines the mesh topology and repositions vertices with weighted averaging
 * rules so that, in the limit, the mesh converges to a smooth surface. Catmull-Clark (Catmull and Clark,
 * 1978) works on arbitrary polygon meshes: each face gains a face point, each edge an edge point, and
 * original vertices are updated; an n-sided face becomes n quads, and the limit surface is bicubic
 * B-spline-like (C2 away from extraordinary vertices). Loop (Loop, 1987) is the triangle analogue: each
 * triangle is split into four by inserting edge midpoints, with weights that approximate a quartic box
 * spline; it is an approximating scheme (the surface generally does not pass through the control
 * vertices). sqrt(3) (Kobbelt, 2000) also operates on triangles but inserts a vertex at each face
 * centroid and flips the old edges, tripling the face count per step (edge length shrinks by sqrt(3)),
 * which gives a slower, more uniform refinement than Loop's factor-of-four. Because Loop and sqrt(3)
 * require triangles, the mesh is triangulated first for those two schemes.
 *
 * <b>Easy3D API</b>
 * - SurfaceMeshSubdivision::catmull_clark() / loop() / sqrt3(): static methods that apply one subdivision
 *   step in place and return success.
 * - SurfaceMesh::triangulate(): converts polygons to triangles before Loop/sqrt(3).
 * - DrawableLines ("edges"): made visible to reveal the refined connectivity.
 * - Renderer::update(): re-extracts the rendering buffers after the connectivity changes.
 * - Viewer::bind_shortcut() / set_usage(): register the Ctrl shortcuts and the help text.
 *
 * <b>Running it</b>
 * Press Ctrl+c for Catmull-Clark, Ctrl+l for Loop, and Ctrl+t for sqrt(3) subdivision; press repeatedly
 * to apply further refinement steps and watch the cube smooth out.
 *
 * \see SurfaceMeshSubdivision, SurfaceMesh, DrawableLines, Tutorial_805_Mesh_Remeshing, Tutorial_806_Mesh_Simplification
 */

using namespace easy3d;

namespace {

    // Apply one subdivision step of the chosen scheme (0 = Catmull-Clark, 1 = Loop, 2 = sqrt(3)).
    bool subdivide(int scheme, Viewer *viewer, Model *model) {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        if (!mesh)
            return false;

        bool ok = false;
        if (scheme == 0) {
            // Catmull-Clark works directly on the polygon (quad) mesh.
            ok = SurfaceMeshSubdivision::catmull_clark(mesh);
        } else {
            // Loop and sqrt(3) are triangle schemes, so triangulate the mesh first.
            mesh->triangulate();
            ok = (scheme == 1) ? SurfaceMeshSubdivision::loop(mesh) : SurfaceMeshSubdivision::sqrt3(mesh);
        }
        if (!ok) {
            std::cerr << "subdivision failed" << std::endl;
            return false;
        }
        mesh->renderer()->update(); // connectivity changed: refresh the rendering buffers
        viewer->update();           // request a redraw
        return true;
    }

} // namespace

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    // A coarse quad cube: the control cage that subdivision will smooth out.
    const std::string file = resource::directory() + "/data/quad_mesh/cube_quad.off";

    // Create the viewer and load the mesh (centered/scaled to fit the scene).
    Viewer viewer(EXAMPLE_TITLE);
    auto mesh = dynamic_cast<SurfaceMesh *>(viewer.add_model(file, true));
    if (!mesh) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    // View the cube from a corner so all three visible faces show the refinement.
    viewer.camera()->setViewDirection(vec3(1, 1, -1));

    // Show the edges so the refinement is visible.
    mesh->renderer()->get_lines_drawable("edges")->set_visible(true);

    // Register one shortcut per subdivision scheme and the on-screen help text.
    viewer.bind_shortcut([](Viewer *v, Model *m) { return subdivide(0, v, m); }, mesh, Viewer::KEY_C, Viewer::MODIF_CTRL);
    viewer.bind_shortcut([](Viewer *v, Model *m) { return subdivide(1, v, m); }, mesh, Viewer::KEY_L, Viewer::MODIF_CTRL);
    viewer.bind_shortcut([](Viewer *v, Model *m) { return subdivide(2, v, m); }, mesh, Viewer::KEY_T, Viewer::MODIF_CTRL);
    viewer.set_usage(
        "default",
        "Ctrl + c: Catmull-Clark subdivision\n" "Ctrl + l: Loop subdivision\n" "Ctrl + t: sqrt(3) subdivision");

    // Enter the interactive loop (returns when the window is closed).
    return viewer.run();
}