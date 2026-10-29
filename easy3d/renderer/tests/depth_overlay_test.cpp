// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/renderer/render_device.h>
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace easy3d;
using namespace easy3d::graphics;
void require(bool value,const char* message) { if (!value) throw std::runtime_error(message); }
DrawCommand rectangle(float x0,float y0,float x1,float y1,float z,const vec4& color,bool overlay=false,bool depth=false) {
    std::vector<Vertex> v(4);
    v[0].position=vec3(x0,y0,z); v[1].position=vec3(x1,y0,z); v[2].position=vec3(x1,y1,z); v[3].position=vec3(x0,y1,z);
    DrawCommand d; d.geometry=std::make_shared<Geometry>(v,std::vector<uint32_t>{0,1,2,0,2,3}); d.color=color;
    d.overlay=overlay; d.overlay_depth_test=depth; return d;
}
void same_picking(const FrameImage& a,const FrameImage& b) {
    require(a.depth==b.depth && a.ids.size()==b.ids.size(),"depth annotation changed scene depth");
    for (size_t i=0;i<a.ids.size();++i)
        require(a.ids[i].object==b.ids[i].object && a.ids[i].primitive==b.ids[i].primitive,"depth annotation changed source picking IDs");
}
void probe(const FrameImage& f,unsigned x,unsigned y,int r,int g,int b,int a=255) {
    const auto i=(size_t(y)*f.width+x)*4;
    const int expected[]={r,g,b,a};
    for (unsigned c=0;c<4;++c) require(std::abs(int(f.rgba[i+c])-expected[c])<=1,"depth annotation blend/occlusion probe mismatch");
}
int main(int argc,char**) {
    try {
        DeviceOptions options; options.validation=options.synchronization_validation=argc>1;
        auto device=create_vulkan_device(options);
        auto near=rectangle(-1,-.8f,-.2f,.8f,-.5f,vec4(0,0,1,1)); near.object_id=13;
        auto far=rectangle(.2f,-.8f,1,.8f,.5f,vec4(0,0,1,1)); far.object_id=17;
        auto guide=rectangle(-.95f,-.95f,.95f,.95f,0,vec4(1,0,0,.25f),true,true);
        for (unsigned count:{1u,4u}) {
            if (!(device->supported_sample_counts()&count)) continue;
            const auto base=device->render(65,65,{near,far},vec4(0),count);
            auto hud_near=rectangle(-.9f,-.9f,.9f,.9f,.2f,vec4(0,1,0,1),true);
            auto hud_far=rectangle(-.9f,-.9f,.9f,.9f,.6f,vec4(1,0,0,1),true);
            hud_near.overlay_independent_depth=hud_far.overlay_independent_depth=true;
            const auto hud_frame=device->render(65,65,{hud_near,near,hud_far,far},vec4(0),count);
            probe(hud_frame,16,32,0,255,0); // HUD wins even behind opaque scene depth
            same_picking(base,hud_frame);
            const auto hud_reversed=device->render(65,65,{hud_far,near,hud_near,far},vec4(0),count);
            require(hud_reversed.rgba==hud_frame.rgba,"3D HUD ignored its own depth ordering");same_picking(base,hud_reversed);
            const auto cleared=device->render(65,65,{near,far,hud_far},vec4(0),count);
            probe(cleared,16,32,255,0,0);same_picking(base,cleared); // own depth clears each frame
            auto flat=rectangle(-1,-1,1,1,1,vec4(0,0,1,1),true);
            const auto layered=device->render(65,65,{flat,hud_near,near,far},vec4(0),count);
            probe(layered,16,32,0,0,255);same_picking(base,layered);
            hud_near.lighting=true;
            const auto lit=device->render(65,65,{near,far,hud_near},vec4(0),count);
            hud_near.normal_matrix=mat3(0,-1,0,0,0,1,1,0,0);
            const auto relit=device->render(65,65,{near,far,hud_near},vec4(0),count);
            require(lit.rgba!=relit.rgba,"3D HUD lighting ignored normal transform");same_picking(base,relit);
            const auto no_hud=device->render(65,65,{near,far},vec4(0),count);
            require(no_hud.rgba==base.rgba,"3D HUD left stale color after removal");same_picking(base,no_hud);
            const auto tinted=device->render(65,65,{guide,far,near},vec4(0),count);
            same_picking(base,tinted);
            probe(tinted,16,32,0,0,255);       // plane is behind the near opaque surface
            probe(tinted,48,32,64,0,191);      // plane is in front of the far opaque surface
            probe(tinted,32,32,64,0,0,64);     // visible plane on transparent background creates no hit
            require(tinted.ids[32*65+32].object==UINT32_MAX && tinted.depth[32*65+32]==1,"plane-only pixel created a picking surface");
            auto hud=rectangle(-1,-1,1,1,.9f,vec4(0,1,0,.5f),true);
            const auto composed=device->render(65,65,{hud,guide,near,far},vec4(0),count);
            same_picking(base,composed); probe(composed,16,32,0,128,128);
            auto coplanar=guide; coplanar.geometry=rectangle(-1,-1,1,1,-.5f,vec4(1)).geometry;
            probe(device->render(65,65,{near,coplanar},vec4(0),count),16,32,64,0,191);
            auto zero=guide; zero.color.w=0;
            require(device->render(65,65,{near,far,zero},vec4(0),count).rgba==base.rgba,"zero-alpha depth overlay changed color");
            auto background=rectangle(-1,-1,1,1,.8f,vec4(0,0,1,1));
            auto back=rectangle(-1,-1,1,1,.6f,vec4(0,1,0,.5f),true,true);
            auto front=rectangle(-1,-1,1,1,.2f,vec4(1,0,0,.5f),true,true);
            const auto ordered=device->render(65,65,{back,background,front},vec4(0),count);
            probe(ordered,32,32,128,64,64);
            const auto reversed=device->render(65,65,{front,background,back},vec4(0),count);
            probe(reversed,32,32,64,128,64); // annotations never write depth, so caller order is honored
            same_picking(ordered,reversed);
            const auto alone=device->render(65,65,{guide},vec4(0),count);
            for (size_t i=0;i<alone.ids.size();++i)
                require(alone.ids[i].object==UINT32_MAX && alone.ids[i].primitive==UINT32_MAX && alone.depth[i]==1,"depth-only annotation changed empty-scene IDs/depth");
            if (count==4) {
                unsigned partial=0;
                for (size_t i=0;i<alone.ids.size();++i) partial+=alone.rgba[4*i+3]>0 && alone.rgba[4*i+3]<63;
                require(partial>0,"transparent guide boundaries lack MSAA coverage");
            }
        }
        if (device->supported_sample_counts()&4) {
            std::vector<Vertex> v(3); v[0].position=vec3(-.8f,-.7f,0); v[1].position=vec3(.8f,-.4f,0); v[2].position=vec3(-.1f,.8f,0);
            DrawCommand triangle; triangle.geometry=std::make_shared<Geometry>(v); triangle.color=vec4(0,0,1,1);
            guide=rectangle(-1,-1,1,1,.5f,vec4(1,0,0,.5f),true,true);
            const auto edge=device->render(65,65,{triangle},vec4(0),4);
            const auto covered=device->render(65,65,{triangle,guide},vec4(0),4); same_picking(edge,covered);
            unsigned partial=0;
            for (size_t i=0;i<edge.ids.size();++i) {
                const int blue=edge.rgba[4*i+2];
                require(covered.rgba[4*i+2]==blue,"annotation overwrote nearer MSAA samples");
                require(std::abs(int(covered.rgba[4*i])-(255-blue)*.5f)<=1.5f &&
                        std::abs(int(covered.rgba[4*i+3])-(blue+(255-blue)*.5f))<=1.5f,
                        "annotation depth testing/blending did not happen before the MSAA resolve");
                partial+=blue>0 && blue<255;
            }
            require(partial>30,"per-sample depth/alpha oracle did not exercise opaque edges");
        }
        for (unsigned size:{31u,73u,39u}) {
            auto d=rectangle(-1,-1,1,1,0,vec4(1,0,0,.5f),true,true);
            probe(device->render(size,size,{d},vec4(0)),size/2,size/2,128,0,0,128);
        }
        auto invalid=guide; invalid.overlay=false; bool rejected=false;
        try { device->render(65,65,{invalid}); } catch (const std::invalid_argument&) { rejected=true; }
        require(rejected,"depth annotation flag was accepted on an opaque scene draw");
        invalid=guide;invalid.overlay_independent_depth=true;rejected=false;
        try { device->render(65,65,{invalid}); } catch (const std::invalid_argument&) {rejected=true;}
        require(rejected,"conflicting HUD/scene depth flags were accepted");
        invalid.overlay_depth_test=false;invalid.overlay=false;rejected=false;
        try { device->render(65,65,{invalid}); } catch (const std::invalid_argument&) {rejected=true;}
        require(rejected,"independent depth was accepted on an opaque scene draw");
        require(device->validation_errors()==0,"depth annotation validation errors");
        std::cout << "PASS: scene-depth alpha annotations, occlusion/coplanarity, premultiplied readback, ordered layers/HUD, per-sample MSAA depth/blending, preserved picking/depth, resize\n";
    } catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
