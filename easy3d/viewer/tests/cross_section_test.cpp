// SPDX-License-Identifier: GPL-3.0-or-later
#include <tutorials/Tutorial_307_CrossSection/viewer.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <easy3d/util/initializer.h>
#include <easy3d/util/setting.h>
#include <easy3d/renderer/clipping_plane.h>
#include <fstream>
#include <stdexcept>
#include <cmath>
#include <algorithm>
#include <iostream>
using namespace easy3d;
void require(bool value,const char* message) { if (!value) throw std::runtime_error(message); }
void frame(CrossSection& viewer) {
    for (unsigned i=0;i<100;++i) {
        VulkanViewer::poll_events(); if (viewer.draw_frame()) return;
        glfwWaitEventsTimeout(.01);
    }
    throw std::runtime_error("cross-section viewer did not present");
}
unsigned hits(const graphics::FrameImage& image) { unsigned count=0; for (auto id:image.ids) count+=id.object!=UINT32_MAX; return count; }
void same_picking(const graphics::FrameImage& a,const graphics::FrameImage& b) {
    require(a.depth==b.depth && a.ids.size()==b.ids.size(),"guide/MSAA changed picking depth");
    for (size_t i=0;i<a.ids.size();++i)
        require(a.ids[i].object==b.ids[i].object && a.ids[i].primitive==b.ids[i].primitive,"guide/MSAA changed source IDs");
}
void check_planes(CrossSection& viewer,const graphics::FrameImage& image) {
    const auto inverse_mvp=inverse(viewer.camera()->modelViewProjectionMatrix());
    bool checked_pick=false;
    for (unsigned y=0;y<image.height;++y) for (unsigned x=0;x<image.width;++x) {
        const auto i=size_t(y)*image.width+x; if (image.ids[i].object==UINT32_MAX) continue;
        const auto world=inverse_mvp*vec4(2*(x+.5f)/image.width-1,1-2*(y+.5f)/image.height,2*image.depth[i]-1,1);
        const vec3 point(world.x/world.w,world.y/world.w,world.z/world.w);
        if (!checked_pick) {
            const auto hit=viewer.pick(x,y);
            require(hit && (hit.position-point).norm()<1e-4f,"picked world point is offset from framebuffer pixel center");
            checked_pick=true;
        }
        for (const auto& plane:viewer.clipping_planes()) require(dot(plane,vec4(point,1))>=-1e-3f,"cross-section depth lies outside world clip volume");
    }
}
struct CrossSectionHost : CrossSection {
    using CrossSection::CrossSection;
    void hide_logo() {show_easy3d_logo_=false;}
};
int main(int argc,char**) {
    try {
        initialize(false,false,false);
        setting::clipping_plane_color=vec4(1,0,0,.25f);
        const std::string file="vulkan-cross-section.off";
        { std::ofstream out(file); out << "OFF\n8 6 0\n-1 -1 -1\n1 -1 -1\n1 1 -1\n-1 1 -1\n"
            "-1 -1 1\n1 -1 1\n1 1 1\n-1 1 1\n4 0 3 2 1\n4 4 5 6 7\n4 0 1 5 4\n4 1 2 6 5\n4 2 3 7 6\n4 3 0 4 7\n";
          require(bool(out),"could not write cross-section fixture"); }
        VulkanViewerOptions options; options.width=360; options.height=280;
        options.device.validation=options.device.synchronization_validation=argc>1;
        CrossSectionHost viewer("Native cross-section test",options);viewer.hide_logo();
        viewer.set_background_color(vec4(1)); // Analytic plane-alpha oracle uses white.
        auto* plane=ClippingPlane::instance();
        require(plane->is_enabled(),"tutorial did not enable shared clipping plane");
        plane->set_cross_section_width(.01f); // keep the thin slab visible in this small fixture
        viewer.camera()->setViewDirection(vec3(-1,1,-1));
        require(viewer.add_model(file)!=nullptr,"cross-section fixture load failed");
        if (viewer.device().supported_sample_counts()&4) viewer.set_samples(4);
        frame(viewer);
        // Look from the retained side: opaque cube surfaces must hide the guide.
        const Camera saved(*viewer.camera());
        viewer.camera()->setViewDirection(vec3(-1,1,1)); viewer.camera()->showEntireScene(); frame(viewer);
        const auto behind=viewer.capture(); plane->set_visible(false); const auto reference=viewer.capture();
        plane->set_visible(true);
        same_picking(behind,reference);
        const auto probe_clip=viewer.camera()->modelViewProjectionMatrix()*vec4(.4f,-.4f,-1,1);
        const int bx=int((probe_clip.x/probe_clip.w+1)*.5f*behind.width),by=int((1-probe_clip.y/probe_clip.w)*.5f*behind.height);
        require(bx>=0 && by>=0 && bx<int(behind.width) && by<int(behind.height),"occlusion probe outside viewport");
        const auto bi=size_t(by)*behind.width+bx;
        require(reference.ids[bi].object!=UINT32_MAX,"occlusion probe missed the cube");
        const auto a=viewer.unproject_pixel(bx,by,0),b=viewer.unproject_pixel(bx,by,1);
        const auto plane_point=a+(b-a)*(-a.z/(b.z-a.z));
        require(std::abs(plane_point.x)<viewer.camera()->sceneRadius()*.95f &&
                std::abs(plane_point.y)<viewer.camera()->sceneRadius()*.95f,"occlusion probe missed guide interior");
        for (unsigned c=0;c<4;++c) require(behind.rgba[4*bi+c]==reference.rgba[4*bi+c],"filled guide showed through nearer opaque geometry");
        *viewer.camera()=saved; frame(viewer);
        const auto initial=viewer.capture();
        require(viewer.clipping_planes().size()==1 && hits(initial)>100,"cross-section did not enable a clipping plane"); check_planes(viewer,initial);
        require((plane->center()-viewer.camera()->sceneCenter()).norm()<1e-6f &&
                std::abs(plane->scene_radius()-viewer.camera()->sceneRadius())<1e-6f,"shared plane was not fitted to tutorial scene");
        // Changes through the public controller must be visible without a
        // tutorial pre_draw() copying state into private planes/geometry.
        const auto color=plane->color();plane->set_color(vec4(0,.5f,1,.4f));
        const auto recolored=viewer.capture();same_picking(initial,recolored);
        require(recolored.rgba!=initial.rgba,"tutorial ignored shared plane color");
        plane->set_color(color);require(viewer.capture().rgba==initial.rgba,"restoring shared plane color changed scene");
        std::vector<graphics::Vertex> marker(3);
        marker[0].position=vec3(.7f,.7f,0);marker[1].position=vec3(.95f,.7f,0);marker[2].position=vec3(.7f,.95f,0);
        graphics::DrawCommand app;app.geometry=std::make_shared<graphics::Geometry>(marker);app.color=vec4(0,1,0,1);
        viewer.set_overlays({app});frame(viewer);const auto annotated=viewer.capture();same_picking(initial,annotated);
        require(annotated.rgba!=initial.rgba,"tutorial erased application overlay");
        frame(viewer);require(viewer.capture().rgba==annotated.rgba,"tutorial frame replaced application overlay");
        viewer.set_overlays({});require(viewer.capture().rgba==initial.rgba,"clearing application overlay erased shared guide");
        auto key=glfwSetKeyCallback(viewer.window(),nullptr); glfwSetKeyCallback(viewer.window(),key);
        auto press=[&](int code) { key(viewer.window(),code,0,GLFW_PRESS,GLFW_MOD_ALT); frame(viewer); };
        press(GLFW_KEY_V); const auto hidden=viewer.capture();
        require(hidden.rgba!=initial.rgba,"guide visibility shortcut did not change overlay"); same_picking(initial,hidden);
        // Probe the plane interior away from the outlines and from the cube.
        const auto clip=viewer.camera()->modelViewProjectionMatrix()*vec4(-1.2f,1.2f,0,1);
        const int x=int((clip.x/clip.w+1)*.5f*initial.width),y=int((1-clip.y/clip.w)*.5f*initial.height);
        require(x>=0 && y>=0 && x<int(initial.width) && y<int(initial.height),"plane interior probe is outside viewport");
        const auto pixel=size_t(y)*initial.width+x;
        require(hidden.ids[pixel].object==UINT32_MAX && initial.rgba[4*pixel]>250 &&
                std::abs(int(initial.rgba[4*pixel+1])-191)<=1 && std::abs(int(initial.rgba[4*pixel+2])-191)<=1,
                "native clipping plane interior was not filled transparently");
        require(!viewer.pick(x,y),"filled clipping guide became a picking surface");
        press(GLFW_KEY_V); require(viewer.capture().rgba==initial.rgba,"showing guide did not restore image");
        press(GLFW_KEY_C); const auto full=viewer.capture();
        require(viewer.clipping_planes().empty() && hits(full)>hits(initial),"disabling clipping did not restore full model");
        press(GLFW_KEY_C); require(viewer.capture().rgba==initial.rgba,"clipping toggle did not restore cut geometry");
        press(GLFW_KEY_X); const auto slab=viewer.capture();
        require(viewer.clipping_planes().size()==2 && hits(slab)>0 && hits(slab)<hits(initial),"cross-section did not retain a thin slab"); check_planes(viewer,slab);
        auto mouse=glfwSetMouseButtonCallback(viewer.window(),nullptr); glfwSetMouseButtonCallback(viewer.window(),mouse);
        auto cursor=glfwSetCursorPosCallback(viewer.window(),nullptr); glfwSetCursorPosCallback(viewer.window(),cursor);
        auto drag=[&](int button,double dx,double dy) {
            int w,h; glfwGetWindowSize(viewer.window(),&w,&h); const double x=w*.5,y=h*.5;
            glfwSetCursorPos(viewer.window(),x,y); mouse(viewer.window(),button,GLFW_PRESS,0);
            cursor(viewer.window(),x+dx,y+dy); glfwSetCursorPos(viewer.window(),x+dx,y+dy);
            mouse(viewer.window(),button,GLFW_RELEASE,0); frame(viewer);
        };
        auto before=viewer.clipping_planes(); drag(GLFW_MOUSE_BUTTON_LEFT,35,15);
        require(viewer.clipping_planes()!=before,"registered left drag did not rotate clipping plane");
        before=viewer.clipping_planes(); drag(GLFW_MOUSE_BUTTON_RIGHT,22,18);
        require(viewer.clipping_planes()!=before,"registered right drag did not translate clipping plane"); check_planes(viewer,viewer.capture());
        viewer.resize(460,320); frame(viewer); const auto resized=viewer.capture();
        check_planes(viewer,resized); viewer.set_samples(1); frame(viewer); same_picking(resized,viewer.capture());
        require(viewer.snapshot("vulkan-cross-section.png"),"cross-section snapshot failed");
        require(viewer.device().validation_errors()==0,"cross-section validation errors");
        std::cout << "PASS: native clipping/slab controls, guide-only visibility, registered rotation/translation, world-depth picking, resize, MSAA and snapshot\n";
    } catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
