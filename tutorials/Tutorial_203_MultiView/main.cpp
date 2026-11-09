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
#include <easy3d/core/model.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_203_MultiView/main.cpp
 *
 * \brief Showing several models (or several drawables of one model) side by side in a grid of views.
 *
 * <b>Overview</b>
 * MultiViewer splits a single window into a rows-by-columns grid of independent viewports, each
 * with its own camera and its own list of models/drawables. This tutorial builds a 2 x 2 layout
 * that displays a graph, a shaded sphere surface, the sphere's wireframe, and the sphere's
 * vertices — a convenient way to compare representations of the same data at once.
 *
 * <b>The technique</b>
 * A multi-view layout is just a set of scissored viewports rendered one after another into the
 * same framebuffer. Each cell owns a camera, so the user can navigate the views independently;
 * mouse events are remapped to the cell under the cursor. The key idea in Easy3D is the
 * separation between ownership and visibility: add_model()/add_drawable() give the viewer
 * ownership of the data (and create its renderer), while assign(row, col, ...) chooses which
 * cell actually displays it. Because a drawable can be assigned to several cells, the four
 * views here can share one underlying sphere model while showing different drawables of it
 * (faces, edges, vertices). Impostor rendering (cylinders for lines, spheres for points) draws
 * thick, smoothly shaded primitives from simple vertex data, which is why the wireframe and
 * point views look like solid tubes and balls.
 *
 * <b>Easy3D API</b>
 * - MultiViewer: the grid viewer; construct it with (rows, cols) and use assign() per cell.
 * - Renderer: each model's renderer exposes named drawables via get_triangles_drawable(),
 *   get_lines_drawable(), and get_points_drawable().
 * - TrianglesDrawable / LinesDrawable / PointsDrawable: the surface, wireframe, and vertex
 *   representations; set_impostor_type(), set_line_width()/set_point_size(), and
 *   set_uniform_coloring() control their appearance.
 *
 * <b>Running it</b>
 * A window opens divided into four cells. Click a cell to make it active, then drag to rotate
 * its camera independently of the others. The top-left shows the graph, top-right the shaded
 * sphere, bottom-left its wireframe, and bottom-right its vertices.
 *
 * \see MultiViewer, Renderer, LinesDrawable, PointsDrawable, Tutorial_201_Viewer_default
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();

    // Create a 2 by 2 grid of independent views.
    MultiViewer viewer(2, 2, EXAMPLE_TITLE);

    // ---------------------------------------------------------------------------
    // setup content for view(0, 0): the graph model (vertices and edges)
    const std::string file_graph = resource::directory() + "/data/graph.ply";
    // add_model() loads the graph and gives the viewer ownership; assign() puts it in a cell.
    auto graph = viewer.add_model(file_graph);
    if (graph)
        viewer.assign(0, 0, graph);
    else
        std::cerr << "failed to load model from file: " << file_graph << std::endl;

    // ---------------------------------------------------------------------------
    // setup content for view(0, 1): the surface of the sphere model
    const std::string file_sphere = resource::directory() + "/data/sphere.obj";
    auto sphere = viewer.add_model(file_sphere);
    if (sphere) {
        // "faces" is the standard triangles drawable of a surface mesh; show it shaded.
        auto sphere_faces = sphere->renderer()->get_triangles_drawable("faces");
        viewer.assign(0, 1, sphere_faces);
    } else
        std::cerr << "failed to load model from file: " << file_sphere << std::endl;

    // ---------------------------------------------------------------------------
    // setup content for view(1, 0): the wireframe of the sphere model
    // Reuse the SAME sphere model, but assign a different drawable (its edges) to this cell.
    auto sphere_wireframe = sphere->renderer()->get_lines_drawable("edges");
    // Render the edges as thick cylinders (impostors) rather than thin GL lines.
    sphere_wireframe->set_impostor_type(LinesDrawable::CYLINDER);
    sphere_wireframe->set_line_width(5);
    sphere_wireframe->set_uniform_coloring(vec4(0.7f, 0.7f, 1.0f, 1.0f));
    viewer.assign(1, 0, sphere_wireframe);

    // ---------------------------------------------------------------------------
    // setup content for view(1, 1): the vertices of the sphere model
    auto sphere_vertices = sphere->renderer()->get_points_drawable("vertices");
    // Render each vertex as a shaded sphere impostor, sized in pixels.
    sphere_vertices->set_impostor_type(PointsDrawable::SPHERE);
    sphere_vertices->set_point_size(15);
    viewer.assign(1, 1, sphere_vertices);

    // Enter the interactive render loop; returns only when the window is closed.
    return viewer.run();
}