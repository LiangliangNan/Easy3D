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
#include <easy3d/algo/surface_mesh_simplification.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

#include <algorithm>

/**
 * \example{lineno} Tutorial_806_Mesh_Simplification/main.cpp
 *
 * This example shows how to simplify a surface mesh (reduce its number of vertices) using
 * the SurfaceMeshSimplification class, which implements quadric-error-metric edge collapse.
 *
 * Each press of the shortcut simplifies the mesh to half of its current vertices, so the
 * progressive loss of detail can be observed. The edges are drawn to make the coarsening visible.
 */

using namespace easy3d;


namespace {

    bool simplify(Viewer *viewer, Model *model) {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        if (!mesh)
            return false;
        SurfaceMeshSimplification simplification(mesh);
        simplification.initialize();
        const unsigned int target = std::max(100u, mesh->n_vertices() / 2);
        simplification.simplify(target);
        std::cout << "simplified to " << mesh->n_vertices() << " vertices / " << mesh->n_faces() << " faces" << std::endl;
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

    // Show the edges so the coarsening is visible.
    mesh->renderer()->get_lines_drawable("edges")->set_visible(true);

    viewer.bind([](Viewer *v, Model *m) { return simplify(v, m); }, mesh, Viewer::KEY_D, Viewer::MODIF_CTRL);
    viewer.set_usage("default", "Ctrl + d: simplify to half of the current vertices");

    viewer.fit_screen();
    return viewer.run();
}