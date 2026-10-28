// SPDX-License-Identifier: GPL-3.0-or-later
#include <tutorials/Tutorial_403_PointSelection/viewer_vulkan.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/manipulated_frame.h>
#include <easy3d/util/initializer.h>
#include <iostream>
using namespace easy3d;
void require(bool ok,const char* message) { if (!ok) throw std::runtime_error(message); }
void frame(VulkanViewer& viewer) {
    for (unsigned i=0;i<100;++i) {
        VulkanViewer::poll_events(); if (viewer.draw_frame()) return;
        glfwWaitEventsTimeout(.01);
    }
    throw std::runtime_error("selection viewer did not present");
}
void same_picking(const graphics::FrameImage& a,const graphics::FrameImage& b) {
    require(a.depth==b.depth && a.ids.size()==b.ids.size(),"selection changed scene depth");
    for (size_t i=0;i<a.ids.size();++i)
        require(a.ids[i].object==b.ids[i].object && a.ids[i].primitive==b.ids[i].primitive,"selection changed picking IDs");
}
// Deterministic framebuffer-coordinate events. macOS cursor warps/focus are
// asynchronous; native callback routing is covered by vulkan.window.
struct SelectionHost : TutorialPointSelection {
    using TutorialPointSelection::TutorialPointSelection;
    using TutorialPointSelection::mouse_press_event;
    using TutorialPointSelection::mouse_drag_event;
    using TutorialPointSelection::mouse_release_event;
};
struct Input {
    SelectionHost& viewer;
    vec2 position=vec2(0);
    int button=-1;
    GLFWkeyfun key;
    explicit Input(SelectionHost& v):viewer(v) {
        key=glfwSetKeyCallback(v.window(),nullptr); glfwSetKeyCallback(v.window(),key);
    }
    void move(vec2 p) {
        const auto delta=p-position; position=p;
        if (button>=0) viewer.mouse_drag_event(int(p.x),int(p.y),int(delta.x),int(delta.y),button,0);
    }
    void press(int code) {
        button=code; viewer.mouse_press_event(int(position.x),int(position.y),code,GLFW_MOD_SHIFT);
    }
    void release(int code) {
        viewer.mouse_release_event(int(position.x),int(position.y),code,0); button=-1;
    }
    void shortcut(int code) { key(viewer.window(),code,0,GLFW_PRESS,0); }
};
vec2 project(VulkanViewer& viewer,vec3 p) {
    const auto q=viewer.camera()->modelViewProjectionMatrix()*vec4(p,1);
    return vec2((q.x/q.w+1)*.5f*viewer.width(),(1-q.y/q.w)*.5f*viewer.height());
}
int main(int argc,char**) {
    try {
        initialize(false,false,false);
        VulkanViewerOptions options; options.visible=false; options.width=360; options.height=260;
        options.device.validation=options.device.synchronization_validation=argc>1;
        SelectionHost viewer("GPU point selection interaction",options);
        auto cloud=std::make_shared<PointCloud>();
        const auto deleted=cloud->add_vertex(vec3(0));
        const auto left=cloud->add_vertex(vec3(-.5f,0,0)),right=cloud->add_vertex(vec3(.5f,0,0));
        const auto back=cloud->add_vertex(vec3(-.5f,0,-.5f)); cloud->delete_vertex(deleted);
        viewer.add_model(cloud);
        auto points=cloud->renderer()->get_points_drawable("vertices");
        points->set_point_size(13); points->set_impostor_type(PointsDrawable::PLAIN);
        points->set_uniform_coloring(vec4(0,0,1,1)); points->set_lighting(false);
        viewer.camera()->setViewDirection(vec3(0,0,-1)); viewer.camera()->setUpVector(vec3(0,1,0));
        viewer.camera()->setType(Camera::ORTHOGRAPHIC); viewer.fit_screen(); frame(viewer); frame(viewer);
        const auto original=viewer.point_resource(points);
        require(original->source_ids()==std::vector<uint32_t>({1,2,3}) && original->source_count()==4,
                "point snapshot did not preserve deleted source slots");
        const auto uploads=viewer.device().point_upload_count();
        require(uploads==1,"point rendering did not realize a single position resource");
        Input input(viewer);
        auto selected=cloud->vertex_property<bool>("v:select");
        auto gesture=[&](vec3 world,int button,bool rectangle=false) {
            const auto c=project(viewer,world); const float r=20;
            input.move(c+vec2(-r,-r)); input.press(button);
            input.move(c+vec2(r,-r)); input.move(c+vec2(r,r));
            if (!rectangle) input.move(c+vec2(-r,r));
            input.release(button); frame(viewer);
        };
        for (unsigned samples:{1u,4u}) {
            if (!(viewer.device().supported_sample_counts()&samples)) continue;
            viewer.set_samples(samples); frame(viewer);
            const auto baseline=viewer.capture();
            // Active guide changes color only; cancelling clears it without selecting.
            const auto c=project(viewer,vec3(-.5f,0,0));
            input.move(c-vec2(20)); input.press(GLFW_MOUSE_BUTTON_LEFT);
            input.move(c+vec2(20,-20)); input.move(c+vec2(20)); input.move(c+vec2(-20,20));
            frame(viewer); const auto guide=viewer.capture();
            require(guide.rgba!=baseline.rgba,"native translucent selection guide was not rendered"); same_picking(baseline,guide);
            const size_t interior=4*(size_t(c.y)*guide.width+size_t(c.x+10));
            require(guide.rgba[interior]>250 && std::abs(int(guide.rgba[interior+1])-204)<=1 &&
                    std::abs(int(guide.rgba[interior+2])-204)<=1,"lasso interior did not contain the transparent fill");
            input.shortcut(GLFW_KEY_ESCAPE); input.release(GLFW_MOUSE_BUTTON_LEFT); frame(viewer);
            require(!viewer.should_close() && !selected[left],"Escape did not cancel gesture");
            require(viewer.capture().rgba==baseline.rgba,"cancelled guide remained visible");
            gesture(vec3(-.5f,0,0),GLFW_MOUSE_BUTTON_LEFT);
            if (!(selected[left] && selected[back] && !selected[right] && !selected[deleted]))
                std::cerr << "selection flags " << selected[left] << " " << selected[back] << " " << selected[right] << " " << selected[deleted] << std::endl;
            require(selected[left] && selected[back] && !selected[right] && !selected[deleted],
                    "lasso did not through-select original live point slots");
            const auto colored=viewer.capture(); require(colored.rgba!=baseline.rgba,"selection color did not update"); same_picking(baseline,colored);
            gesture(vec3(-.5f,0,0),GLFW_MOUSE_BUTTON_RIGHT);
            require(!selected[left] && !selected[back],"right lasso did not deselect"); same_picking(baseline,viewer.capture());
            input.shortcut(GLFW_KEY_R); gesture(vec3(.5f,0,0),GLFW_MOUSE_BUTTON_LEFT,true);
            require(selected[right] && !selected[left],"rectangle did not select correct point");
            gesture(vec3(.5f,0,0),GLFW_MOUSE_BUTTON_RIGHT,true); input.shortcut(GLFW_KEY_L);
        }
        // Camera, HiDPI/resize and manipulated-model transforms feed the GPU MVP.
        viewer.resize(470,310); frame(viewer);
        cloud->set_manipulator(std::make_shared<Manipulator>(cloud.get()));
        cloud->manipulator()->frame()->translate(vec3(.2f,.2f,0)); frame(viewer);
        gesture(vec3(-.3f,.2f,0),GLFW_MOUSE_BUTTON_LEFT);
        require(selected[left] && selected[back],"resized/transformed lasso missed source points");
        gesture(vec3(-.3f,.2f,0),GLFW_MOUSE_BUTTON_RIGHT);
        viewer.set_clipping_planes({vec4(0,0,1,.1f)}); frame(viewer);
        gesture(vec3(-.3f,.2f,0),GLFW_MOUSE_BUTTON_LEFT);
        require(selected[left] && !selected[back],"GPU region selection ignored world-space clipping");
        require(viewer.point_resource(points)==original && viewer.device().point_upload_count()==uploads,
                "selection colors/transforms/MSAA/resize/clipping replaced the shared point buffer");
        cloud->position(left).x-=.1f; cloud->renderer()->update(); frame(viewer);
        require(original->positions()[0].x==-.5f,"position edit mutated the retained immutable point snapshot");
        require(viewer.point_resource(points)!=original && viewer.device().point_upload_count()==uploads+1,
                "position mutation did not upload exactly one replacement resource");
        cloud->delete_vertex(right); cloud->renderer()->update(); frame(viewer);
        require(viewer.point_resource(points)->source_ids()==std::vector<uint32_t>({1,3}) &&
                viewer.device().point_upload_count()==uploads+2,"deletion lost source IDs or re-uploaded repeatedly");
        graphics::PointSelectionRequest retained; retained.points=original;
        retained.width=viewer.width(); retained.height=viewer.height();
        retained.polygon={{0,0},{float(viewer.width()),0},{float(viewer.width()),float(viewer.height())},{0,float(viewer.height())}};
        require(viewer.select_points(retained)[0]==14 && viewer.device().point_upload_count()==uploads+2,
                "retained point snapshot did not keep its original shared GPU realization");
        require(viewer.device().validation_errors()==0,"native selection validation errors");
        std::cout << "PASS: native lasso/rectangle select/deselect, modifier release, deleted/occluded points, transparent HUD, stable depth/IDs, MSAA, resize/model transforms and clipping\n";
    } catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
