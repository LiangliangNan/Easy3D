// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/graphics/render_device.h>
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace easy3d;
using namespace easy3d::graphics;
void require(bool ok,const char* message) {if (!ok) throw std::runtime_error(message);}
DrawCommand quad(float x0,float y0,float x1,float y1,float z,const vec4& color) {
    std::vector<Vertex> vertices(4);
    const vec3 p[]={vec3(x0,y0,z),vec3(x1,y0,z),vec3(x1,y1,z),vec3(x0,y1,z)};
    const vec2 uv[]={vec2(0,0),vec2(1,0),vec2(1,1),vec2(0,1)};
    for (unsigned i=0;i<4;++i) {vertices[i].position=p[i];vertices[i].texcoord=uv[i];}
    DrawCommand result;result.geometry=std::make_shared<Geometry>(vertices,std::vector<uint32_t>{0,1,2,0,2,3});
    result.color=color;return result;
}
void same_picking(const FrameImage& a,const FrameImage& b) {
    require(a.depth==b.depth && a.ids.size()==b.ids.size(),"depth texture changed scene depth");
    for (size_t i=0;i<a.ids.size();++i)
        require(a.ids[i].object==b.ids[i].object && a.ids[i].primitive==b.ids[i].primitive,"depth texture changed IDs");
}
void verify(const FrameImage& frame) {
    // The preview occupies [3,26] x [4,21] and covers the full source image.
    for (unsigned y=5;y<20;++y) for (unsigned x=4;x<25;++x) {
        const unsigned sx=unsigned((x+.5f-3)/23*frame.width),sy=unsigned((y+.5f-4)/17*frame.height);
        const float expected=255*frame.depth[size_t(sy)*frame.width+sx];
        const auto target=(size_t(y)*frame.width+x)*4;
        for (unsigned c=0;c<3;++c)
            require(std::abs(frame.rgba[target+c]-expected)<1.5f,"GPU preview differs from current center-sample depth");
        require(frame.rgba[target+3]==255,"depth preview alpha is not opaque");
    }
}
int main(int argc,char**) {
    try {
        DeviceOptions options;options.validation=options.synchronization_validation=argc>1;
        auto device=create_vulkan_device(options);
        for (unsigned samples:{1u,4u}) {
            if (!(device->supported_sample_counts()&samples)) continue;
            for (unsigned width:{65u,83u,41u}) {
                const unsigned height=width-16;
                auto left=quad(-1,-1,-.1f,1,-.8f,vec4(1,0,0,1));left.object_id=7;
                auto upper=quad(.1f,0,1,1,.6f,vec4(0,0,1,1));upper.object_id=11;
                auto guide=quad(-1,-1,1,1,0,vec4(1,0,1,.2f));guide.overlay=guide.overlay_depth_test=true;
                auto preview=quad(3,4,26,21,0,vec4(1));preview.overlay=preview.scene_depth_texture=true;
                preview.model_view_projection=screen_space_projection(width,height);
                auto hud=quad(.2f,-.8f,.9f,-.2f,-.9f,vec4(0,1,0,1));hud.overlay=hud.overlay_independent_depth=true;
                // Exercise both pass variants, including HUD depth that must
                // never be sampled as scene depth and a depth-tested annotation.
                for (bool widget:{false,true}) {
                    std::vector<DrawCommand> scene{left,upper,guide};if (widget) scene.push_back(hud);
                    const auto base=device->render(width,height,scene,vec4(0),samples);
                    scene.push_back(preview);const auto image=device->render(width,height,scene,vec4(0),samples);
                    same_picking(base,image);verify(image);
                    require(base.rgba!=image.rgba,"depth texture was not rendered");
                    // A new surface depth must be visible on this frame, rather
                    // than sampling the target from its preceding render.
                    scene[0]=quad(-1,-1,-.1f,1,.2f,vec4(1,0,0,1));scene[0].object_id=7;
                    const auto changed=device->render(width,height,scene,vec4(0),samples);verify(changed);
                    require(changed.rgba!=image.rgba,"depth preview retained stale frame");
                    const auto empty=device->render(width,height,{preview},vec4(0),samples);verify(empty);
                    for (size_t i=0;i<empty.ids.size();++i)
                        require(empty.depth[i]==1 && empty.ids[i].object==UINT32_MAX && empty.ids[i].primitive==UINT32_MAX,"empty preview became a scene surface");
                }
            }
        }
        auto invalid=quad(-1,-1,1,1,0,vec4(1));invalid.scene_depth_texture=true;
        for (unsigned mode=0;mode<4;++mode) {
            invalid.overlay=mode!=0;invalid.overlay_depth_test=mode==1;invalid.overlay_independent_depth=mode==2;
            invalid.texture=mode==3 ? std::make_shared<Image>(1,1,std::vector<unsigned char>(4,255)) : nullptr;
            bool rejected=false;try {device->render(33,33,{invalid});} catch(const std::invalid_argument&) {rejected=true;}
            require(rejected,"invalid scene-depth texture flags were accepted");
        }
        require(device->validation_errors()==0,"depth texture validation errors");
        std::cout<<"PASS: GPU scene-depth texture, current-frame sampling, orientation, MSAA, HUD separation, resize, cleared background and preserved IDs/depth\n";
    } catch(const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
