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
#include <easy3d/algo/surface_mesh_smoothing.h>
#include <easy3d/algo/gaussian_noise.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_802_Mesh_Smoothing/main.cpp
 *
 * \brief Removing noise from a triangle mesh with explicit and implicit Laplacian smoothing.
 *
 * <b>Overview</b>
 * This tutorial demonstrates mesh smoothing (denoising) with SurfaceMeshSmoothing. To make the
 * effect obvious, Gaussian noise is first added to the bunny mesh; the smoothing shortcuts then
 * progressively remove it. Two variants are shown side by side: explicit Laplacian smoothing (a few
 * local averaging iterations) and implicit Laplacian smoothing (a single global solve). Smoothing is
 * the workhorse for cleaning up scanned or reconstructed geometry before further processing.
 *
 * <b>The technique</b>
 * Laplacian smoothing moves each vertex toward the average of its neighbors, which attenuates
 * high-frequency noise. The discrete Laplacian of the vertex positions is L(p); with the cotangent
 * weights it approximates the mean-curvature normal, so iterating p <- p + dt * L(p) is a discretization
 * of mean-curvature flow (the surface diffuses like heat). Explicit smoothing applies this update
 * directly for a fixed number of iterations: it is cheap and local, but the timestep must be small or
 * the iteration becomes unstable, and repeated averaging also shrinks the mesh and blurs features.
 * Implicit smoothing instead solves (I - dt * L) p_new = p_old in one sparse linear system. Because the
 * update is implicit (backward Euler), it is unconditionally stable for large timesteps and removes
 * noise in a single step, at the cost of solving a global system. Both approaches are described in
 * Desbrun et al., "Implicit fairing of irregular meshes using diffusion and curvature flow" (SIGGRAPH
 * 1999); see also Kazhdan et al. (CGF 2012) on non-singular mean-curvature flow. Easy3D defaults to the
 * cotangent (geometry-aware) Laplacian rather than uniform weights.
 *
 * <b>Easy3D API</b>
 * - SurfaceMeshSmoothing: explicit_smoothing(iters) runs local averaging iterations;
 *   implicit_smoothing(timestep) solves the global implicit system.
 * - GaussianNoise::apply(): perturbs vertex positions along their normals with a given standard
 *   deviation, used here to synthesize a noisy input.
 * - Renderer::update(): re-extracts the rendering buffers after the vertex positions change.
 * - Viewer::bind() / set_usage(): register the Ctrl shortcuts and the help text.
 *
 * <b>Running it</b>
 * The model starts noisy. Press Ctrl+e for explicit smoothing (10 iterations), Ctrl+i for implicit
 * smoothing, and Ctrl+n to add Gaussian noise again so you can repeat the comparison.
 *
 * \see SurfaceMeshSmoothing, GaussianNoise, SurfaceMesh, Tutorial_803_Mesh_Fairing
 */

using namespace easy3d;

namespace {

    // Perturb every vertex along its normal by Gaussian noise. The standard deviation is scaled to
    // the model size (0.3% of the bounding-box diagonal) so the noise is visible but not extreme.
    void add_noise(SurfaceMesh *mesh) {
        const float sigma = mesh->bounding_box().diagonal_length() * 0.003f;
        GaussianNoise::apply(mesh, sigma);
        mesh->renderer()->update(); // positions changed: refresh the rendering buffers
    }

    // Shortcut handler: add noise again so the smoothing can be re-tested.
    bool noise(Viewer *viewer, Model *model) {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        if (!mesh)
            return false;
        add_noise(mesh);
        mesh->renderer()->update();
        viewer->update(); // request a redraw
        return true;
    }

    // Shortcut handler: explicit Laplacian smoothing — 10 local neighbor-averaging iterations.
    bool smooth_explicit(Viewer *viewer, Model *model) {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        if (!mesh)
            return false;
        SurfaceMeshSmoothing smoothing(mesh);
        smoothing.explicit_smoothing(10);
        mesh->renderer()->update(); // positions changed: refresh the rendering buffers
        viewer->update();
        return true;
    }

    // Shortcut handler: implicit Laplacian smoothing — one stable global solve (default timestep).
    bool smooth_implicit(Viewer *viewer, Model *model) {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        if (!mesh)
            return false;
        SurfaceMeshSmoothing smoothing(mesh);
        smoothing.implicit_smoothing();
        mesh->renderer()->update(); // positions changed: refresh the rendering buffers
        viewer->update();
        return true;
    }

} // namespace

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    // Create the viewer and load the bunny triangle mesh (centered/scaled to fit the scene).
    Viewer viewer(EXAMPLE_TITLE);

    const std::string file = resource::directory() + "/data/bunny.ply";
    auto mesh = dynamic_cast<SurfaceMesh *>(viewer.add_model(file, true));
    if (!mesh) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    // Look at the bunny from the front.
    viewer.camera()->setViewDirection(vec3(0, 1, 0));

    // Perturb the model so the smoothing effect is clearly visible.
    add_noise(mesh);

    // Register the smoothing/noise shortcuts and the on-screen help text.
    viewer.bind([](Viewer *v, Model *m) { return smooth_explicit(v, m); }, mesh, Viewer::KEY_E, Viewer::MODIF_CTRL);
    viewer.bind([](Viewer *v, Model *m) { return smooth_implicit(v, m); }, mesh, Viewer::KEY_I, Viewer::MODIF_CTRL);
    viewer.bind([](Viewer *v, Model *m) { return noise(v, m); }, mesh, Viewer::KEY_N, Viewer::MODIF_CTRL);
    viewer.set_usage(
        "default",
        "Ctrl + e: explicit Laplacian smoothing (10 iterations)\n" "Ctrl + i: implicit Laplacian smoothing\n" "Ctrl + n: add Gaussian noise again");

    // Enter the interactive loop (returns when the window is closed).
    return viewer.run();
}