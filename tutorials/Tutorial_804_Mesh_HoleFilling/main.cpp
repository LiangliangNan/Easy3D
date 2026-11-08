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
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/algo/surface_mesh_hole_filling.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_804_Mesh_HoleFilling/main.cpp
 *
 * This example shows how to close holes (boundary loops) in a surface mesh using the
 * SurfaceMeshHoleFilling class.
 *
 * The input is an open mesh (a hemisphere) whose boundary is highlighted by the "borders"
 * drawable. Filling triangulates each hole and then refines and fairs the new patch so it
 * blends smoothly with the surrounding surface.
 */

using namespace easy3d;


namespace {

    bool fill_holes(Viewer *viewer, Model *model) {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        if (!mesh)
            return false;
        SurfaceMeshHoleFilling filling(mesh);
        const int n = filling.fill_holes(1000);
        std::cout << "filled " << n << " hole(s)" << std::endl;
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

    // Turn the camera to look at the bottom of the bunny model
    viewer.camera()->setUpVector(vec3(0, 1, 0));
    viewer.camera()->setViewDirection(vec3(0, 0, 1));

    // Highlight the boundary loops (the holes to be filled).
    auto borders = mesh->renderer()->get_lines_drawable("borders");
    borders->set_visible(true);
    borders->set_uniform_coloring(vec4(1.0f, 0.0f, 0.0f, 1.0f));
    borders->set_line_width(3.0f);

    viewer.bind([](Viewer *v, Model *m) { return fill_holes(v, m); }, mesh, Viewer::KEY_F, Viewer::MODIF_CTRL);
    viewer.set_usage("default", "Ctrl + f: fill the holes");

    return viewer.run();
}