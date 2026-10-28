// SPDX-License-Identifier: GPL-3.0-or-later
#include <tutorials/Tutorial_206_CameraInterpolation/viewer.h>
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
void same_picking(const graphics::FrameImage& a,const graphics::FrameImage& b) {
    require(a.depth==b.depth && a.ids.size()==b.ids.size(),"path guides changed depth");
    for (size_t i=0;i<a.ids.size();++i)
        require(a.ids[i].object==b.ids[i].object && a.ids[i].primitive==b.ids[i].primitive,"path guides changed IDs");
}
struct PathViewer : TutorialCameraInterpolation {
    using TutorialCameraInterpolation::TutorialCameraInterpolation;
    KeyFrameInterpolator& path() {return *interpolator_;}
    bool has_usage() const {return hint_.find("Press 'k'")!=std::string::npos;}
};
void core_playback() {
    Frame target;KeyFrameInterpolator path(&target);
    unsigned samples=0,stopped=0;const auto caller=std::this_thread::get_id();
    path.frame_interpolated.connect([&] {require(std::this_thread::get_id()==caller,"manual interpolation used worker thread");++samples;});
    path.interpolation_stopped.connect([&] {++stopped;});
    path.start_manual_interpolation();require(!path.is_interpolation_started() && !path.advance_interpolation(),"empty path started");
    require(path.add_keyframe(Frame(vec3(0,0,5),quat()),0),"first key failed");
    require(path.add_keyframe(Frame(vec3(2,0,5),quat()),.002f),"second key failed");
    require(path.interpolate().size()==2,"short path did not retain two finite endpoints");
    path.start_manual_interpolation();std::this_thread::sleep_for(std::chrono::milliseconds(40));
    require(target.position()==vec3(0),"manual playback advanced without an explicit step");
    require(path.advance_interpolation() && target.position()==vec3(0,0,5),"first sample failed");
    path.stop_interpolation();require(!path.advance_interpolation(),"stopped path advanced");
    path.start_manual_interpolation();require(path.advance_interpolation() && target.position()==vec3(2,0,5),"resume skipped next sample");
    require(!path.is_interpolation_started() && stopped==1 && samples==2,"completion state/signals failed");
    path.start_manual_interpolation();require(path.advance_interpolation() && target.position()==vec3(0,0,5),"completed path did not restart at beginning");
    path.delete_path();require(!path.is_interpolation_started() && !path.advance_interpolation(),"deletion left playback active");
    path.add_keyframe(Frame(vec3(1,2,3),quat()));path.start_manual_interpolation();
    require(path.advance_interpolation() && target.position()==vec3(1,2,3) && !path.is_interpolation_started(),"single-key path failed");
    path.add_keyframe(Frame(vec3(2,2,3),quat()));
    bool invalid=false;try {path.set_frame_rate(0);} catch (const std::invalid_argument&) {invalid=true;}
    require(invalid && !path.is_interpolation_started(),"invalid frame rate accepted");
    path.set_frame_rate(30);invalid=false;
    try {path.set_interpolation_speed(0);} catch (const std::invalid_argument&) {invalid=true;}
    require(invalid,"zero interpolation speed accepted");
    path.start_manual_interpolation();path.set_frame(nullptr);
    require(!path.advance_interpolation() && !path.is_interpolation_started(),"detached target left playback active");
}
int main(int argc,char**) {
    try {
        initialize(false,false,false);core_playback();
        VulkanViewerOptions options;options.visible=false;options.width=580;options.height=420;
        options.device.validation=options.device.synchronization_validation=argc>1;
        PathViewer v("Native camera paths",options);require(v.has_usage(),"counterpart usage hint missing");
        require(v.add_model(resource::directory()+"/data/sphere.obj"),"sphere load failed");
        auto key=glfwSetKeyCallback(v.window(),nullptr);glfwSetKeyCallback(v.window(),key);
        auto press=[&](int code,int mods=0){key(v.window(),code,0,GLFW_PRESS,mods);};
        const std::vector<vec3> keys={vec3(0,0,100),vec3(60,0,100),vec3(60,60,80)};
        for (const auto& p:keys) {v.camera()->setPosition(p);v.camera()->lookAt(vec3(0));press(GLFW_KEY_K);}
        require(v.path().number_of_keyframes()==3,"K did not record camera keyframes");
        press(GLFW_KEY_K,GLFW_MOD_SHIFT);require(v.path().number_of_keyframes()==3,"modified K recorded a keyframe");
        v.camera()->setPosition(vec3(160,120,240));v.camera()->lookAt(vec3(20,20,0));v.camera()->setSceneRadius(160);v.camera()->showEntireScene();
        for (unsigned count:{1u,4u}) {
            if (!(v.device().supported_sample_counts()&count)) continue;v.set_samples(count);
            require(v.draw_frame(),"camera-guide frame failed");const auto guides=v.capture();
            v.set_overlays({});const auto plain=v.capture();same_picking(guides,plain);
            require(guides.rgba!=plain.rgba,"camera markers/path were not visible");
            v.draw_frame();require(v.capture().rgba==guides.rgba,"stationary guide rendering changed");
        }
        vulkan::KeyFrameInterpolatorRenderer renderer;
        const auto a=renderer.draw_path(v.path(),*v.camera());const auto b=renderer.draw_path(v.path(),*v.camera());
        require(a.geometry==b.geometry,"stationary path re-created geometry");
        const auto glyph=renderer.draw_cameras(v.path(),*v.camera(),.4f);
        require(glyph.geometry==renderer.draw_cameras(v.path(),*v.camera(),.4f).geometry,"stationary camera markers re-created geometry");
        require(v.snapshot("vulkan-camera-path.png",1,1),"camera path snapshot failed");
        press(GLFW_KEY_SPACE);require(v.path().is_interpolation_started(),"Space did not start path");
        const auto observer=v.camera()->position();v.capture();require(v.camera()->position()==observer,"capture advanced path");
        require(v.draw_frame() && v.camera()->position()==keys.front(),"UI frame did not apply first path sample");
        press(GLFW_KEY_SPACE);require(!v.path().is_interpolation_started(),"Space did not pause path");
        const auto paused=v.camera()->position();std::this_thread::sleep_for(std::chrono::milliseconds(45));v.draw_frame();
        require(v.camera()->position()==paused,"paused path changed camera");
        press(GLFW_KEY_SPACE);v.draw_frame();require(v.camera()->position()!=paused,"resume did not apply next sample");
        press(GLFW_KEY_K);require(!v.path().is_interpolation_started() && v.path().number_of_keyframes()==4,"path edit did not stop playback");
        press(GLFW_KEY_D);require(v.path().number_of_keyframes()==0 && !v.path().is_interpolation_started(),"D did not delete path");
        v.draw_frame();const auto cleared=v.capture();v.set_overlays({});require(v.capture().rgba==cleared.rgba,"deleted path left stale markers");
        press(GLFW_KEY_SPACE);require(!v.path().is_interpolation_started(),"empty tutorial path started");
        v.resize(610,440);VulkanViewer::poll_events();v.draw_frame();
        require(v.device().validation_errors()==0,"camera path validation errors");
        std::cout<<"PASS: shared manual interpolation, resume/endpoints, native paths/cameras, depth/IDs/MSAA, controls and lifetime\n";
    } catch (const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
