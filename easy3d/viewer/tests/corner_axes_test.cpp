// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/viewer.h>
#include <easy3d/util/initializer.h>
#include <easy3d/util/resource.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
using namespace easy3d;
void require(bool ok,const char* message) {if (!ok) throw std::runtime_error(message);}
struct AxesViewer : VulkanViewer {
    using VulkanViewer::VulkanViewer;
    void hide_logo() {show_easy3d_logo_=false;}
    bool visible() const {return show_corner_axes_;}
};
void same_picking(const graphics::FrameImage& a,const graphics::FrameImage& b) {
    require(a.depth==b.depth && a.ids.size()==b.ids.size(),"corner axes changed scene depth");
    for (size_t i=0;i<a.ids.size();++i)
        require(a.ids[i].object==b.ids[i].object && a.ids[i].primitive==b.ids[i].primitive,"corner axes changed picking IDs");
}
int main(int argc,char**) {
    try {
        initialize(false,false,false);
        VulkanViewerOptions options;options.visible=false;options.width=360;options.height=280;
        options.device.validation=options.device.synchronization_validation=argc>1;
        AxesViewer viewer("Corner axes",options);viewer.set_background_color(vec4(0));viewer.hide_logo();
        auto key=glfwSetKeyCallback(viewer.window(),nullptr);glfwSetKeyCallback(viewer.window(),key);
        auto toggle=[&]{key(viewer.window(),GLFW_KEY_A,0,GLFW_PRESS,0);};
        require(viewer.visible(),"axes not visible by default");
        for (unsigned count:{1u,4u}) {
            if (!(viewer.device().supported_sample_counts()&count)) continue;
            viewer.set_samples(count);const auto shown=viewer.capture();
            int logical_width=0,logical_height=0;glfwGetWindowSize(viewer.window(),&logical_width,&logical_height);
            const unsigned size=unsigned(100.0*viewer.width()/logical_width)+1;
            unsigned pixels=0,partial=0;
            for (unsigned y=0;y<shown.height;++y) for (unsigned x=0;x<shown.width;++x) {
                const auto i=size_t(y)*shown.width+x;
                require(shown.ids[i].object==UINT32_MAX && shown.depth[i]==1,"axes became a scene surface");
                if (!shown.rgba[4*i+3]) continue;
                require(x<size && y+size>=shown.height,"axes escaped DPI-scaled lower-left corner");
                ++pixels;partial+=shown.rgba[4*i+3]<255;
            }
            require(pixels>100,"axes did not render");
            if (count==4) require(partial>20,"axes edges lack MSAA coverage");
            key(viewer.window(),GLFW_KEY_A,0,GLFW_PRESS,GLFW_MOD_SHIFT);
            require(viewer.visible(),"Shift-A toggled axes");
            toggle();require(!viewer.visible(),"A did not hide axes");const auto hidden=viewer.capture();same_picking(shown,hidden);
            for (auto value:hidden.rgba) require(value==0,"hidden axes left stale pixels");
            toggle();require(viewer.capture().rgba==shown.rgba,"axes toggle did not restore rendering");
        }
        const auto before=viewer.capture();
        viewer.camera()->setOrientation(viewer.camera()->orientation()*quat(vec3(0,1,0),.7f));
        const auto rotated=viewer.capture();same_picking(before,rotated);
        require(rotated.rgba!=before.rgba,"axes ignored camera orientation");
        viewer.camera()->setPosition(viewer.camera()->position()+vec3(2,3,4));
        viewer.camera()->setType(Camera::ORTHOGRAPHIC);
        require(viewer.capture().rgba==rotated.rgba,"camera translation/projection changed orientation-only axes");
        require(viewer.add_model(resource::directory()+"/data/sphere.obj"),"scene fixture missing");
        viewer.set_usage("default","Corner axes and usage hint");
        std::vector<graphics::Vertex> vertices(3);
        vertices[0].position=vec3(.7f,.7f,0);vertices[1].position=vec3(.95f,.7f,0);vertices[2].position=vec3(.7f,.95f,0);
        graphics::DrawCommand app;app.geometry=std::make_shared<graphics::Geometry>(vertices);app.color=vec4(1,0,0,1);
        viewer.set_overlays({app});require(viewer.draw_frame(),"axes frame did not present");
        const auto scene=viewer.capture();toggle();const auto without=viewer.capture();same_picking(scene,without);
        require(scene.rgba!=without.rgba,"axes did not coexist with scene/HUD");toggle();
        viewer.set_overlays({});const auto no_app=viewer.capture();same_picking(scene,no_app);
        require(scene.rgba!=no_app.rgba,"axes erased application overlay");
        viewer.set_overlays({app});require(viewer.snapshot("vulkan-corner-axes.png",1.5f,int(viewer.samples()),1),"axes snapshot failed");
        viewer.resize(420,320);VulkanViewer::poll_events();viewer.draw_frame();
        require(viewer.visible(),"resize lost axes visibility");
        const auto resized=viewer.capture();toggle();const auto resized_hidden=viewer.capture();same_picking(resized,resized_hidden);
        int logical_width=0,logical_height=0;glfwGetWindowSize(viewer.window(),&logical_width,&logical_height);
        const unsigned size=unsigned(100.0*viewer.width()/logical_width)+1;
        unsigned changed=0;
        for (unsigned y=0;y<resized.height;++y) for (unsigned x=0;x<resized.width;++x) {
            const auto i=(size_t(y)*resized.width+x)*4;
            bool different=false;for (unsigned c=0;c<4;++c) different|=resized.rgba[i+c]!=resized_hidden.rgba[i+c];
            if (!different) continue;
            require(x<size && y+size>=resized.height,"resized axes used stale placement");++changed;
        }
        require(changed>100,"resized axes did not render");toggle();
        viewer.clear_scene();require(viewer.visible(),"scene clear lost axes visibility");
        AxesViewer independent("Independent corner axes",options);
        toggle();require(!viewer.visible() && independent.visible(),"axes visibility leaked across viewers");
        bool bound=false;
        viewer.bind([&](VulkanViewer*,Model*){bound=true;return true;},nullptr,ViewerInput::KEY_A);
        toggle();require(bound && !viewer.visible(),"custom axes binding did not take precedence");
        require(viewer.device().validation_errors()==0 && independent.device().validation_errors()==0,"axes validation errors");
        std::cout<<"PASS: corner axes, visibility/modifiers, orientation-only transform, DPI, MSAA, overlays, independent viewers and preserved depth/IDs\n";
    } catch(const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
