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
#include <easy3d/algo/surface_mesh_subdivision.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_807_Mesh_Subdivision/main.cpp
 *
 * This example shows how to subdivide a surface mesh using the SurfaceMeshSubdivision class,
 * which provides Catmull-Clark, Loop, and sqrt(3) subdivision schemes.
 *
 * Starting from a coarse cube, each shortcut applies one subdivision step (repeatable).
 * Catmull-Clark operates on polygon meshes; Loop and sqrt(3) operate on triangle meshes, so the
 * mesh is triangulated first for those two schemes.
 */

using namespace easy3d;


namespace {

    bool subdivide(int scheme, Viewer *viewer, Model *model) {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        if (!mesh)
            return false;

        bool ok = false;
        if (scheme == 0) {
            ok = SurfaceMeshSubdivision::catmull_clark(mesh);
        } else {
            mesh->triangulate();
            ok = (scheme == 1) ? SurfaceMeshSubdivision::loop(mesh)
                               : SurfaceMeshSubdivision::sqrt3(mesh);
        }
        if (!ok) {
            std::cerr << "subdivision failed" << std::endl;
            return false;
        }
        mesh->renderer()->update();
        viewer->update();
        return true;
    }

} // namespace


int main(int argc, char **argv) {
    initialize();

    const std::string file = resource::directory() + "/data/quad_mesh/cube_quad.off";

    Viewer viewer(EXAMPLE_TITLE);
    auto mesh = dynamic_cast<SurfaceMesh *>(viewer.add_model(file, true));
    if (!mesh) {
        std::cerr << "failed to load model. Please make sure the file exists and format is correct" << std::endl;
        return EXIT_FAILURE;
    }

    viewer.camera()->setViewDirection(vec3(1, 1, -1));

    // Show the edges so the refinement is visible.
    mesh->renderer()->get_lines_drawable("edges")->set_visible(true);

    viewer.bind([](Viewer *v, Model *m) { return subdivide(0, v, m); }, mesh, Viewer::KEY_C, Viewer::MODIF_CTRL);
    viewer.bind([](Viewer *v, Model *m) { return subdivide(1, v, m); }, mesh, Viewer::KEY_L, Viewer::MODIF_CTRL);
    viewer.bind([](Viewer *v, Model *m) { return subdivide(2, v, m); }, mesh, Viewer::KEY_T, Viewer::MODIF_CTRL);
    viewer.set_usage("default",
                     "Ctrl + c: Catmull-Clark subdivision\n"
                     "Ctrl + l: Loop subdivision\n"
                     "Ctrl + t: sqrt(3) subdivision");

    viewer.fit_screen();
    return viewer.run();
}