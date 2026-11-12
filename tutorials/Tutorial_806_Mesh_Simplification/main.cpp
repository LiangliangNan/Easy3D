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
#include <easy3d/algo/surface_mesh_simplification.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

#include <algorithm>

/**
 * \example{lineno} Tutorial_806_Mesh_Simplification/main.cpp
 *
 * \brief Reducing a mesh's vertex count with quadric-error-metric edge collapse (decimation).
 *
 * <b>Overview</b>
 * This tutorial demonstrates mesh simplification (decimation) with SurfaceMeshSimplification. Each press
 * of the shortcut collapses the mesh down to half of its current vertices (with a floor of 100), so you
 * can watch detail disappear progressively while the overall shape is preserved. The edges are drawn to
 * make the coarsening visible. Simplification is essential for level-of-detail rendering, faster
 * processing, and compact storage of high-resolution scans.
 *
 * <b>The technique</b>
 * The algorithm is incremental greedy decimation by halfedge collapse: repeatedly merge one vertex into
 * a neighbor, removing the two adjacent triangles, until the target vertex count is reached. The key
 * question is which collapse to do next and where to place the surviving vertex. Easy3D uses the quadric
 * error metric of Garland and Heckbert, "Surface simplification using quadric error metrics" (SIGGRAPH
 * 1997): each vertex accumulates a 4x4 symmetric matrix Q that encodes the sum of squared distances to
 * the planes of its incident faces. The cost of placing a vertex at position p is p^T Q p, so the
 * optimal collapse position minimizes this quadratic error, and collapses are prioritized by their
 * error in a heap (cheapest first). This preserves the geometry far better than naive midpoint collapse.
 * The implementation also follows the general decimation framework of Kobbelt et al. (Graphics Interface
 * 1998), with optional fairness/quality constraints (aspect ratio, normal deviation, Hausdorff error).
 *
 * <b>Easy3D API</b>
 * - SurfaceMeshSimplification: initialize() sets up the quadrics and priority queue; simplify(n_vertices)
 *   collapses edges until roughly n_vertices remain.
 * - DrawableLines ("edges"): made visible to reveal the coarsening triangulation.
 * - Renderer::update(): re-extracts the rendering buffers after the connectivity changes.
 * - Viewer::bind_shortcut() / set_usage(): register the Ctrl shortcut and the help text.
 *
 * <b>Running it</b>
 * Press Ctrl+d to simplify to half of the current vertices; repeat to decimate further. The console
 * prints the resulting vertex/face counts after each pass.
 *
 * \see SurfaceMeshSimplification, SurfaceMesh, DrawableLines, Tutorial_805_Mesh_Remeshing, Tutorial_807_Mesh_Subdivision
 */

using namespace easy3d;

namespace {

    // Shortcut handler: decimate the mesh to half its current vertices (but never below 100).
    bool simplify(Viewer *viewer, Model *model) {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        if (!mesh)
            return false;
        SurfaceMeshSimplification simplification(mesh);
        simplification.initialize(); // build per-vertex error quadrics and the collapse priority queue
        const unsigned int target = std::max(100u, mesh->n_vertices() / 2);
        simplification.simplify(target); // greedily collapse the cheapest edges until the target is met
        std::cout << "simplified to " << mesh->n_vertices() << " vertices / " << mesh->n_faces() << " faces"
                  << std::endl;
        mesh->renderer()->update(); // connectivity changed: refresh the rendering buffers
        viewer->update();           // request a redraw
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

    viewer.camera()->set_view_direction(vec3(0, 1, 0));

    // Show the edges so the coarsening is visible.
    mesh->renderer()->get_lines_drawable("edges")->set_visible(true);

    // Register the simplify shortcut and the on-screen help text.
    viewer.bind_shortcut([](Viewer *v, Model *m) { return simplify(v, m); }, mesh, Viewer::KEY_D, Viewer::MODIF_CTRL);
    viewer.set_usage("default", "Ctrl + d: simplify to half of the current vertices");

    // Enter the interactive loop (returns when the window is closed).
    return viewer.run();
}