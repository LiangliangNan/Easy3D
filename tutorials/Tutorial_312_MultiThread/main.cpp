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
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/random.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/util/initializer.h>
#include "point_worker.h"

/**
 * \example{lineno} Tutorial_312_MultiThread/main.cpp
 * This example generates point batches on a worker and applies them safely on the viewer thread.
 */

using namespace easy3d;


int main(int argc, char **argv) {
    // initialize Easy3D.
    initialize();

    // create the viewer
    Viewer viewer(EXAMPLE_TITLE);

    // create a point cloud (with a per point color property) from a set of random points
    auto cloud = new PointCloud;
    // add a per point color property
    auto colors = cloud->add_vertex_property<vec3>("v:color");
    for (int i = 0; i < 100; ++i) {
        auto v = cloud->add_vertex(easy3d::vec3(random_float(), random_float(), random_float()));
        // assign a color to each point (here simply a red color)
        colors[v] = vec3(1.0f, 0.0f, 0.0f);
    }
    // add the point cloud to the viewer for visualization
    viewer.add_model(std::shared_ptr<PointCloud>(cloud));

    // set up visualization parameters
    auto drawable = cloud->renderer()->get_points_drawable("vertices");
    // set point size
    drawable->set_point_size(10.0f);
    // set coloring method: we want to visualize the point cloud using the per point color property
    drawable->set_coloring(Drawable::COLOR_PROPERTY, Drawable::VERTEX, "v:color");

    // The worker owns its random generator and produces immutable CPU batches.
    // One pending batch bounds memory if the viewer is paused or minimized.
    // No worker ever reads or modifies the cloud, renderer or viewer.
    PointBatchWorker worker;
    if (!worker.wait_for_batch(std::chrono::seconds(5))) {
        LOG(ERROR) << "point worker did not produce its first batch";
        return EXIT_FAILURE;
    }

    viewer.animation_func_ = [&](Viewer* v) {
        const auto batch = worker.take(); // nonblocking handoff to the UI thread
        if (batch) {
            append_point_batch(*cloud, *batch);
            v->update();
            std::cout << "#points: " << cloud->n_vertices() << std::endl;
        }
        if (worker.drained()) v->set_animation(false);
        return bool(batch);
    };
    viewer.set_animation(true); // F2 pauses/resumes consumption

    // worker's destructor interrupts its waits and joins before viewer teardown,
    // including early window closure and exceptions.
    return viewer.run();
}
