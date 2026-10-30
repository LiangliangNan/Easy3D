// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/viewer.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/util/initializer.h>
#include <easy3d/util/resource.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <thread>
using namespace easy3d;
void require(bool ok,const char* text) {if (!ok) throw std::runtime_error(text);}
struct PathViewer : VulkanViewer {
    using VulkanViewer::VulkanViewer;
    KeyFrameInterpolator& path() {return *kfi_;}
    bool visible_path() const {return show_camera_path_;}
    void path_visibility(bool visible) {show_camera_path_=visible;}
    std::vector<graphics::DrawCommand> application;
    void pre_draw() override {set_overlays(application);}
};
void same_picking(const graphics::FrameImage& a,const graphics::FrameImage& b) {
    require(a.depth==b.depth && a.ids.size()==b.ids.size(),"global path annotations changed depth");
    for (size_t i=0;i<a.ids.size();++i)
        require(a.ids[i].object==b.ids[i].object && a.ids[i].primitive==b.ids[i].primitive,"global path annotations changed IDs");
}
int main(int argc,char**) {
    try {
        initialize(false,false,false);
        VulkanViewerOptions options;options.visible=false;options.width=580;options.height=420;
        options.device.validation=options.device.synchronization_validation=argc>1;
        PathViewer v("Shared viewer camera path",options);
        v.set_usage("Global camera path regression","Alt-K: record; Ctrl/Command-K: play; Ctrl/Command-T: show path");
        require(v.add_model(resource::directory()+"/data/sphere.obj"),"sphere missing");
        auto key=glfwSetKeyCallback(v.window(),nullptr);glfwSetKeyCallback(v.window(),key);
        auto press=[&](int code,int mods){key(v.window(),code,0,GLFW_PRESS,mods);};
        const std::vector<vec3> poses={vec3(0,0,100),vec3(60,0,100),vec3(60,60,80)};
        for (const auto& pose:poses) {
            v.camera()->setPosition(pose);v.camera()->lookAt(vec3(0));press(GLFW_KEY_K,GLFW_MOD_ALT);
        }
        require(v.path().number_of_keyframes()==3 && !v.visible_path(),"Alt-K did not record independent path");
        press(GLFW_KEY_K,GLFW_MOD_SHIFT);require(v.path().number_of_keyframes()==3,"modified K recorded keyframe");
        const auto scene_radius=v.camera()->sceneRadius();
        press(GLFW_KEY_T,VulkanViewer::MODIF_CTRL);
        require(v.visible_path() && v.camera()->sceneRadius()>scene_radius,"show path did not include keyframe bounds");
        press(GLFW_KEY_T,VulkanViewer::MODIF_CTRL);
        require(!v.visible_path() && v.camera()->sceneRadius()==scene_radius,"hide path did not restore scene bounds");
        press(GLFW_KEY_T,VulkanViewer::MODIF_CTRL);
        v.camera()->setPosition(vec3(160,120,240));v.camera()->lookAt(vec3(20,20,0));
        v.camera()->setSceneRadius(160);v.camera()->showEntireScene();
        std::vector<graphics::Vertex> vertices(3);
        vertices[0].position=vec3(.7f,.7f,0);vertices[1].position=vec3(.95f,.7f,0);vertices[2].position=vec3(.7f,.95f,0);
        graphics::DrawCommand app;app.geometry=std::make_shared<graphics::Geometry>(vertices);app.color=vec4(1,0,0,1);
        v.application={app};
        for (unsigned count:{1u,4u}) {
            if (!(v.device().supported_sample_counts()&count)) continue;v.set_samples(count);
            require(v.draw_frame(),"global guide frame failed");const auto both=v.capture();
            v.path_visibility(false);const auto plain=v.capture();same_picking(both,plain);
            require(both.rgba!=plain.rgba,"global path/markers not visible");
            v.path_visibility(true);v.application.clear();v.draw_frame();const auto no_app=v.capture();
            same_picking(both,no_app);require(no_app.rgba!=both.rgba,"application overlay was not independent");
            v.path_visibility(false);require(v.capture().rgba!=no_app.rgba,"application pre_draw erased host path layer");
            v.path_visibility(true);v.application={app};v.draw_frame();
            require(v.capture().rgba==both.rgba,"path and application layers did not restore");
        }
        require(v.snapshot("vulkan-viewer-camera-path.png",1,1),"global path snapshot failed");
        unsigned animated=0;v.animation_func_=[&](VulkanViewer*){++animated;return true;};v.set_animation(true);
        const auto observer=v.camera()->position();press(GLFW_KEY_K,VulkanViewer::MODIF_CTRL);
        require(v.path().is_interpolation_started() && v.is_animating(),"global playback replaced application animation");
        v.capture();require(v.camera()->position()==observer && animated==0,"capture advanced path/application animation");
        v.draw_frame();require(v.camera()->position()==poses.front() && animated==1,"first shared path sample/animation failed");
        press(GLFW_KEY_K,VulkanViewer::MODIF_CTRL);const auto paused=v.camera()->position();
        std::this_thread::sleep_for(std::chrono::milliseconds(45));v.draw_frame();
        require(!v.path().is_interpolation_started() && v.camera()->position()==paused && animated==2,"path pause interfered with application animation");
        press(GLFW_KEY_K,VulkanViewer::MODIF_CTRL);v.draw_frame();
        require(v.camera()->position()!=paused && animated==3,"shared path resume failed");
        press(GLFW_KEY_K,GLFW_MOD_ALT);require(!v.path().is_interpolation_started() && v.path().number_of_keyframes()==4,"recording did not stop playback");
        v.bind([](VulkanViewer*,Model*){return true;},nullptr,VulkanViewer::KEY_D,VulkanViewer::MODIF_ALT);
        press(GLFW_KEY_D,GLFW_MOD_ALT);require(v.path().number_of_keyframes()==4,"custom binding did not override default path key");
        v.path().delete_path();v.set_animation(false);v.animation_func_={};
        v.camera()->setPosition(poses.front());press(GLFW_KEY_K,GLFW_MOD_ALT);
        v.clear_scene();require(v.path().number_of_keyframes()==1,"scene clearing discarded viewer camera path");
        v.path_visibility(true);v.draw_frame();
        PathViewer independent("Independent viewer path",options);
        require(independent.path().number_of_keyframes()==0 && !independent.visible_path(),"viewers shared path state");
        auto key2=glfwSetKeyCallback(independent.window(),nullptr);glfwSetKeyCallback(independent.window(),key2);
        key2(independent.window(),GLFW_KEY_D,0,GLFW_PRESS,GLFW_MOD_ALT);
        key2(independent.window(),GLFW_KEY_T,0,GLFW_PRESS,PathViewer::MODIF_CTRL);
        require(independent.path().number_of_keyframes()==0,"empty path visibility created keyframes");
        // Deletion via the default handler in a viewer without a custom binding.
        independent.camera()->setPosition(poses.front());key2(independent.window(),GLFW_KEY_K,0,GLFW_PRESS,GLFW_MOD_ALT);
        key2(independent.window(),GLFW_KEY_K,0,GLFW_PRESS,PathViewer::MODIF_CTRL);
        key2(independent.window(),GLFW_KEY_D,0,GLFW_PRESS,GLFW_MOD_ALT);
        require(!independent.path().is_interpolation_started() && independent.path().number_of_keyframes()==0,"Alt-D did not stop/delete path");
        independent.resize(610,440);VulkanViewer::poll_events();independent.draw_frame();
        require(v.device().validation_errors()==0 && independent.device().validation_errors()==0,"global camera-path validation errors");
        std::cout<<"PASS: global path keys, UI stepping, independent overlays/animation, MSAA/depth/IDs and viewer lifetime\n";
    } catch (const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
