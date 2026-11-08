/********************************************************************
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
 ********************************************************************/

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
 * This example shows how to fair a surface mesh using the SurfaceMeshFairing class
 * (implicit fairing by diffusion / curvature flow; Desbrun et al., SIGGRAPH 1999).
 * Fairing solves a k-harmonic equation: degree 1 minimizes surface area and degree 2
 * minimizes surface curvature. Boundary vertices are kept fixed as constraints; with
 * no selection the whole (interior of the) mesh is faired.
 *
 * To make the effect visible, Gaussian noise is added to the model first. After
 * modifying the vertex positions we call renderer()->update() so the rendering
 * buffers are re-extracted from the mesh.
 */

using namespace easy3d;


namespace {

    void add_noise(SurfaceMesh *mesh) {
        const float sigma = mesh->bounding_box().diagonal_length() * 0.001f;
        GaussianNoise::apply(mesh, sigma);
    }

    bool noise(Viewer *viewer, Model *model) {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        if (!mesh)
            return false;
        add_noise(mesh);
        mesh->renderer()->update();
        viewer->update();
        return true;
    }

    bool fair_area(Viewer *viewer, Model *model) {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        if (!mesh)
            return false;
        SurfaceMeshFairing fairing(mesh);
        fairing.minimize_area();
        mesh->renderer()->update();
        viewer->update();
        return true;
    }

    bool fair_curvature(Viewer *viewer, Model *model) {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        if (!mesh)
            return false;
        SurfaceMeshFairing fairing(mesh);
        fairing.minimize_curvature();
        mesh->renderer()->update();
        viewer->update();
        return true;
    }

} // namespace


int main(int argc, char **argv) {
    initialize();

    const std::string file = resource::directory() + "/data/bunny.ply";

    Viewer viewer(EXAMPLE_TITLE);
    auto mesh = dynamic_cast<SurfaceMesh *>(viewer.add_model(file, true));
    if (!mesh) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    viewer.camera()->setViewDirection(vec3(0, 1, 0));

    // Perturb the model so the fairing effect is clearly visible.
    add_noise(mesh);
    mesh->renderer()->update();

    viewer.bind([](Viewer *v, Model *m) { return fair_area(v, m); }, mesh, Viewer::KEY_A, Viewer::MODIF_CTRL);
    viewer.bind([](Viewer *v, Model *m) { return fair_curvature(v, m); }, mesh, Viewer::KEY_C, Viewer::MODIF_CTRL);
    viewer.bind([](Viewer *v, Model *m) { return noise(v, m); }, mesh, Viewer::KEY_N, Viewer::MODIF_CTRL);
    viewer.set_usage("default",
                     "Ctrl + a: minimize surface area (fairing degree 1)\n"
                     "Ctrl + c: minimize surface curvature (fairing degree 2)\n"
                     "Ctrl + n: add Gaussian noise again");

    viewer.fit_screen();
    return viewer.run();
}