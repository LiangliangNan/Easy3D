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
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/algo/surface_mesh_curvature.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_801_Mesh_Curvature/main.cpp
 *
 * This example shows how to compute curvature information on a surface mesh using the
 * SurfaceMeshCurvature class, and how to visualize it as a scalar field mapped to a colormap.
 *
 * The principal curvatures are estimated per vertex; from them we derive the mean curvature,
 * the Gaussian curvature, and the maximum absolute curvature, each stored as a vertex property
 * ("v:curv-mean", "v:curv-gauss", "v:curv-max_abs") and rendered with set_scalar_coloring().
 */

using namespace easy3d;


namespace {

    bool apply_curvature(int type, Viewer *viewer, Model *model) {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        if (!mesh)
            return false;

        SurfaceMeshCurvature curvature(mesh);
        curvature.analyze(2); // a few post-smoothing steps

        std::string property;
        if (type == 0) {
            curvature.compute_mean_curvature();
            property = "v:curv-mean";
        } else if (type == 1) {
            curvature.compute_gauss_curvature();
            property = "v:curv-gauss";
        } else {
            curvature.compute_max_abs_curvature();
            property = "v:curv-max_abs";
        }

        auto drawable = mesh->renderer()->get_triangles_drawable("faces");
        drawable->set_scalar_coloring(State::VERTEX, property);
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

    // Show the mean curvature initially.
    apply_curvature(0, &viewer, mesh);

    viewer.bind([](Viewer *v, Model *m) { return apply_curvature(0, v, m); }, mesh, Viewer::KEY_E, Viewer::MODIF_CTRL);
    viewer.bind([](Viewer *v, Model *m) { return apply_curvature(1, v, m); }, mesh, Viewer::KEY_G, Viewer::MODIF_CTRL);
    viewer.bind([](Viewer *v, Model *m) { return apply_curvature(2, v, m); }, mesh, Viewer::KEY_K, Viewer::MODIF_CTRL);
    viewer.set_usage("default",
                     "Ctrl + e: mean curvature\n"
                     "Ctrl + g: Gaussian curvature\n"
                     "Ctrl + k: maximum absolute curvature");

    return viewer.run();
}