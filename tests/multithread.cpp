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

#include <thread>

#include <easy3d/viewer/viewer.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/random.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/util/timer.h>

using namespace easy3d;



void edit_model(PointCloud *cloud, Viewer *viewer) {
    // in this simple example, we add more points (with per point colors) to the point cloud.
    auto colors = cloud->vertex_property<vec3>("v:color");
    for (int i = 0; i < 100; ++i) {
        auto v = cloud->add_vertex(vec3(random_float(), random_float(), random_float()));
        colors[v] = vec3(random_float(), random_float(), random_float()); // we use a random color
    }

    // notify the renderer to update the OpenGL buffers
    cloud->renderer()->update();
    // notify the viewer to update the display
    viewer->update();
}

int test_multithread() {
    // create the viewer
    Viewer viewer("MultiThread");

    // create a point cloud (with a per point color property) from a set of random points
    auto cloud = new PointCloud;
    // add a per point color property
    auto colors = cloud->add_vertex_property<vec3>("v:color");
    for (int i = 0; i < 50; ++i) {
        auto v = cloud->add_vertex(easy3d::vec3(random_float(), random_float(), random_float()));
        // assign a color to each point (here simply a red color)
        colors[v] = vec3(1.0f, 0.0f, 0.0f);
    }
    // add the point cloud to the viewer for visualization
    viewer.add_model(cloud);

    // set up visualization parameters
    auto drawable = cloud->renderer()->get_points_drawable("vertices");
    // set point size
    drawable->set_point_size(10.0f);
    // set coloring method: we want to visualize the point cloud using the per point color property
    drawable->set_coloring(Drawable::COLOR_PROPERTY, Drawable::VERTEX, "v:color");

    // run the process in another thread
    Timer<PointCloud*, Viewer*> timer;
    // call the edit_model() function every 300 milliseconds
    timer.set_interval(300, edit_model, cloud, &viewer);

    // stop the timer before exit
    Timer<>::single_shot(4000, &timer, &Timer<PointCloud*, Viewer*>::stop); // or Timer<>::single_shot(4000, [&]() -> void { timer.stop(); });

    viewer.set_usage("testing multithreading...");

    // exit the viewer after 5 seconds
    Timer<>::single_shot(5000, (Viewer*)&viewer, &Viewer::exit);

    // run the viewer
    return viewer.run();
}