#include "test_window.h"
// SPDX-License-Identifier: GPL-3.0-or-later
#include <tutorials/Tutorial_206_CameraInterpolation/viewer.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/backends/vulkan/key_frame_interpolator.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/util/initializer.h>
#include <easy3d/util/resource.h>
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
    // Hide the guides without touching the path or the scene bounds, so depth/IDs stay comparable.
    bool show_path = true;
    void draw() const override {
        if (show_path)
            TutorialCameraInterpolation::draw();
        else {
            set_viewer_annotations({});
            Viewer::draw();
        }
    }
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
void tutorial_keys(PathViewer& v) {
    auto key=glfwSetKeyCallback(v.window(),nullptr);glfwSetKeyCallback(v.window(),key);
    require(key!=nullptr,"keyboard callback missing");
    auto press=[&](int code,int mods=0){key(v.window(),code,0,GLFW_PRESS,mods);};
    const std::vector<vec3> poses={vec3(0,0,100),vec3(60,0,100),vec3(60,60,80)};
    for (const auto& pose:poses) {
        v.camera()->set_position(pose);v.camera()->look_at(vec3(0));press(Viewer::KEY_K);
    }
    // K leaves the camera on the last keyframe; back off to see the whole path.
    v.camera()->set_position(vec3(160,120,240));v.camera()->look_at(vec3(20,20,0));
    v.camera()->set_scene_radius(160);v.camera()->show_entire_scene();
    require(v.draw_frame(),"path frame failed");const auto with_path=v.capture();
    // Guides are stable between frames, visible, and do not write depth or IDs.
    v.draw_frame();require(v.capture().rgba==with_path.rgba,"stationary path rendering changed");
    v.show_path=false;require(v.draw_frame(),"hidden-guide frame failed");const auto without_guides=v.capture();
    require(without_guides.rgba!=with_path.rgba,"keyframe path/markers were not visible");
    same_picking(with_path,without_guides);
    v.show_path=true;v.draw_frame();
    // A modified key is not a keyframe command.
    press(Viewer::KEY_K,Viewer::MODIF_SHIFT);v.draw_frame();
    require(v.capture().rgba==with_path.rgba,"modified K recorded a keyframe");
    press(Viewer::KEY_D);require(v.draw_frame(),"delete frame failed");
    require(v.capture().rgba!=with_path.rgba,"D did not delete the camera path");
    // The tutorial owns a timer-driven interpolator; Space starts and stops it.
    for (const auto& pose:poses) {
        v.camera()->set_position(pose);v.camera()->look_at(vec3(0));press(Viewer::KEY_K);
    }
    v.draw_frame();
    const auto before=v.camera()->position();
    press(Viewer::KEY_SPACE);
    std::this_thread::sleep_for(std::chrono::milliseconds(120));
    v.draw_frame();
    require(distance(v.camera()->position(),before)>1e-4f,"Space did not start the tutorial animation");
    press(Viewer::KEY_SPACE);
    const auto paused=v.camera()->position();
    std::this_thread::sleep_for(std::chrono::milliseconds(80));
    v.draw_frame();
    require(v.camera()->position()==paused,"paused tutorial animation moved the camera");
    require(v.snapshot("vulkan-camera-path.png",1,1),"camera path snapshot failed");
}
void renderer_cache(PathViewer& v) {
    KeyFrameInterpolator path(v.camera()->frame());
    require(path.add_keyframe(Frame(vec3(0,0,50),quat()),0) &&
            path.add_keyframe(Frame(vec3(50,0,50),quat()),1),"renderer path fixture failed");
    vulkan::KeyFrameInterpolatorRenderer renderer;
    const auto a=renderer.draw_path(path,*v.camera());const auto b=renderer.draw_path(path,*v.camera());
    require(a.geometry && a.geometry==b.geometry,"stationary path re-created geometry");
    const auto glyph=renderer.draw_cameras(path,*v.camera(),.4f);
    require(glyph.geometry && glyph.geometry==renderer.draw_cameras(path,*v.camera(),.4f).geometry,
            "stationary camera markers re-created geometry");
    v.camera()->set_position(v.camera()->position()+vec3(1,0,0));
    require(renderer.draw_path(path,*v.camera()).geometry!=a.geometry,"path did not follow the camera");
    v.camera()->set_position(v.camera()->position()-vec3(1,0,0));
}
int main(int argc,char**) {
    try {
        initialize(false,false,false);core_playback();
        TestWindow options;options.visible=false;options.width=580;options.height=420;
        options.samples=4;
        options.device.validation=options.device.sync_validation=argc>1;
        PathViewer v("Tutorial camera paths");set_size(v,options);require(v.has_usage(),"tutorial usage hint missing");
        require(v.add_model(resource::directory()+"/data/sphere.obj"),"sphere load failed");
        tutorial_keys(v);
        renderer_cache(v);
        require(v.device().validation_errors()==0,"camera path validation errors");
        std::cout<<"PASS: shared manual interpolation, tutorial timer playback and keys, native paths/cameras, depth/IDs, snapshot\n";
    } catch (const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
