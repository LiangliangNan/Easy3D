// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/vulkan_viewer.h>
#include <easy3d/util/initializer.h>
#include <easy3d/util/resource.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <cmath>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <thread>
using namespace easy3d;
void require(bool ok,const char* text) {if (!ok) throw std::runtime_error(text);}
struct RateViewer : VulkanViewer {
    using VulkanViewer::VulkanViewer;
    bool shown() const {return show_frame_rate_;}
    const std::string& rate() const {return framerate_;}
    std::vector<graphics::DrawCommand> application;
    void pre_draw() override {set_overlays(application);}
};
void same_picking(const graphics::FrameImage& a,const graphics::FrameImage& b) {
    require(a.depth==b.depth && a.ids.size()==b.ids.size(),"frame-rate HUD changed depth");
    for (size_t i=0;i<a.ids.size();++i)
        require(a.ids[i].object==b.ids[i].object && a.ids[i].primitive==b.ids[i].primitive,"frame-rate HUD changed picking IDs");
}
int main(int argc,char**) {
    try {
        initialize(false,false,false);
        VulkanViewerOptions options;options.visible=false;options.width=580;options.height=420;
        options.device.validation=options.device.synchronization_validation=argc>1;
        RateViewer v("Frame rate HUD",options);
        v.set_usage("Frame rate regression","Usage hint remains below the frame rate\nSecond hint line");
        require(v.add_model(resource::directory()+"/data/sphere.obj"),"scene missing");
        std::vector<graphics::Vertex> vertices(3);
        vertices[0].position=vec3(.7f,.7f,0);vertices[1].position=vec3(.95f,.7f,0);vertices[2].position=vec3(.7f,.95f,0);
        graphics::DrawCommand app;app.geometry=std::make_shared<graphics::Geometry>(vertices);app.color=vec4(1,0,0,1);
        v.application={app};VulkanViewer::poll_events();v.draw_frame();v.draw_frame();
        require(!v.shown(),"frame rate is not disabled by default");
        auto key=glfwSetKeyCallback(v.window(),nullptr);glfwSetKeyCallback(v.window(),key);
        auto toggle=[&]{key(v.window(),GLFW_KEY_F,0,GLFW_PRESS,VulkanViewer::MODIF_CTRL);};
        for (unsigned count:{1u,4u}) {
            if (!(v.device().supported_sample_counts()&count)) continue;v.set_samples(count);v.draw_frame();
            const auto baseline=v.capture();const auto camera=v.camera()->modelViewProjectionMatrix();
            key(v.window(),GLFW_KEY_F,0,GLFW_PRESS,GLFW_MOD_SHIFT);
            require(!v.shown(),"Shift-F toggled frame rate");
            toggle();require(v.shown() && v.rate().find("--")!=std::string::npos,"Ctrl/Command-F did not start fresh sampling");
            const auto shown=v.capture();same_picking(baseline,shown);
            require(shown.rgba!=baseline.rgba,"frame rate/hint reflow did not render");
            const auto after=v.camera()->modelViewProjectionMatrix();
            for (unsigned i=0;i<16;++i) require(camera[i]==after[i],"modified F fitted the scene");
            const auto placeholder=v.rate();std::this_thread::sleep_for(std::chrono::milliseconds(1030));
            require(v.capture().rgba==shown.rgba && v.rate()==placeholder,"capture advanced frame-rate sampling");
            require(v.draw_frame(),"rate frame did not present");
            require(v.rate()!=placeholder && v.rate().find("--")==std::string::npos,"presented frame did not update measured rate");
            std::istringstream in(v.rate());std::string label;double fps=0,milliseconds=0;char parenthesis=0;
            in>>label>>fps>>parenthesis>>milliseconds;
            require(in && label=="fps:" && parenthesis=='(' && std::isfinite(fps) && fps>=0 &&
                    std::isfinite(milliseconds) && milliseconds>0,"invalid fps/ms-per-frame display");
            same_picking(baseline,v.capture());
            const auto measured=v.capture();v.application.clear();v.draw_frame();
            require(v.capture().rgba!=measured.rgba,"HUD replaced application overlay");
            v.application={app};v.draw_frame();
            require(v.snapshot("vulkan-frame-rate.png",1,1),"frame rate snapshot failed");
            toggle();require(!v.shown() && v.capture().rgba==baseline.rgba,"hiding rate did not restore original hint layout");
        }
        RateViewer independent("Independent rate",options);independent.set_usage("","");
        auto key2=glfwSetKeyCallback(independent.window(),nullptr);glfwSetKeyCallback(independent.window(),key2);
        const auto plain=independent.capture();key2(independent.window(),GLFW_KEY_F,0,GLFW_PRESS,RateViewer::MODIF_CTRL);
        require(independent.shown() && independent.rate().find("--")!=std::string::npos,"viewers shared frame-rate state");
        require(independent.capture().rgba!=plain.rgba,"rate without usage hint did not load font/render");
        independent.resize(610,440);VulkanViewer::poll_events();independent.draw_frame();
        require(independent.shown(),"resize reset rate visibility");
        independent.clear_scene();require(independent.capture().rgba!=std::vector<unsigned char>(independent.width()*independent.height()*4,255),
                                          "scene clearing erased persistent rate HUD");
        bool bound=false;v.bind([&](VulkanViewer*,Model*){bound=true;return true;},nullptr,RateViewer::KEY_F,RateViewer::MODIF_CTRL);
        toggle();require(bound && !v.shown(),"custom binding did not override rate shortcut");
        require(v.device().validation_errors()==0 && independent.device().validation_errors()==0,"frame-rate validation errors");
        std::cout<<"PASS: frame-rate shortcut, per-viewer sampling, capture purity, HUD layering/layout, MSAA and depth/IDs\n";
    } catch (const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
