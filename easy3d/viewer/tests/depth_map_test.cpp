// SPDX-License-Identifier: GPL-3.0-or-later
#include <tutorials/Tutorial_506_DepthMap/viewer.h>
#include <easy3d/core/model.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/util/initializer.h>
#include <easy3d/util/resource.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace easy3d;
void require(bool ok,const char* text) {if (!ok) throw std::runtime_error(text);}
void frame(VulkanViewer& v) {
    for (unsigned i=0;i<100;++i) {VulkanViewer::poll_events();if (v.draw_frame()) return;}
    throw std::runtime_error("depth-map frame did not present");
}
void same_picking(const graphics::FrameImage& a,const graphics::FrameImage& b) {
    require(a.depth==b.depth && a.ids.size()==b.ids.size(),"depth inset changed scene depth");
    for (size_t i=0;i<a.ids.size();++i)
        require(a.ids[i].object==b.ids[i].object && a.ids[i].primitive==b.ids[i].primitive,"depth inset changed IDs");
}
unsigned verify(TutorialDepthMap& v,const graphics::FrameImage& image) {
    const float left=20*v.dpi_scaling(),top=40*v.dpi_scaling();
    const float w=image.width/4.0f,h=image.height/4.0f;
    unsigned checked=0,foreground=0;
    for (int y=int(top)+4;y<int(top+h)-4;y+=3) for (int x=int(left)+4;x<int(left+w)-4;x+=3) {
        const float sx=(x+.5f-left)/w*image.width,sy=(y+.5f-top)/h*image.height;
        // Quarter-size resampling lands exactly on texel boundaries. Permit
        // either neighbor there to accommodate sampler interpolation rounding.
        const int x0=int(std::floor(sx-1e-4f)),x1=int(std::floor(sx+1e-4f));
        const int y0=int(std::floor(sy-1e-4f)),y1=int(std::floor(sy+1e-4f));
        const auto pixel=(size_t(y)*image.width+x)*4;bool matched=false;
        for (int yy=y0;yy<=y1;++yy) for (int xx=x0;xx<=x1;++xx) {
            const float depth=image.depth[size_t(yy)*image.width+xx];
            matched|=std::abs(image.rgba[pixel]-255*depth)<1.5f;
        }
        require(matched && image.rgba[pixel]==image.rgba[pixel+1] && image.rgba[pixel]==image.rgba[pixel+2],"inset does not display current raw scene depth");
        require(image.rgba[pixel+3]==255,"depth inset is not opaque");
        ++checked;foreground+=image.rgba[pixel]<254;
    }
    require(checked>100,"depth inset too small to verify");return foreground;
}
int main(int argc,char**) {
    try {
        initialize(false,false,false);
        VulkanViewerOptions options;options.visible=false;options.width=320;options.height=280;
        options.device.validation=options.device.synchronization_validation=argc>1;
        TutorialDepthMap v("Native depth map",options);
        auto* model=v.add_model(resource::directory()+"/data/fountain/pointcloud.ply");require(model,"fixture missing");
        model->renderer()->get_points_drawable("vertices")->set_point_size(5);
        const auto bare=v.capture();frame(v);const auto initial=v.capture();same_picking(bare,initial);
        require(initial.rgba!=bare.rgba && verify(v,initial)>50,"depth inset missing foreground");
        // Placement must match the original logical 20/40 margin and 1/4 size.
        const float left=20*v.dpi_scaling(),top=40*v.dpi_scaling(),right=left+v.width()/4.0f,bottom=top+v.height()/4.0f;
        for (unsigned y=0;y<initial.height;++y) for (unsigned x=0;x<initial.width;++x) {
            const auto i=(size_t(y)*initial.width+x)*4;bool changed=false;
            for (unsigned c=0;c<4;++c) changed|=initial.rgba[i+c]!=bare.rgba[i+c];
            if (changed) require(x+2>=left && x<=right+2 && y+2>=top && y<=bottom+2,"depth inset ignored DPI placement");
        }
        for (unsigned count:{1u,4u}) {
            if (!(v.device().supported_sample_counts()&count)) continue;
            v.set_samples(count);frame(v);const auto image=v.capture();same_picking(initial,image);verify(v,image);
        }
        v.camera()->setViewDirection(vec3(.2f,.1f,-1));frame(v);const auto moved=v.capture();
        require(moved.depth!=initial.depth && moved.rgba!=initial.rgba && verify(v,moved)>10,"camera motion left stale depth inset");
        v.camera()->setType(Camera::ORTHOGRAPHIC);v.fit_screen();frame(v);verify(v,v.capture());
        v.resize(360,300);frame(v);verify(v,v.capture());
        require(v.snapshot("vulkan-depth-map.png",1.5f,int(v.samples()),1),"depth map snapshot failed");
        v.clear_scene();frame(v);const auto empty=v.capture();require(verify(v,empty)==0,"empty scene retained stale depth");
        for (size_t i=0;i<empty.ids.size();++i)
            require(empty.depth[i]==1 && empty.ids[i].object==UINT32_MAX,"depth inset became a picking surface");
        require(v.device().validation_errors()==0,"depth map validation errors");
        std::cout<<"PASS: native GPU depth map, DPI, current-frame camera changes, projections, resize, MSAA, clear, snapshot and unchanged picking/depth\n";
    } catch(const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
