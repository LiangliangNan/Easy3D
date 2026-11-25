#include "test_window.h"
// SPDX-License-Identifier: GPL-3.0-or-later
#include <tutorials/Tutorial_206_CameraInterpolation/viewer.h>
#include <easy3d/renderer/keyframe_interpolator.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/util/initializer.h>
#include <easy3d/util/resource.h>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>
using namespace easy3d;
void require(bool ok,const char* text) {if (!ok) throw std::runtime_error(text);}
struct PathViewer : TutorialCameraInterpolation {
    using TutorialCameraInterpolation::TutorialCameraInterpolation;
    // Hides the tutorial's guide drawables without touching the path or the scene bounds;
    // the tutorial's own pre_draw() would show them again.
    bool show_path = true;
    void pre_draw() override {
        if (show_path) {
            TutorialCameraInterpolation::pre_draw();
            return;
        }
        Viewer::pre_draw();
        for (const auto& drawable : drawables())
            drawable->set_visible(false);
    }
    bool has_usage() const {return hint_.find("Press 'k'")!=std::string::npos;}
};
void core_playback() {
    Frame target;KeyframeInterpolator path(&target);
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
    // Both guides are scene drawables: they write depth and object ids, so picking and the
    // keyframe drag can land on the glyphs.
    require(with_path.depth!=without_guides.depth,"the guides did not write depth");
    bool guide_ids=false;
    for (std::size_t i=0;i<with_path.ids.size();++i)
        guide_ids|=with_path.ids[i].object!=without_guides.ids[i].object;
    require(guide_ids,"the guides are not in the id buffer");
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
void scene_guides(PathViewer& v) {
    // The path and the keyframe cameras are ordinary scene drawables: the callers own them,
    // the capture includes them, they write depth, and removing them clears the scene. This
    // viewer has no tutorial keyframes yet, so the counts below are the guides' own.
    // Derive the fixture from the fitted view so it stays framed and unoccluded: two
    // keyframes in the upper half of the frustum at the scene-center depth, clear of the
    // model at the center.
    const Camera *cam = v.camera();
    const vec3 view = cam->view_direction();
    const vec3 up = cam->up_vector();
    const float depth = distance(cam->position(), cam->scene_center());
    const float half_height = depth * std::tan(0.5f * cam->field_of_view());
    const vec3 center = cam->position() + view * depth + 0.5f * half_height * up;
    const vec3 first = center - 0.25f * half_height * up;
    const vec3 second = center + 0.25f * half_height * up;
    const float glyph_width = 0.2f * half_height;
    KeyframeInterpolator path(v.camera()->frame());
    require(path.add_keyframe(Frame(first,quat()),0),"scene guide fixture failed");
    auto line=std::make_shared<LinesDrawable>("scene guide path");
    require(!path.update_path_drawable(*line),"one keyframe produced a path");
    require(path.add_keyframe(Frame(second,quat()),1),"second keyframe failed");
    require(path.update_path_drawable(*line),"scene path fill failed");
    auto glyphs=std::make_shared<TrianglesDrawable>("scene guide glyphs");
    require(!path.update_cameras_drawable(*glyphs,v.camera(),0),"a non-positive glyph width was accepted");
    require(path.update_cameras_drawable(*glyphs,v.camera(),glyph_width),"scene glyph fill failed");
    v.draw_frame();const auto bare=v.capture();
    v.add_drawable(glyphs);v.draw_frame();const auto with_glyphs=v.capture();
    require(with_glyphs.rgba!=bare.rgba,"scene glyphs were not visible");
    require(with_glyphs.depth!=bare.depth,"scene glyphs did not write depth");
    // The glyphs are 20 units wide and sit on the path; capture the path alone.
    v.delete_drawable(glyphs.get());v.draw_frame();
    v.add_drawable(line);v.draw_frame();const auto with_line=v.capture();
    require(with_line.rgba!=bare.rgba,"scene path was not visible");
    require(with_line.depth!=bare.depth,"scene path did not write depth");
    v.delete_drawable(line.get());
    v.draw_frame();const auto cleared=v.capture();
    require(cleared.rgba==bare.rgba && cleared.depth==bare.depth,"removed scene guides still rendered");
}
int main(int argc,char**) {
    try {
        initialize(false,false,false);core_playback();
        TestWindow options;options.visible=false;options.width=580;options.height=420;
        options.samples=4;
        options.device.validation=options.device.sync_validation=argc>1;
        PathViewer v("Tutorial camera paths");set_size(v,options);require(v.has_usage(),"tutorial usage hint missing");
        require(v.add_model(resource::directory()+"/data/sphere.obj"),"sphere load failed");
        scene_guides(v);
        tutorial_keys(v);
        require(v.device().validation_errors()==0,"camera path validation errors");
        std::cout<<"PASS: shared manual interpolation, scene-drawable guides, tutorial timer playback and keys, snapshot\n";
    } catch (const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
