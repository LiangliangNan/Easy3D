// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/viewer.h>
#include <tutorials/Tutorial_312_MultiThread/point_worker.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/util/initializer.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <cmath>
#include <iostream>
#include <set>
#include <stdexcept>
using namespace easy3d;
void require(bool ok,const char* message) { if (!ok) throw std::runtime_error(message); }
std::shared_ptr<const PointBatch> next(PointBatchWorker& worker) {
    require(worker.wait_for_batch(std::chrono::seconds(5)),"worker batch timeout");
    auto batch=worker.take(); require(bool(batch),"worker lost a ready batch"); return batch;
}
void frame(VulkanViewer& viewer) {
    for (unsigned i=0;i<100;++i) {
        VulkanViewer::poll_events();
        if (viewer.draw_frame()) return;
        glfwWaitEventsTimeout(.01);
    }
    throw std::runtime_error("streamed frame did not present");
}
void worker_lifetime() {
    bool rejected=false;
    try { PointBatchWorker invalid(1,std::chrono::milliseconds(-1)); }
    catch (const std::invalid_argument&) { rejected=true; }
    require(rejected,"negative interval accepted");
    PointBatchWorker empty(0);
    require(!empty.wait_for_batch(std::chrono::seconds(5)) && empty.drained(),"empty producer did not finish");
    PointBatchWorker worker(3,std::chrono::milliseconds(0));
    require(worker.wait_for_batch(std::chrono::seconds(5)),"initial handoff failed");
    require(!worker.drained(),"pending batch reported as drained");
    // Repeated readiness checks never consume/replace the one-slot mailbox.
    for (unsigned i=0;i<5;++i) require(worker.wait_for_batch(std::chrono::milliseconds(0)),"pending batch disappeared");
    auto first=next(worker); const PointBatch saved=*first;
    auto second=next(worker); auto third=next(worker);
    require(first!=second && second!=third && first->size()==100,"batch storage was reused");
    require(!worker.wait_for_batch(std::chrono::seconds(5)) && worker.drained(),"finite worker dropped work or failed to finish");
    for (unsigned i=0;i<100;++i)
        require((*first)[i].position==saved[i].position && (*first)[i].color==saved[i].color,"retained batch was mutated");
    // Independent seeded generators also detect dropped/reordered publications.
    PointBatchWorker reference(3,std::chrono::milliseconds(0));
    for (const auto& actual:{first,second,third}) {
        auto expected=next(reference);
        for (unsigned i=0;i<100;++i)
            require((*actual)[i].position==(*expected)[i].position && (*actual)[i].color==(*expected)[i].color,
                    "batch order or worker-local random state changed");
    }
    { // Cancel while publication is blocked by a paused consumer.
        PointBatchWorker blocked(10000,std::chrono::milliseconds(0));
        require(blocked.wait_for_batch(std::chrono::seconds(5)),"backpressure startup failed");
        blocked.stop(); blocked.stop();
        require(bool(blocked.take()) && !blocked.take() && blocked.drained(),"cancelled worker lost or added a batch");
    }
    { // Destruction must wake long pacing waits, not wait for their deadline.
        PointBatchWorker sleeping(2,std::chrono::hours(1)); next(sleeping);
    }
}
struct StreamViewer : VulkanViewer {
    using VulkanViewer::VulkanViewer;
    void hide_logo() {show_easy3d_logo_=false;}
};
int main(int argc,char**) {
    try {
        initialize(false,false,false);
        worker_lifetime();
        VulkanViewerOptions options; options.visible=false; options.width=211; options.height=179;
        options.device.validation=options.device.synchronization_validation=argc>1;
        StreamViewer viewer("Worker handoff regression",options);viewer.hide_logo();
        auto key=glfwSetKeyCallback(viewer.window(),nullptr); glfwSetKeyCallback(viewer.window(),key);
        // This oracle compares every point's rendered color with its source;
        // hide the persistent HUD so it cannot cover otherwise valid point hits.
        key(viewer.window(),GLFW_KEY_A,0,GLFW_PRESS,0);
        auto cloud=std::make_shared<PointCloud>();
        cloud->add_vertex_property<vec3>("v:color");
        cloud->add_vertex(vec3(0,0,0)); cloud->add_vertex(vec3(1,1,1));
        viewer.add_model(cloud);
        auto points=cloud->renderer()->get_points_drawable("vertices");
        points->set_coloring(Drawable::COLOR_PROPERTY,Drawable::VERTEX,"v:color");
        points->set_impostor_type(PointsDrawable::PLAIN);
        points->set_point_size(10); points->set_lighting(false);
        viewer.fit_screen(); frame(viewer);
        // The initial fixed bounds enclose every generated point; no camera
        // motion can disguise stale geometry, colors or picking IDs.
        auto baseline=viewer.capture();
        const auto ui_thread=std::this_thread::get_id();
        unsigned applied=0, allowed=4;
        PointBatchWorker worker(4,std::chrono::milliseconds(0));
        std::vector<ColoredPoint> appended;
        viewer.animation_func_=[&](VulkanViewer*) {
            require(std::this_thread::get_id()==ui_thread,"model updated off the UI thread");
            if (applied<allowed) {
                if (auto batch=worker.take()) {
                    append_point_batch(*cloud,*batch);
                    appended.insert(appended.end(),batch->begin(),batch->end()); ++applied;
                }
            }
            return true;
        };
        viewer.set_animation(true);
        key(viewer.window(),GLFW_KEY_F2,0,GLFW_PRESS,0);
        require(!viewer.is_animating(),"F2 did not pause the stream");
        require(worker.wait_for_batch(std::chrono::seconds(5)),"paused worker did not publish");
        frame(viewer);
        require(cloud->n_vertices()==2 && viewer.capture().rgba==baseline.rgba,"worker directly modified the displayed model");
        key(viewer.window(),GLFW_KEY_F2,0,GLFW_PRESS,0);
        for (unsigned batch=0;batch<4;++batch) {
            // WSI may retry presentation during resize. Permit one batch per
            // probe, independent of the number of renderable frame attempts.
            allowed=batch+1;
            require(worker.wait_for_batch(std::chrono::seconds(5)),"streamed batch did not arrive");
            viewer.set_samples((batch%2 && (viewer.device().supported_sample_counts()&4))?4:1);
            if (batch==2) viewer.resize(229,193);
            frame(viewer);
            require(applied==batch+1 && cloud->n_vertices()==2+100*applied,"frame dropped or duplicated a batch");
            const auto image=viewer.capture();
            std::set<uint32_t> visible;
            for (size_t i=0;i<image.ids.size();++i) {
                const auto id=image.ids[i];
                if (id.object==UINT32_MAX || id.primitive<2) continue;
                require(id.primitive<cloud->n_vertices(),"streamed point ID out of range");
                visible.insert(id.primitive);
                const auto& point=appended[id.primitive-2];
                const auto world=viewer.unproject_pixel(int(i%image.width),int(i/image.width),image.depth[i]);
                require(std::abs(dot(world-point.position,viewer.camera()->viewDirection()))<1e-4f,
                        "streamed point depth belongs to stale geometry");
                if (viewer.samples()==1) {
                    for (unsigned c=0;c<3;++c)
                        require(std::abs(int(image.rgba[4*i+c])-255*point.color[c])<=1.5f,"streamed point color is stale");
                }
            }
            require(visible.size()>30,"new point batches were not rendered/pickable");
            require(cloud->position(PointCloud::Vertex(2+100*batch))==appended[100*batch].position,"model and batch disagree");
        }
        viewer.set_animation(false);
        require(!worker.wait_for_batch(std::chrono::seconds(5)) && worker.drained(),"stream completion lost final batch");
        const auto stable=viewer.capture(); frame(viewer);
        require(viewer.capture().rgba==stable.rgba,"completed stream did not remain stable");
        // Close with unconsumed work and then leave scope: the worker must join
        // while the viewer and its device are still alive.
        { PointBatchWorker pending(10000,std::chrono::milliseconds(0));
          require(pending.wait_for_batch(std::chrono::seconds(5)),"close test worker startup failed");
          viewer.request_close(); require(viewer.run_frames(1)==0,"early close failed"); }
        require(viewer.device().validation_errors()==0,"streaming validation errors");
        std::cout << "PASS: immutable bounded worker batches, cancellation/join, UI-thread append, pause/resume, streamed color/depth/IDs, MSAA, resize, early close\n";
    } catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
