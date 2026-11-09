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
#include <easy3d/algo/surface_mesh_hole_filling.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_804_Mesh_HoleFilling/main.cpp
 *
 * \brief Closing the boundary loops (holes) of an open triangle mesh with a smooth filled patch.
 *
 * <b>Overview</b>
 * This tutorial demonstrates hole filling with SurfaceMeshHoleFilling. The bunny mesh is not watertight
 * — it has open boundary loops (for example around its base). Those loops are highlighted in red with
 * the "borders" drawable so you can see exactly what will be closed. Pressing the shortcut triangulates
 * each hole and then refines and fairs the new patch so it blends smoothly into the surrounding
 * surface. Hole filling is a common repair step for scanned meshes that have missing data.
 *
 * <b>The technique</b>
 * A hole is a boundary loop of manifold vertices. Filling it well is more than just triangulating the
 * loop: a naive fan triangulation creates poor-quality, visible patches. Easy3D follows Peter Liepa,
 * "Filling holes in meshes" (SGP 2003), a three-stage pipeline. (1) Triangulate the hole with a dynamic-
 * programming search that minimizes a combined angle/area weight, avoiding sharp dihedral angles and
 * large flat triangles. (2) Refine the patch by isometric remeshing (splitting long edges, collapsing
 * short ones, flipping, and relaxing) so the new triangles match the density and quality of the existing
 * mesh. (3) Fair the patch by minimizing curvature while keeping the original boundary fixed, so the
 * filled region joins the surrounding surface smoothly rather than as a flat cap. The size argument
 * limits filling to holes whose boundary loop has at most that many edges.
 *
 * <b>Easy3D API</b>
 * - SurfaceMeshHoleFilling::fill_holes(size): fills every hole whose boundary loop has <= size edges and
 *   returns the number of holes filled (fill_hole() fills a single hole given a boundary halfedge).
 * - DrawableLines ("borders"): the renderer's boundary-loop drawable; set_visible(), set_uniform_coloring(),
 *   and set_line_width() make the holes easy to spot before filling.
 * - Renderer::update(): re-extracts the rendering buffers after the connectivity and positions change.
 * - Viewer::bind() / set_usage(): register the Ctrl shortcut and the help text.
 *
 * <b>Running it</b>
 * The boundary loops are drawn in red. Press Ctrl+f to fill the holes; the console reports how many
 * holes were filled and the red borders disappear as the mesh becomes closed.
 *
 * \see SurfaceMeshHoleFilling, SurfaceMesh, DrawableLines, Tutorial_803_Mesh_Fairing, Tutorial_805_Mesh_Remeshing
 */

using namespace easy3d;

namespace {

    // Shortcut handler: close every hole whose boundary loop has at most 1000 edges.
    bool fill_holes(Viewer *viewer, Model *model) {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        if (!mesh)
            return false;
        // fill_holes() triangulates each hole, then refines and fairs the patch (Liepa 2003).
        SurfaceMeshHoleFilling filling(mesh);
        const int n = filling.fill_holes(1000);
        std::cout << "filled " << n << " hole(s)" << std::endl;
        mesh->renderer()->update(); // connectivity/positions changed: refresh the rendering buffers
        viewer->update();           // request a redraw (the red borders are now gone)
        return true;
    }

} // namespace

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    // The bunny has open boundary loops (holes), e.g. around its base.
    const std::string file = resource::directory() + "/data/bunny.ply";

    // Create the viewer and load the mesh (centered/scaled to fit the scene).
    Viewer viewer(EXAMPLE_TITLE);
    auto mesh = dynamic_cast<SurfaceMesh *>(viewer.add_model(file, true));
    if (!mesh) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    // Turn the camera to look at the bottom of the bunny model
    viewer.camera()->setUpVector(vec3(0, 1, 0));
    viewer.camera()->setViewDirection(vec3(0, 0, 1));

    // Highlight the boundary loops (the holes to be filled). "borders" is the renderer's built-in
    // lines drawable for boundary edges; color it red and thicken it so the holes stand out.
    auto borders = mesh->renderer()->get_lines_drawable("borders");
    borders->set_visible(true);
    borders->set_uniform_coloring(vec4(1.0f, 0.0f, 0.0f, 1.0f));
    borders->set_line_width(3.0f);

    // Register the fill shortcut and the on-screen help text.
    viewer.bind([](Viewer *v, Model *m) { return fill_holes(v, m); }, mesh, Viewer::KEY_F, Viewer::MODIF_CTRL);
    viewer.set_usage("default", "Ctrl + f: fill the holes");

    // Enter the interactive loop (returns when the window is closed).
    return viewer.run();
}