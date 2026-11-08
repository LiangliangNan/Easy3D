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
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/algo/surface_mesh_remeshing.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_805_Mesh_Remeshing/main.cpp
 *
 * This example shows how to remesh a surface mesh using the SurfaceMeshRemeshing
 * class (incremental remeshing by edge collapse, split, flip and tangential
 * relaxation; Botsch & Kobbelt, SGP 2004). Two schemes are demonstrated:
 *      - uniform remeshing towards a target edge length (a multiple of the current
 *        average edge length);
 *      - adaptive remeshing, which refines according to local curvature while
 *        respecting a minimum / maximum edge length and an approximation error.
 *
 * Remeshing changes the connectivity, so we call renderer()->update() afterwards to
 * re-extract the rendering buffers. The edges are drawn so the new triangulation is
 * visible.
 */

using namespace easy3d;


namespace {

    float average_edge_length(const SurfaceMesh *mesh) {
        float len = 0.0f;
        for (auto e: mesh->edges())
            len += distance(mesh->position(mesh->vertex(e, 0)), mesh->position(mesh->vertex(e, 1)));
        return mesh->n_edges() > 0 ? len / static_cast<float>(mesh->n_edges()) : 0.0f;
    }

    bool uniform_remesh(Viewer *viewer, Model *model) {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        if (!mesh)
            return false;
        const float target = average_edge_length(mesh) * 1.5f;
        SurfaceMeshRemeshing remeshing(mesh);
        remeshing.uniform_remeshing(target);
        std::cout << "uniform remeshing: " << mesh->n_vertices() << " vertices / "
                  << mesh->n_faces() << " faces" << std::endl;
        mesh->renderer()->update();
        viewer->update();
        return true;
    }

    bool adaptive_remesh(Viewer *viewer, Model *model) {
        auto mesh = dynamic_cast<SurfaceMesh *>(model);
        if (!mesh)
            return false;
        const float bb = mesh->bounding_box().diagonal_length();
        SurfaceMeshRemeshing remeshing(mesh);
        remeshing.adaptive_remeshing(0.001f * bb, 0.100f * bb, 0.001f * bb);
        std::cout << "adaptive remeshing: " << mesh->n_vertices() << " vertices / "
                  << mesh->n_faces() << " faces" << std::endl;
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

    // Show the edges so the new triangulation is visible.
    mesh->renderer()->get_lines_drawable("edges")->set_visible(true);

    viewer.bind([](Viewer *v, Model *m) { return uniform_remesh(v, m); }, mesh, Viewer::KEY_U, Viewer::MODIF_CTRL);
    viewer.bind([](Viewer *v, Model *m) { return adaptive_remesh(v, m); }, mesh, Viewer::KEY_A, Viewer::MODIF_CTRL);
    viewer.set_usage("default",
                     "Ctrl + u: uniform remeshing (target = 1.5 x average edge length)\n"
                     "Ctrl + a: adaptive remeshing (curvature-driven)");

    return viewer.run();
}