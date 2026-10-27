// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/vulkan_viewer.h>
#include <tutorials/Tutorial_311_Animation/animation.h>
#include <easy3d/util/initializer.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace easy3d;
void require(bool ok, const char* message) { if (!ok) throw std::runtime_error(message); }
struct AnimationViewer : VulkanViewer {
    using VulkanViewer::VulkanViewer;
    unsigned updates=0, observed=0;
    void pre_draw() override { observed=updates; }
};
void frame(VulkanViewer& viewer) {
    for (unsigned i=0;i<100;++i) {
        VulkanViewer::poll_events();
        if (viewer.draw_frame()) return;
        glfwWaitEventsTimeout(.01);
    }
    throw std::runtime_error("animation frame did not present");
}
int main(int argc, char**) {
    try {
        initialize(false,false,false);
        VulkanViewerOptions options; options.visible=false; options.width=181; options.height=151;
        options.device.validation=options.device.synchronization_validation=argc>1;
        AnimationViewer viewer("Animation regression",options);
        auto surface=std::make_shared<TrianglesDrawable>("animated triangle");
        const std::vector<vec3> original={{-1,-1,0},{1,-1,0},{0,1,0}};
        surface->update_vertex_buffer(original,true);
        surface->set_uniform_coloring(vec4(1,0,0,1)); surface->set_lighting(false);
        viewer.add_drawable(surface);
        viewer.camera()->setViewDirection(vec3(0,0,-1));
        viewer.camera()->setUpVector(vec3(0,1,0));
        viewer.fit_screen(); frame(viewer); frame(viewer);
        viewer.set_animation(true);
        require(!viewer.is_animating(),"missing callback reported as active");
        float z=0;
        unsigned count=0;
        viewer.animation_func_=[&, count](VulkanViewer*) mutable {
            ++count; viewer.updates=count; z=.02f*count;
            auto vertices=original;
            for (auto& p:vertices) p.z=z;
            surface->update_vertex_buffer(vertices,true);
            return true;
        };
        for (unsigned samples: {1u,4u}) {
            if (!(viewer.device().supported_sample_counts()&samples)) continue;
            viewer.set_samples(samples);
            frame(viewer);
            require(viewer.observed==viewer.updates && viewer.updates>0,"animation ran after pre_draw");
            const unsigned count=viewer.updates;
            const auto a=viewer.capture();
            require(viewer.updates==count,"capture advanced animation");
            const int x=a.width/2,y=a.height/2;
            const auto hit=viewer.pick(x,y);
            require(bool(hit),"animated triangle not pickable");
            require(hit.drawable==surface && hit.primitive==0,"animation changed drawable/primitive identity");
            require(std::abs(hit.position.z-z)<1e-4f,"picking used stale animated geometry");
            frame(viewer);
            require(viewer.updates==count+1,"stateful animation callback lost state or ran twice");
            const auto b=viewer.capture();
            require(a.depth!=b.depth,"animation did not update depth");
            auto key=glfwSetKeyCallback(viewer.window(),nullptr); glfwSetKeyCallback(viewer.window(),key);
            key(viewer.window(),GLFW_KEY_F2,0,GLFW_PRESS,0);
            require(!viewer.is_animating(),"F2 did not pause animation");
            frame(viewer);
            require(viewer.capture().depth==b.depth && viewer.updates==count+1,"paused scene advanced");
            viewer.resize(191+int(samples),159+int(samples)); frame(viewer);
            key(viewer.window(),GLFW_KEY_F2,0,GLFW_PRESS,0);
            require(viewer.is_animating(),"F2 did not resume animation"); frame(viewer);
        }
        // A callback can pause itself; its final geometry is still presented.
        viewer.animation_func_=[&](VulkanViewer* v) { ++viewer.updates; v->set_animation(false); return false; };
        frame(viewer);
        const unsigned stopped=viewer.updates; frame(viewer);
        require(viewer.updates==stopped && !viewer.is_animating(),"callback pause was ignored");
        // Exercise the actual tutorial's stretch/reset cycle and finite run loop.
        surface->update_vertex_buffer(resource::bunny_vertices,true);
        surface->update_element_buffer(resource::bunny_indices);
        viewer.fit_screen();
        BunnyAnimation animation;
        viewer.animation_func_=[&](VulkanViewer*) { return animation.update(*surface); };
        viewer.set_animation(true);
        frame(viewer); const auto first=viewer.capture();
        require(viewer.run(40)==0 && animation.scale()>1.5f,"bounded run did not advance tutorial animation");
        const auto stretched=viewer.capture();
        require(first.rgba!=stretched.rgba,"tutorial stretch did not change rendered geometry");
        frame(viewer);
        require(animation.scale()==1.0f,"tutorial did not reset after stretching");
        frame(viewer);
        require(viewer.capture().rgba==first.rgba,"tutorial cycle accumulated deformation");
        viewer.animation_func_=[](VulkanViewer*) -> bool { throw std::runtime_error("expected animation failure"); };
        require(viewer.run(1)==1,"animation exception was swallowed by run");
        viewer.animation_func_=[](VulkanViewer* v) { v->request_close(); return true; };
        require(!viewer.draw_frame() && viewer.should_close(),"animation close still presented a frame");
        require(viewer.device().validation_errors()==0,"animation validation errors");
        std::cout << "PASS: animation lifecycle, stateful callbacks, F2, CPU updates, depth/picking, MSAA, resize, tutorial reset, bounded run/errors/close\n";
    } catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
