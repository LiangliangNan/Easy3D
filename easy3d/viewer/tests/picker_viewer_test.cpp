// SPDX-License-Identifier: GPL-3.0-or-later
#include <tutorials/Tutorial_401_ModelPicker/viewer.h>
#include <tutorials/Tutorial_402_FacePicker/viewer.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/util/initializer.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <fstream>
#include <iostream>
using namespace easy3d;
void require(bool value,const char* message) { if (!value) throw std::runtime_error(message); }
void frame(VulkanViewer& viewer) {
    for (unsigned i=0;i<100;++i) {
        VulkanViewer::poll_events(); if (viewer.draw_frame()) return;
        glfwWaitEventsTimeout(.01);
    }
    throw std::runtime_error("picker viewer did not present");
}
void click(VulkanViewer& viewer,const vec3& world,int button=GLFW_MOUSE_BUTTON_LEFT,int modifiers=0) {
    const auto p=viewer.camera()->projectedCoordinatesOf(world);
    int w,h; glfwGetWindowSize(viewer.window(),&w,&h);
    glfwSetCursorPos(viewer.window(),p.x*w/viewer.width(),p.y*h/viewer.height());
    auto mouse=glfwSetMouseButtonCallback(viewer.window(),nullptr); glfwSetMouseButtonCallback(viewer.window(),mouse);
    mouse(viewer.window(),button,GLFW_PRESS,modifiers); mouse(viewer.window(),button,GLFW_RELEASE,modifiers);
    frame(viewer);
}
void same_picking(const graphics::FrameImage& a,const graphics::FrameImage& b) {
    require(a.depth==b.depth && a.ids.size()==b.ids.size(),"picker interaction changed depth");
    for (size_t i=0;i<a.ids.size();++i)
        require(a.ids[i].object==b.ids[i].object && a.ids[i].primitive==b.ids[i].primitive,"picker interaction changed IDs");
}
int main(int argc,char**) {
    try {
        initialize(false,false,false);
        const std::string file="vulkan-picker-mesh.off";
        { std::ofstream out(file); out << "OFF\n8 2 0\n-2 -1 0\n0 -1 0\n0 1 0\n-2 1 0\n"
            "1 -1 0\n3 -1 0\n3 1 0\n1 1 0\n4 0 1 2 3\n4 4 5 6 7\n"; require(bool(out),"could not write picker fixture"); }
        VulkanViewerOptions options; options.width=420; options.height=280;
        options.device.validation=options.device.synchronization_validation=argc>1;
        {
            TutorialModelPicker viewer("Native model picker test",options);
            auto* model=viewer.add_model(file); require(model!=nullptr,"model picker load failed");
            viewer.camera()->setType(Camera::ORTHOGRAPHIC); viewer.fit_screen();
            if (viewer.device().supported_sample_counts()&4) viewer.set_samples(4);
            frame(viewer); const auto baseline=viewer.capture();
            click(viewer,vec3(-1.4f,.3f,0));
            require(model->renderer()->is_selected(),"registered click did not select model");
            auto selected=viewer.capture(); require(selected.rgba!=baseline.rgba,"model selection was not displayed");
            same_picking(baseline,selected);
            click(viewer,vec3(-1.4f,.3f,0),GLFW_MOUSE_BUTTON_RIGHT);
            require(model->renderer()->is_selected(),"right click toggled model selection");
            click(viewer,vec3(-1.4f,.3f,0));
            require(!model->renderer()->is_selected(),"second registered click did not unselect model");
            same_picking(baseline,viewer.capture());
            require(viewer.device().validation_errors()==0,"model picker validation errors");
        }
        {
            TutorialFacePicker viewer("Native face picker test",options);
            auto* model=viewer.add_model(file); require(model!=nullptr,"face picker load failed");
            auto* faces=model->renderer()->get_triangles_drawable("faces");
            viewer.camera()->setType(Camera::ORTHOGRAPHIC); viewer.fit_screen();
            if (viewer.device().supported_sample_counts()&4) viewer.set_samples(4);
            frame(viewer); const auto baseline=viewer.capture();
            click(viewer,vec3(-1.4f,.3f,0));
            require(faces->highlight() && faces->highlight_range()==std::make_pair(0,1),"polygon face did not highlight both generated triangles");
            const auto first=viewer.capture(); require(first.rgba!=baseline.rgba,"face selection was not displayed"); same_picking(baseline,first);
            click(viewer,vec3(1.6f,.3f,0));
            require(faces->highlight() && faces->highlight_range()==std::make_pair(2,3),"face picker did not change source polygon");
            same_picking(baseline,viewer.capture());
            require(viewer.snapshot("vulkan-face-picking.png"),"face picker snapshot failed");
            viewer.resize(500,340); frame(viewer);
            click(viewer,vec3(-1.4f,.3f,0));
            require(faces->highlight_range()==std::make_pair(0,1),"HiDPI/resize click selected wrong polygon");
            viewer.set_samples(1); frame(viewer);
            click(viewer,vec3(1.6f,.3f,0));
            require(faces->highlight_range()==std::make_pair(2,3),"sample count changed face picking");
            auto mouse=glfwSetMouseButtonCallback(viewer.window(),nullptr); glfwSetMouseButtonCallback(viewer.window(),mouse);
            glfwSetCursorPos(viewer.window(),0,0);
            mouse(viewer.window(),GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS,0);
            mouse(viewer.window(),GLFW_MOUSE_BUTTON_LEFT,GLFW_RELEASE,0); frame(viewer);
            require(!faces->highlight(),"background click did not clear highlight");
            require(viewer.device().validation_errors()==0,"face picker validation errors");
        }
        std::cout << "PASS: registered model/face clicks, polygon highlights, unchanged MSAA picking/depth, HiDPI/resize, sample changes, background clear, snapshot\n";
    } catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
