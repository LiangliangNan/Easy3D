// SPDX-License-Identifier: GPL-3.0-or-later
#include <tests/test.h>
#include <tutorials/Tutorial_205_RealCamera/viewer.h>
#include <tutorials/Tutorial_404_ObjectManipulation/viewer.h>
#include <tutorials/Tutorial_506_DepthMap/viewer.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/manipulated_frame.h>
#include <easy3d/renderer/shape.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/util/resource.h>
#include <cmath>
using namespace easy3d;
using test::require;
struct Pixels {
    std::vector<unsigned char> rgba;int w=0,h=0,c=0;
    explicit Pixels(const std::string& file) {require(ImageIO::load(file,rgba,w,h,c,4,false),"cannot decode tutorial snapshot");}
    const unsigned char* at(int x,int y) const {return rgba.data()+4*(size_t(y)*w+x);}
};
struct DepthView:TutorialDepthMap {using TutorialDepthMap::TutorialDepthMap;};
struct RealView:TutorialRealCamera {
    using TutorialRealCamera::TutorialRealCamera;
    using TutorialRealCamera::key_press_event;
    // OS hover must not disturb the pixel baselines; the test enables it for its
    // synthetic hover call.
    bool hover_enabled=false;
    bool mouse_free_move_event(int x,int y,int dx,int dy,int modifiers) override {
        return hover_enabled ? TutorialRealCamera::mouse_free_move_event(x,y,dx,dy,modifiers) : true;
    }
    Rect image_rect() const {return calculate_image_rect();}
};
struct ManipulationView:TutorialObjectManipulation {
    using TutorialObjectManipulation::TutorialObjectManipulation;
    using TutorialObjectManipulation::mouse_press_event;
    using TutorialObjectManipulation::mouse_release_event;
};
void frame(Viewer& v) {require(v.run_frames(2)==0,"tutorial frame failed");}
void front(Viewer& v) {
    v.camera()->setViewDirection(vec3(0,0,-1));v.camera()->setUpVector(vec3(0,1,0));
    v.camera()->setType(Camera::ORTHOGRAPHIC);v.camera()->setSceneBoundingBox(vec3(-1),vec3(1));v.camera()->showEntireScene();
}
std::vector<vec3> plane(float z) {return {vec3(-.8f,-.8f,z),vec3(.8f,-.8f,z),vec3(.8f,.8f,z),vec3(-.8f,-.8f,z),vec3(.8f,.8f,z),vec3(-.8f,.8f,z)};}
void verify_depth(DepthView& v,float depth,float scale,int samples) {
    require(v.snapshot("depth-preview.png",scale,samples,1,true),"depth preview snapshot failed");Pixels image("depth-preview.png");
    const float sx=float(image.w)/v.framebuffer_width(),sy=float(image.h)/v.framebuffer_height();
    const int x=int((20*v.dpi_scaling()+v.framebuffer_width()/8.0f)*sx),y=int((40*v.dpi_scaling()+v.framebuffer_height()/8.0f)*sy);
    const auto p=image.at(x,y);
    require(std::abs(p[0]-255*depth)<3 && p[0]==p[1] && p[1]==p[2] && p[3]==255,"GPU depth preview differs from the scene query");
}
void depth_preview() {
    DepthView v("Shared depth preview");v.resize(320,280);frame(v);front(v);
    auto faces=std::make_shared<TrianglesDrawable>("plane");faces->update_vertex_buffer(plane(0));faces->set_lighting(false);v.add_drawable(faces);frame(v);
    auto hit=v.pick(v.width()/2,v.height()/2);require(bool(hit),"depth fixture missed");
    const auto camera=v.camera()->modelViewProjectionMatrix();
    for(float scale:{1.0f,1.5f,2.0f}) for(int samples:{1,4}) verify_depth(v,hit.depth,scale,samples);
    for(unsigned i=0;i<16;++i) require(camera[i]==v.camera()->modelViewProjectionMatrix()[i],"depth preview changed camera");
    const auto again=v.pick(v.width()/2,v.height()/2);
    require(again.drawable==faces && std::abs(again.depth-hit.depth)<1e-6f,"depth preview changed picking");
    faces->update_vertex_buffer(plane(.4f));frame(v);hit=v.pick(v.width()/2,v.height()/2);
    require(hit && std::abs(hit.position.z-.4f)<1e-3f,"updated geometry was not picked");verify_depth(v,hit.depth,1.5f,4);
    faces->set_visible(false);frame(v);verify_depth(v,1,1.5f,4);
    auto sphere=std::make_shared<PointsDrawable>("sphere");sphere->update_vertex_buffer({vec3(0)});sphere->set_point_size(50);sphere->set_impostor_type(PointsDrawable::SPHERE);v.add_drawable(sphere);frame(v);
    hit=v.pick(v.width()/2,v.height()/2);require(hit && hit.position.z>0,"sphere depth fixture missed");
    verify_depth(v,hit.depth,1,4);
    v.resize(360,300);frame(v);hit=v.pick(v.width()/2,v.height()/2);require(hit && hit.position.z>0,"resized sphere depth fixture missed");
    verify_depth(v,hit.depth,1.5f,4);
    v.clear_scene();frame(v);verify_depth(v,1,1,1);require(v.validation_errors()==0,"depth preview validation errors");
}
void multi_depth_preview() {
    MultiViewer v(1,2,"Multi-view GPU depth",320,280,4,false,false,true);v.set_division_visible(false);
    auto faces=std::make_shared<TrianglesDrawable>("left cell");faces->update_vertex_buffer(plane(0));faces->set_lighting(false);
    v.add_drawable(faces);v.assign(0,0,faces.get());front(v);frame(v);
    bool found=false;const auto point=v.point_under_pixel(v.width()/4,v.height()/2,found);require(found,"multi-view depth fixture missed");
    const float depth=v.world_to_window(point).z;
    const float left=20*v.dpi_scaling(),top=40*v.dpi_scaling(),width=v.framebuffer_width()/4.0f,height=v.framebuffer_height()/4.0f;
    v.set_overlays({graphics::shape::draw_depth_texture(Rect(left,left+width,top,top+height),v.framebuffer_width(),v.framebuffer_height())});frame(v);
    require(v.snapshot("multi-depth-preview.png",1.5f,4,1,true),"multi-view depth snapshot failed");Pixels image("multi-depth-preview.png");
    const float sx=float(image.w)/v.framebuffer_width(),sy=float(image.h)/v.framebuffer_height();
    const auto foreground=image.at(int((left+width*.25f)*sx),int((top+height*.5f)*sy));
    const auto background=image.at(int((left+width*.75f)*sx),int((top+height*.5f)*sy));
    require(std::abs(foreground[0]-depth*255)<3 && foreground[0]==foreground[1] && background[0]>253 &&
            background[0]==background[1],"multi-view preview ignored cell assignments");
}
void real_camera() {
    const auto root=resource::directory()+"/data/fountain/";
    RealView v("Shared calibrated camera",root+"bundle.out",root+"pointcloud.ply");
    frame(v);
    const auto rect=v.image_rect();require(rect.width()>0 && rect.height()>0,"calibrated image missing");
    v.hover_enabled=true;
    v.mouse_free_move_event(int((rect.x_min()+rect.width()*.5)*v.camera()->screenWidth()/v.framebuffer_width()),
                           int((rect.y_min()+rect.height()*.5)*v.camera()->screenHeight()/v.framebuffer_height()),0,0,0);
    require(v.draw_frame(),"synthetic hover frame failed");bool ray=false;for(const auto& d:v.drawables()) if(d->name()=="ray" && d->is_visible()) ray=true;
    require(ray,"calibrated image hover missed the ray at native DPI");
    require(v.snapshot("calibrated-camera.png",1.5f,4,1,true),"calibrated camera snapshot failed");
    v.key_press_event(Viewer::KEY_SPACE,0);frame(v);require(v.image_rect().width()>0,"switching calibrated views lost image");
    require(v.validation_errors()==0,"calibrated camera validation errors");
}
void manipulator() {
    ManipulationView v("Shared manipulator guide");auto mesh=std::make_shared<SurfaceMesh>();
    auto a=mesh->add_vertex(vec3(-.8f,-.8f,0)),b=mesh->add_vertex(vec3(.8f,-.8f,0)),c=mesh->add_vertex(vec3(.8f,.8f,0)),d=mesh->add_vertex(vec3(-.8f,.8f,0));mesh->add_quad(a,b,c,d);v.add_model(mesh);front(v);
    const auto p=v.camera()->projectedCoordinatesOf(vec3(0));v.mouse_press_event(int(p.x),int(p.y),Viewer::BUTTON_LEFT,0);v.mouse_release_event(int(p.x),int(p.y),Viewer::BUTTON_LEFT,0);frame(v);
    require(bool(mesh->manipulator()),"model selection did not create manipulator");
    auto guide=mesh->manipulator()->draw_frame(v.camera(),v.framebuffer_width(),v.framebuffer_height());
    require(guide.geometry && guide.overlay_depth_test,"manipulator did not return a depth-tested guide");
    require(guide.geometry==mesh->manipulator()->draw_frame(v.camera(),v.framebuffer_width(),v.framebuffer_height()).geometry,"unchanged guide rebuilt geometry");
    mesh->manipulator()->frame()->translate(vec3(.1f,0,0));frame(v);
    require(guide.geometry!=mesh->manipulator()->draw_frame(v.camera(),v.framebuffer_width(),v.framebuffer_height()).geometry,"manipulated guide retained stale geometry");
    require(v.snapshot("shared-manipulator.png",1.5f,4,1,true),"shared manipulator snapshot failed");
    mat4 projection=mat4::identity();projection(3,2)=1;projection(3,3)=.2f;
    const auto clipped=graphics::shape::draw_box_wire(projection,320,240,1,true);
    for(const auto& vertex:clipped.geometry->vertices()) for(unsigned i=0;i<3;++i) require(std::isfinite(vertex.position[i]),"eye-crossing guide produced non-finite geometry");
    require(graphics::shape::draw_box_wire(mat4::translation(vec3(10,0,0)),320,240).geometry->vertices().empty(),"offscreen guide survived clipping");
    require(v.validation_errors()==0,"manipulator guide validation errors");
}
int main() {return test::run("shared calibrated camera, manipulator and GPU depth preview",[]{depth_preview();multi_depth_preview();real_camera();manipulator();});}
