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
#include <easy3d/algo/surface_mesh_fairing.h>
#include <easy3d/algo/gaussian_noise.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_803_Mesh_Fairing/main.cpp
 *
 * \brief Fairing a noisy open surface by minimizing area (degree 1) or curvature (degree 2).
 *
 * <b>Overview</b>
 * This tutorial demonstrates implicit mesh fairing with SurfaceMeshFairing. The input is a noisy
 * hemisphere — an open surface patch whose circular boundary is kept fixed. Two fairing modes are
 * available: minimizing surface area (degree 1) and minimizing surface curvature (degree 2). Gaussian
 * noise is added first so the smoothing effect is visible. Fairing is the constrained cousin of
 * smoothing: instead of denoising a closed mesh everywhere, it reconstructs a smooth patch that
 * interpolates prescribed boundary conditions, which is exactly what hole filling and surface
 * reconstruction need.
 *
 * <b>The technique</b>
 * Fairing solves a k-harmonic equation L^k x = 0 on the interior vertices, where L is the (cotangent)
 * Laplace-Beltrami operator and the boundary vertices are held fixed as constraints. The degree k
 * selects which energy is minimized: k = 1 minimizes surface area (a membrane / mean-curvature flow,
 * producing a minimal-area patch), while k = 2 minimizes the integral of squared curvature (a thin-plate
 * / biharmonic energy, producing a smoother, curvature-continuous patch). Because the system is solved
 * implicitly in one sparse linear solve, it is stable and the result depends only on the boundary
 * constraints, not on a timestep. This is the implicit fairing framework of Desbrun et al., "Implicit
 * fairing of irregular meshes using diffusion and curvature flow" (SIGGRAPH 1999). With no vertex
 * selection, the whole interior of the mesh is faired; locked/selected vertices act as constraints.
 *
 * <b>Easy3D API</b>
 * - SurfaceMeshFairing: minimize_area() fairs with degree 1; minimize_curvature() fairs with degree 2
 *   (both forward to fair(k)). Boundary vertices are automatically treated as fixed constraints.
 * - GaussianNoise::apply(mesh, sigma, true): adds noise while preserving the boundary, so the patch
 *   still meets its (fixed) rim after perturbation.
 * - Renderer::update(): re-extracts the rendering buffers after the vertex positions change.
 * - Viewer::bind_shortcut() / set_usage(): register the Ctrl shortcuts and the help text.
 *
 * <b>Running it</b>
 * The hemisphere starts noisy. Press Ctrl+a to minimize surface area (degree 1), Ctrl+c to minimize
 * surface curvature (degree 2), and Ctrl+n to add Gaussian noise again for another comparison.
 *
 * \see SurfaceMeshFairing, GaussianNoise, SurfaceMesh, Tutorial_802_Mesh_Smoothing, Tutorial_804_Mesh_HoleFilling
 */

using namespace easy3d;

namespace {

    // Perturb the vertices by Gaussian noise scaled to the model size (0.5% of the bounding-box
    // diagonal). The third argument (true) preserves the boundary, so the noisy patch still meets the
    // fixed rim that fairing will interpolate.
    void add_noise(SurfaceMesh *mesh) {
        const float sigma = mesh->bounding_box().diagonal_length() * 0.005f;
        GaussianNoise::apply(mesh, sigma, true);
    }

    // Shortcut handler: add noise again so the fairing can be re-tested.
    bool noise(Viewer *viewer, Model *model) {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        if (!mesh)
            return false;
        add_noise(mesh);
        mesh->renderer()->update(); // positions changed: refresh the rendering buffers
        viewer->update();           // request a redraw
        return true;
    }

    // Shortcut handler: fairing degree 1 — minimize surface area (membrane / mean-curvature flow).
    bool fair_area(Viewer *viewer, Model *model) {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        if (!mesh)
            return false;
        SurfaceMeshFairing fairing(mesh);
        fairing.minimize_area();
        mesh->renderer()->update(); // positions changed: refresh the rendering buffers
        viewer->update();
        return true;
    }

    // Shortcut handler: fairing degree 2 — minimize surface curvature (thin-plate / biharmonic energy).
    bool fair_curvature(Viewer *viewer, Model *model) {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        if (!mesh)
            return false;
        SurfaceMeshFairing fairing(mesh);
        fairing.minimize_curvature();
        mesh->renderer()->update(); // positions changed: refresh the rendering buffers
        viewer->update();
        return true;
    }

} // namespace

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    // The hemisphere is an open surface patch: its fixed circular boundary gives fairing the
    // constraints it needs to reconstruct a smooth interior.
    const std::string file = resource::directory() + "/data/hemisphere.ply";

    // Create the viewer and load the mesh (centered/scaled to fit the scene).
    Viewer viewer(EXAMPLE_TITLE);
    auto mesh = dynamic_cast<SurfaceMesh *>(viewer.add_model(file, true));
    if (!mesh) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    // Look at the hemisphere from the side so the patch and its rim are visible.
    viewer.camera()->set_view_direction(vec3(0, 1, 0));

    // Perturb the model so the fairing effect is clearly visible.
    add_noise(mesh);
    mesh->renderer()->update(); // show the noisy surface before any fairing

    // Register the fairing/noise shortcuts and the on-screen help text.
    viewer.bind_shortcut([](Viewer *v, Model *m) { return fair_area(v, m); }, mesh, Viewer::KEY_A, Viewer::MODIF_CTRL);
    viewer.bind_shortcut([](Viewer *v, Model *m) { return fair_curvature(v, m); }, mesh, Viewer::KEY_C, Viewer::MODIF_CTRL);
    viewer.bind_shortcut([](Viewer *v, Model *m) { return noise(v, m); }, mesh, Viewer::KEY_N, Viewer::MODIF_CTRL);
    viewer.set_usage(
        "default",
        "Ctrl + a: minimize surface area (fairing degree 1)\n" "Ctrl + c: minimize surface curvature (fairing degree 2)\n" "Ctrl + n: add Gaussian noise again");

    // Enter the interactive loop (returns when the window is closed).
    return viewer.run();
}