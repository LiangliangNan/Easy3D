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
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/random.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/util/initializer.h>
#include "point_worker.h"

/**
 * \example{lineno} Tutorial_312_MultiThread/main.cpp
 *
 * \brief Streaming data from a worker thread into a live viewer with a safe producer/consumer handoff.
 *
 * <b>Overview</b>
 * This tutorial grows a point cloud while the viewer is running: a background worker thread
 * generates batches of 100 random colored points every 300 ms, and the UI thread picks them up
 * and appends them to the displayed cloud. It demonstrates the pattern every interactive
 * application needs when computation and rendering run at different speeds — and the golden
 * rule that makes it safe: only the UI thread ever touches scene or graphics state.
 *
 * <b>The technique</b>
 * Graphics APIs (OpenGL contexts, Vulkan command submission) and Easy3D's scene structures are
 * not thread-safe, so the classic mistake is letting a worker thread modify a model or drawable
 * directly. The correct design is a producer/consumer pipeline: the worker (producer) computes
 * plain CPU data — here immutable batches of positions and colors — and publishes them through
 * a mutex-protected slot; the UI thread (consumer) takes a batch during its animation callback
 * and applies it to the cloud, then refreshes the renderer so the new points appear. Ownership
 * transfer via std::shared_ptr<const PointBatch> means no data is shared mutably: once published,
 * a batch is read-only, and the consumer owns its copy. At most one batch may wait in the slot
 * (backpressure): if the viewer is paused or minimized, the worker blocks instead of queueing
 * unbounded data in memory. A condition_variable coordinates the handoff, and the worker's
 * destructor signals shutdown and joins the thread before any viewer resource is destroyed —
 * including when the user closes the window early.
 *
 * <b>Easy3D API</b>
 * - PointCloud + vertex properties: the cloud carries a "v:color" per-point color property;
 *   PointsDrawable::set_coloring() visualizes it.
 * - Viewer::animation_func_ / set_animation(): the per-frame UI-thread hook doubles as the
 *   consumer loop here; it takes a pending batch, appends it, and requests a redraw.
 * - Renderer::update(): re-syncs the drawable's GPU buffers after the cloud grew.
 * - PointBatchWorker / append_point_batch(): the tutorial's helper (shown below) implementing
 *   the producer and the UI-thread-only application step.
 *
 * <b>Running it</b>
 * The cloud starts with 100 red points and gains 100 randomly colored points every ~300 ms
 * (66 batches total); the point count is printed to the console. Press F2 to pause/resume
 * consumption — while paused the worker waits instead of producing more. When all batches are
 * consumed the animation stops automatically.
 *
 * The worker helper (producer/consumer implementation):
 * \include{lineno} Tutorial_312_MultiThread/point_worker.h
 *
 * \see Viewer, PointCloud, Renderer, PointsDrawable, Tutorial_311_Animation
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
    // Constructing the worker starts its thread immediately.
    PointBatchWorker worker;
    // Block (bounded) until the first batch exists, so the demo never starts empty-handed.
    // This wait is for startup only; the render loop below uses the nonblocking take().
    if (!worker.wait_for_batch(std::chrono::seconds(5))) {
        std::cerr << "point worker did not produce its first batch" << std::endl;
        return EXIT_FAILURE;
    }

    // The consumer runs inside the viewer's animation callback — i.e. on the UI thread,
    // the only thread allowed to mutate the cloud and its renderer.
    viewer.animation_func_ = [&](Viewer *v) {
        const auto batch = worker.take(); // nonblocking handoff to the UI thread
        if (batch) {
            // Append the points and refresh the GPU buffers (see append_point_batch).
            append_point_batch(*cloud, *batch);
            v->update();
            std::cout << "#points: " << cloud->n_vertices() << std::endl;
        }
        // All batches produced and consumed: stop the animation loop.
        if (worker.drained())
            v->set_animation(false);
        return bool(batch);
    };
    viewer.set_animation(true); // F2 pauses/resumes consumption

    // worker's destructor interrupts its waits and joins before viewer teardown,
    // including early window closure and exceptions.
    // (worker is declared after viewer, so C++ destroys it first — the thread is always
    // joined while the viewer and cloud are still alive.)
    return viewer.run();
}
