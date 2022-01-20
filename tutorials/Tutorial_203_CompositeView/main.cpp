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


#if 0

#include "viewer.h"
#include <easy3d/fileio/resources.h>
#include <easy3d/util/logging.h>

using namespace easy3d;

// This example shows how to render a scene into multiple views within a single window.

int main(int argc, char** argv) {
    // Initialize logging.
    logging::initialize();

    CompositeView viewer("Tutorial_203_CompositeView");

    // Load model from a file
    const std::string file_name = resource::directory() + "/data/torusknot.obj";
    if (!viewer.add_model(file_name, true)) {
        LOG(ERROR) << "Error: failed to load model. Please make sure the file exists and format is correct.";
        return EXIT_FAILURE;
    }

    // Run the viewer
    return viewer.run();
}
#else

#include <easy3d/viewer/comp_viewer.h>
#include <easy3d/fileio/resources.h>
#include <easy3d/util/logging.h>
#include <easy3d/core/model.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_triangles.h>

using namespace easy3d;

// This example shows how to use the built-in composite viewer.

int main(int argc, char** argv) {
    // Initialize logging.
    logging::initialize();

    /// create a 2 by 2 composite viewer
    CompViewer viewer(2, 2, "Tutorial_203_CompositeView");

    // ---------------------------------------------------------------------------
    /// setup content for view(0, 0): we show the graph model (vertices and edges)
    const std::string file_graph = resource::directory() + "/data/graph.ply";
    auto graph = viewer.add_model(file_graph, true);
    if (graph)
        viewer.view(0, 0).models.push_back(graph);
    else
        LOG(ERROR) << "failed to load model from file: " << file_graph;

    // ---------------------------------------------------------------------------
    /// setup content for view(0, 1): we show the surface of the sphere model
    const std::string file_sphere = resource::directory() + "/data/sphere.obj";
    auto sphere = viewer.add_model(file_sphere, true);
    if (sphere) {
        auto sphere_faces = sphere->renderer()->get_triangles_drawable("faces");
        viewer.view(0, 1).drawables.push_back(sphere_faces);
    }
    else
        LOG(ERROR) << "failed to load model from file: " << file_sphere;

    // ---------------------------------------------------------------------------
    /// setup content for view(1, 0): we show the wireframe of the sphere model
    auto sphere_wireframe = sphere->renderer()->get_lines_drawable("edges");
    sphere_wireframe->set_impostor_type(LinesDrawable::CYLINDER);
    sphere_wireframe->set_line_width(5);
    sphere_wireframe->set_uniform_coloring(vec4(0.7f, 0.7f, 1.0f, 1.0f));
    sphere_wireframe->set_visible(true); // by default wireframe is hidden
    viewer.view(1, 0).drawables.push_back(sphere_wireframe);

    // ---------------------------------------------------------------------------
    /// setup content for view(1, 1): we show the vertices of the sphere model
    auto sphere_vertices = sphere->renderer()->get_points_drawable("vertices");
    sphere_vertices->set_impostor_type(PointsDrawable::SPHERE);
    sphere_vertices->set_point_size(15);
    sphere_vertices->set_visible(true); // by default vertices are hidden
    viewer.view(1, 1).drawables.push_back(sphere_vertices);

    // Run the viewer
    return viewer.run();
}
#endif

