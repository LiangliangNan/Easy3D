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
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/algo/surface_mesh_curvature.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_801_Mesh_Curvature/main.cpp
 *
 * \brief Estimating per-vertex curvature on a triangle mesh and visualizing it as a color-mapped scalar field.
 *
 * <b>Overview</b>
 * This tutorial is the first of the surface-mesh geometry-processing series. It loads the bunny
 * triangle mesh, estimates discrete curvature at every vertex with SurfaceMeshCurvature, and paints
 * the result on the surface with a colormap. Three scalar fields are available (mean, Gaussian, and
 * maximum absolute curvature) and can be switched interactively. Curvature is a fundamental shape
 * descriptor: it tells you where a surface is flat, where it bends, and where it has sharp features,
 * and it drives many downstream algorithms (smoothing, remeshing, feature detection).
 *
 * <b>The technique</b>
 * On a smooth surface, curvature describes how fast the surface turns. At each point there are two
 * principal curvatures k1 and k2 (the extreme bending rates over all tangent directions); from them
 * we form the mean curvature H = (k1 + k2) / 2 and the Gaussian curvature K = k1 * k2. H is zero on
 * minimal (soap-film-like) surfaces and measures average bending; K is positive on dome-like (elliptic)
 * regions, negative on saddle-like (hyperbolic) regions, and zero on flat or developable regions — it
 * is an intrinsic measure tied to the angle defect of the surface. A triangle mesh has no smooth
 * derivatives, so curvature must be estimated discretely from the local neighborhood. Easy3D follows
 * the discrete differential-geometry operators of Meyer et al. (2003, "Discrete Differential-Geometry
 * Operators for Triangulated 2-Manifolds") and the normal-cycle / restricted-Delaunay approach of
 * Cohen-Steiner and Morvan (2003): principal curvatures are recovered per vertex from the surrounding
 * faces/edges, boundary vertices are interpolated from their interior neighbors, and a few smoothing
 * iterations suppress estimation noise. The maximum absolute curvature max(|k1|, |k2|) highlights the
 * strongest bending regardless of sign, which is useful for spotting creases and corners.
 *
 * <b>Easy3D API</b>
 * - SurfaceMeshCurvature: construct it on a SurfaceMesh, call analyze() to estimate the principal
 *   curvatures (stored as "v:curv-min" / "v:curv-max"), then compute_mean_curvature(),
 *   compute_gauss_curvature(), or compute_max_abs_curvature() to fill the derived vertex properties
 *   "v:curv-mean", "v:curv-gauss", "v:curv-max_abs".
 * - DrawableTriangles::set_scalar_coloring(): maps a per-vertex scalar property to a colormap so the
 *   field becomes visible on the surface.
 * - Viewer::bind_shortcut() / set_usage(): register the Ctrl-key shortcuts and the on-screen help text.
 * - Renderer::update(): re-extracts rendering buffers after the property used for coloring changes.
 *
 * <b>Running it</b>
 * The mean curvature is shown first. Press Ctrl+e for mean curvature, Ctrl+g for Gaussian curvature,
 * and Ctrl+k for maximum absolute curvature; the surface is recolored each time.
 *
 * \see SurfaceMeshCurvature, SurfaceMesh, DrawableTriangles, Tutorial_802_Mesh_Smoothing
 */

using namespace easy3d;

namespace {

    // Recompute the requested curvature field and recolor the mesh with it.
    // Bound to the Ctrl shortcuts below; returns false if the model is not a surface mesh.
    bool apply_curvature(int type, Viewer *viewer, Model *model) {
        // The viewer stores models as generic Model pointers; curvature needs a SurfaceMesh.
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        if (!mesh)
            return false;

        // Estimate the principal curvatures k1/k2 at every vertex. The argument (2) asks for a
        // couple of post-smoothing iterations so the discrete estimate is less noisy.
        SurfaceMeshCurvature curvature(mesh);
        curvature.analyze(2); // a few post-smoothing steps

        // Derive the requested scalar field from the principal curvatures and remember the name of
        // the vertex property it is stored in (these names are fixed by SurfaceMeshCurvature).
        std::string property;
        if (type == 0) {
            curvature.compute_mean_curvature(); // H = (k1 + k2) / 2
            property = "v:curv-mean";
        } else if (type == 1) {
            curvature.compute_gauss_curvature(); // K = k1 * k2
            property = "v:curv-gauss";
        } else {
            curvature.compute_max_abs_curvature(); // max(|k1|, |k2|)
            property = "v:curv-max_abs";
        }

        // Color the surface by the scalar field: "faces" is the mesh's standard triangles drawable,
        // and set_scalar_coloring() maps the per-vertex property through a colormap.
        auto drawable = mesh->renderer()->get_triangles_drawable("faces");
        drawable->set_scalar_coloring(State::VERTEX, property);
        mesh->renderer()->update(); // rebuild GPU buffers from the updated mesh/properties
        viewer->update();           // request a redraw so the new colors appear
        return true;
    }

} // namespace

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    // The bunny is a classic closed triangle mesh with rich curvature variation.
    const std::string file = resource::directory() + "/data/bunny.ply";

    // Create the viewer and load the mesh; add_model() centers/scales it to fit the scene (true).
    Viewer viewer(EXAMPLE_TITLE);
    auto mesh = dynamic_cast<SurfaceMesh *>(viewer.add_model(file, true));
    if (!mesh) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    // Look at the bunny from the front so the curvature pattern is easy to read.
    viewer.camera()->setViewDirection(vec3(0, 1, 0));

    // Show the mean curvature initially.
    apply_curvature(0, &viewer, mesh);

    // Bind one shortcut per curvature field; each lambda forwards to apply_curvature() with the
    // matching type. set_usage() documents the keys in the viewer's help overlay.
    viewer.bind_shortcut([](Viewer *v, Model *m) { return apply_curvature(0, v, m); }, mesh, Viewer::KEY_E, Viewer::MODIF_CTRL);
    viewer.bind_shortcut([](Viewer *v, Model *m) { return apply_curvature(1, v, m); }, mesh, Viewer::KEY_G, Viewer::MODIF_CTRL);
    viewer.bind_shortcut([](Viewer *v, Model *m) { return apply_curvature(2, v, m); }, mesh, Viewer::KEY_K, Viewer::MODIF_CTRL);
    viewer.set_usage(
        "default",
        "Ctrl + e: mean curvature\n" "Ctrl + g: Gaussian curvature\n" "Ctrl + k: maximum absolute curvature");

    // Enter the interactive loop (returns when the window is closed).
    return viewer.run();
}