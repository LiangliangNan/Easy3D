// SPDX-License-Identifier: GPL-3.0-or-later
#include <tutorials/Tutorial_207_RealCamera/viewer.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/util/initializer.h>
#include <easy3d/util/resource.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace easy3d;
void require(bool ok,const char* message) {if (!ok) throw std::runtime_error(message);}
struct RealCameraViewer : TutorialRealCamera {
    using TutorialRealCamera::TutorialRealCamera;
    Rect image_rect() const {return calculate_image_rect();}
    const std::string& hint() const {return hint_;}
};
void frame(VulkanViewer& v) {
    for (unsigned i=0;i<100;++i) {VulkanViewer::poll_events();if (v.draw_frame()) return;glfwWaitEventsTimeout(.01);}
    throw std::runtime_error("real-camera frame did not present");
}
void same_picking(const graphics::FrameImage& a,const graphics::FrameImage& b) {
    require(a.depth==b.depth && a.ids.size()==b.ids.size(),"image/cross/MSAA changed scene depth");
    for (size_t i=0;i<a.ids.size();++i)
        require(a.ids[i].object==b.ids[i].object && a.ids[i].primitive==b.ids[i].primitive,"image/cross/MSAA changed source IDs");
}
struct View {float focal;mat3 R;vec3 t;};
std::vector<View> read_views(const std::string& path) {
    std::ifstream in(path);std::string line;std::getline(in,line);unsigned count=0,points=0;in>>count>>points;
    std::vector<View> views(count);
    for (auto& v:views) {
        float k0,k1;in>>v.focal>>k0>>k1;
        for (unsigned row=0;row<3;++row) for (unsigned col=0;col<3;++col) in>>v.R(row,col);
        in>>v.t.x>>v.t.y>>v.t.z;
    }
    require(bool(in) && count>1,"Bundler fixture missing camera views");return views;
}
void image_pixels(RealCameraViewer& v,const graphics::FrameImage& image,const std::string& file) {
    std::vector<unsigned char> rgba;int w=0,h=0,channels=0;
    require(ImageIO::load(file,rgba,w,h,channels,4,false),"source image missing");const auto rect=v.image_rect();
    for (float fraction:{.23f,.47f,.71f}) {
        const int x=int(rect.x_min()+rect.width()*fraction),y=int(rect.y_min()+rect.height()*.37f);
        require(x>=0 && y>=0 && x<int(image.width) && y<int(image.height),"inset outside viewport");
        const float u=(x+.5f-rect.x_min())/rect.width()*w-.5f;
        const float t=(y+.5f-rect.y_min())/rect.height()*h-.5f;
        const int x0=int(std::floor(u)),y0=int(std::floor(t));const float fx=u-x0,fy=t-y0;
        for (unsigned c=0;c<3;++c) {
            float expected=0;
            for (int dy=0;dy<2;++dy) for (int dx=0;dx<2;++dx) {
                const int px=std::max(0,std::min(w-1,x0+dx)),py=std::max(0,std::min(h-1,y0+dy));
                expected+=rgba[(size_t(py)*w+px)*4+c]*(dx?fx:1-fx)*(dy?fy:1-fy);
            }
            require(std::abs(image.rgba[(size_t(y)*image.width+x)*4+c]-expected)<4,"inset texture orientation/filter mismatch");
        }
    }
}
int main(int argc,char**) {
    try {
        initialize(false,false,false);
        const mat3 rotation=mat3(quat(vec3(0,1,0),.4f).matrix());const vec3 translation(1,-2,3);
        for (bool convert:{false,true}) {
            const auto ray=TutorialRealCamera::pixel_to_ray(18,39,100,120,3,31,27,rotation,translation,convert);
            const auto p=TutorialRealCamera::camera_pos(rotation,translation)+5*ray;
            const auto pixel=TutorialRealCamera::point_to_pixel(p,100,120,3,31,27,rotation,translation,convert);
            require((pixel-vec2(18,39)).norm()<1e-3f,"intrinsic/extrinsic ray projection round trip failed");
        }
        const auto root=resource::directory()+"/data/fountain/";const auto views=read_views(root+"bundle.out");
        VulkanViewerOptions options;options.visible=false;options.width=500;options.height=420;
        options.device.validation=options.device.synchronization_validation=argc>1;
        RealCameraViewer v("Native real camera",root+"bundle.out",root+"pointcloud.ply",options);
        require(v.models().size()==1 && v.hint().find("corresponding 3D ray")!=std::string::npos,"scene/usage hint missing");
        const auto bare=v.capture();frame(v);const auto initial=v.capture();same_picking(bare,initial);
        require(bare.rgba!=initial.rgba,"image inset missing");image_pixels(v,initial,root+"images/0000.jpg");
        if (v.device().supported_sample_counts()&4) {v.set_samples(4);frame(v);same_picking(initial,v.capture());}
        auto key=glfwSetKeyCallback(v.window(),nullptr);glfwSetKeyCallback(v.window(),key);
        auto press=[&](int code) {key(v.window(),code,0,GLFW_PRESS,0);frame(v);};
        auto cursor=glfwSetCursorPosCallback(v.window(),nullptr);glfwSetCursorPosCallback(v.window(),cursor);
        auto hover=[&](int x,int y) {cursor(v.window(),x/v.dpi_scaling(),y/v.dpi_scaling());frame(v);};
        const auto rect=v.image_rect();
        require(std::abs(rect.x_min()-20*v.dpi_scaling())<1e-4f && std::abs(rect.y_min()-120*v.dpi_scaling())<1e-4f,"inset margins ignored DPI");
        const auto clean=v.capture();hover(int(rect.center().x),int(rect.center().y));
        LinesDrawable* ray=nullptr;LinesDrawable* cameras=nullptr;
        for (const auto& d:v.drawables()) {
            if (d->name()=="ray") ray=dynamic_cast<LinesDrawable*>(d.get());
            if (d->name()=="cameras") cameras=dynamic_cast<LinesDrawable*>(d.get());
        }
        require(ray && ray->is_visible() && cameras,"image hover did not create visible 3D ray/camera guides");
        require(ray->cpu_buffers().vertices.size()==2 &&
                (ray->cpu_buffers().vertices[0]-TutorialRealCamera::camera_pos(views[0].R,views[0].t)).norm()<1e-4f,"ray origin differs from source camera");
        hover(-10,-10);require(!ray->is_visible(),"leaving inset retained ray");same_picking(clean,v.capture());
        const auto scene=v.capture();bool marked=false;
        for (size_t i=0;i<scene.ids.size();i+=7) {
            if (scene.ids[i].object!=0) continue;
            const int x=int(i%scene.width),y=int(i/scene.width);
            if (x>=rect.x_min() && x<=rect.x_max() && y>=rect.y_min() && y<=rect.y_max()) continue;
            const auto p=v.unproject_pixel(x,y,scene.depth[i]);
            const auto q=TutorialRealCamera::point_to_pixel(p,views[0].focal,views[0].focal,0,1520.69f,1006.81f,views[0].R,views[0].t);
            if (!(q.x>310 && q.x<2760 && q.y>210 && q.y<1840)) continue;
            hover(x,y);const auto cross=v.capture();same_picking(scene,cross);
            const int cx=int(rect.x_min()+q.x/3072*rect.width()),cy=int(rect.y_min()+q.y/2048*rect.height());
            const auto offset=(size_t(cy)*cross.width+cx)*4;
            require(cross.rgba[offset]<10 && cross.rgba[offset+1]>240 && cross.rgba[offset+2]<10,"scene hover did not mark corresponding image point");
            marked=true;break;
        }
        require(marked,"no source point available for correspondence check");
        hover(-10,-10);require(v.capture().rgba==scene.rgba,"background hover left stale correspondence cross");
        press(GLFW_KEY_H);require(!cameras->is_visible(),"H did not hide cameras");press(GLFW_KEY_H);require(cameras->is_visible(),"H did not show cameras");
        press(GLFW_KEY_SPACE);require(v.title().find("View_1")!=std::string::npos,"Space did not switch view");
        const auto expected=TutorialRealCamera::camera_pos(views[1].R,views[1].t);
        require((v.camera()->position()-expected).norm()<1e-3f,"ground truth camera pose mismatch");
        image_pixels(v,v.capture(),root+"images/0001.jpg");
        press(GLFW_KEY_1);require((v.camera()->position()-expected).norm()<1e-3f,"calibration camera pose mismatch");
        press(GLFW_KEY_2);require((v.camera()->position()-expected).norm()<1e-3f,"ground-truth mode did not restore pose");
        v.resize(520,380);frame(v);image_pixels(v,v.capture(),root+"images/0001.jpg");
        require(v.snapshot("vulkan-real-camera.png",1, int(v.samples()),1),"real camera snapshot failed");
        require(v.device().validation_errors()==0,"real camera validation errors");
        std::cout<<"PASS: native calibrated camera, shared ray/projection math, view/image switching, DPI inset, cross/ray tools, MSAA, depth/IDs and snapshots\n";
    } catch(const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
