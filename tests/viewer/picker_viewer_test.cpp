#include "test_window.h"
// SPDX-License-Identifier: GPL-3.0-or-later
#include <tutorials/Tutorial_401_ModelPicker/viewer.h>
#include <tutorials/Tutorial_402_FacePicker/viewer.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/util/initializer.h>
#include <fstream>
#include <iostream>
using namespace easy3d;
void require(bool value,const char* message) { if (!value) throw std::runtime_error(message); }
void frame(Viewer& viewer) {
    for (unsigned i=0;i<100;++i) {
        Viewer::poll_events(); if (viewer.draw_frame()) return;
        glfwWaitEventsTimeout(.01);
    }
    throw std::runtime_error("picker viewer did not present");
}
// Tutorial behavior is deterministic in camera-screen coordinates. Native
// callback translation is covered by vulkan.window; cursor warps are asynchronous.
template<class Tutorial> struct PickerHost:Tutorial {
    using Tutorial::Tutorial;
    using Tutorial::mouse_press_event;
    using Tutorial::mouse_release_event;
};
template<class Host>
void click(Host& viewer,const vec3& world,int button=Viewer::BUTTON_LEFT,int modifiers=0) {
    const auto p=viewer.camera()->projectedCoordinatesOf(world);
    viewer.mouse_press_event(int(p.x),int(p.y),button,modifiers);
    viewer.mouse_release_event(int(p.x),int(p.y),button,modifiers);
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
        TestWindow options; options.width=420; options.height=280;
        options.device.validation=options.device.sync_validation=argc>1;
        {
            PickerHost<TutorialModelPicker> viewer("Native model picker test"); set_size(viewer, options);
            block_navigation_input(viewer);glfwSetKeyCallback(viewer.window(),nullptr);
            auto* model=viewer.add_model(file); require(model!=nullptr,"model picker load failed");
            viewer.camera()->setType(Camera::ORTHOGRAPHIC); viewer.fit_screen();
            if (viewer.device().supported_sample_counts()&4) viewer.set_samples(4);
            frame(viewer); const auto baseline=viewer.capture();
            click(viewer,vec3(-1.4f,.3f,0));
            require(model->renderer()->is_selected(),"tutorial click did not select model");
            auto selected=viewer.capture(); require(selected.rgba!=baseline.rgba,"model selection was not displayed");
            same_picking(baseline,selected);
            click(viewer,vec3(-1.4f,.3f,0),Viewer::BUTTON_RIGHT);
            require(model->renderer()->is_selected(),"right click toggled model selection");
            click(viewer,vec3(-1.4f,.3f,0));
            require(!model->renderer()->is_selected(),"second tutorial click did not unselect model");
            same_picking(baseline,viewer.capture());
            require(viewer.device().validation_errors()==0,"model picker validation errors");
        }
        {
            PickerHost<TutorialFacePicker> viewer("Native face picker test"); set_size(viewer, options);
            block_navigation_input(viewer);glfwSetKeyCallback(viewer.window(),nullptr);
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
            viewer.mouse_press_event(0,0,Viewer::BUTTON_LEFT,0);
            viewer.mouse_release_event(0,0,Viewer::BUTTON_LEFT,0);frame(viewer);
            require(!faces->highlight(),"background click did not clear highlight");
            require(viewer.device().validation_errors()==0,"face picker validation errors");
        }
        std::cout << "PASS: model/face tutorial clicks, polygon highlights, unchanged MSAA picking/depth, HiDPI/resize, sample changes, background clear, snapshot\n";
    } catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
