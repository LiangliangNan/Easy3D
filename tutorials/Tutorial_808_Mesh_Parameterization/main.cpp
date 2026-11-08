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
#include <easy3d/algo/surface_mesh_parameterization.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_808_Mesh_Parameterization/main.cpp
 *
 * This example shows how to compute a texture parameterization of a surface mesh using the
 * SurfaceMeshParameterization class, and how to visualize it by mapping a checkerboard texture
 * onto the surface.
 *
 * Two methods are available: discrete harmonic parameterization and least squares conformal
 * mapping (LSCM). Both require a mesh with a boundary and store the resulting 2D coordinates in
 * the "v:texcoord" vertex property, which is then used with set_texture_coloring(). A hemisphere
 * (a disk-topology patch) is used here because parameterization needs an open boundary.
 */

using namespace easy3d;


namespace {

    bool parameterize(int method, Viewer *viewer, Model *model) {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        if (!mesh)
            return false;

        SurfaceMeshParameterization parameterization(mesh);
        if (method == 0)
            parameterization.harmonic();
        else {
            mesh->triangulate(); // LSCM requires a triangle mesh
            parameterization.lscm();
        }

        auto drawable = mesh->renderer()->get_triangles_drawable("faces");
        drawable->set_texture_coloring(State::VERTEX, "v:texcoord");
        mesh->renderer()->update();
        viewer->update();
        return true;
    }

} // namespace


int main(int argc, char **argv) {
    initialize();

    const std::string file = resource::directory() + "/data/girl_face.ply";

    Viewer viewer(EXAMPLE_TITLE);
    auto mesh = dynamic_cast<SurfaceMesh *>(viewer.add_model(file, true));
    if (!mesh) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    viewer.camera()->setUpVector(vec3(0, 1, 0));
    viewer.camera()->setViewDirection(vec3(0, 0, 1));

    // Show the harmonic parameterization initially.
    parameterize(0, &viewer, mesh);

    viewer.bind([](Viewer *v, Model *m) { return parameterize(0, v, m); }, mesh, Viewer::KEY_A, Viewer::MODIF_CTRL);
    viewer.bind([](Viewer *v, Model *m) { return parameterize(1, v, m); }, mesh, Viewer::KEY_L, Viewer::MODIF_CTRL);
    viewer.set_usage("default",
                     "Ctrl + a: harmonic parameterization\n"
                     "Ctrl + l: least squares conformal mapping (LSCM)");

    return viewer.run();
}