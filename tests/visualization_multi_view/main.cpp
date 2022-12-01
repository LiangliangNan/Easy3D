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

#include <easy3d/viewer/multi_viewer.h>
#include <easy3d/core/model.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/timer.h>


using namespace easy3d;

int test_composite_view(int duration) {
    // create a 2 by 2 MultiViewer
    MultiViewer viewer(2, 2, "MultiView");

    // ---------------------------------------------------------------------------
    // setup content for view(0, 0): the graph model (vertices and edges)
    const std::string file_graph = resource::directory() + "/data/graph.ply";
    auto graph = viewer.add_model(file_graph, true);
    if (graph)
        viewer.assign(0, 0, graph);
    else
        LOG(ERROR) << "failed to load model from file: " << file_graph;

    // ---------------------------------------------------------------------------
    // setup content for view(0, 1): the surface of the sphere model
    const std::string file_sphere = resource::directory() + "/data/sphere.obj";
    auto sphere = viewer.add_model(file_sphere, true);
    if (sphere) {
        auto sphere_faces = sphere->renderer()->get_triangles_drawable("faces");
        viewer.assign(0, 1, sphere_faces);
    }
    else
        LOG(ERROR) << "failed to load model from file: " << file_sphere;

    // ---------------------------------------------------------------------------
    // setup content for view(1, 0): the wireframe of the sphere model
    auto sphere_wireframe = sphere->renderer()->get_lines_drawable("edges");
    sphere_wireframe->set_impostor_type(LinesDrawable::CYLINDER);
    sphere_wireframe->set_line_width(5);
    sphere_wireframe->set_uniform_coloring(vec4(0.7f, 0.7f, 1.0f, 1.0f));
    viewer.assign(1, 0, sphere_wireframe);

    // ---------------------------------------------------------------------------
    // setup content for view(1, 1): the vertices of the sphere model
    auto sphere_vertices = sphere->renderer()->get_points_drawable("vertices");
    sphere_vertices->set_impostor_type(PointsDrawable::SPHERE);
    sphere_vertices->set_point_size(15);
    viewer.assign(1, 1, sphere_vertices);

    // show a simple message
    viewer.set_usage("testing composite view...");

    Timer<>::single_shot(duration, (Viewer*)&viewer, &Viewer::exit);
    return viewer.run();
}