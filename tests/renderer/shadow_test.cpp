// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/renderer/render_device.h>
#include <easy3d/renderer/shape.h>
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace easy3d;
using namespace easy3d::graphics;
void require(bool ok,const char* message) {if (!ok) throw std::runtime_error(message);}
DrawCommand quad(float x0,float y0,float x1,float y1,float z,const vec4& color) {
    std::vector<Vertex> vertices(4);
    const vec3 p[]={vec3(x0,y0,z),vec3(x1,y0,z),vec3(x1,y1,z),vec3(x0,y1,z)};
    for (unsigned i=0;i<4;++i) vertices[i].position=p[i];
    DrawCommand result;result.geometry=std::make_shared<Geometry>(vertices,std::vector<uint32_t>{0,1,2,0,2,3});
    result.color=color;return result;
}
size_t pixel(const FrameImage& frame,unsigned x,unsigned y) {
    return (size_t(y)*frame.width+x)*4;
}
void same_picking(const FrameImage& a,const FrameImage& b) {
    require(a.depth==b.depth && a.ids.size()==b.ids.size(),"shadow changed scene depth");
    for (size_t i=0;i<a.ids.size();++i)
        require(a.ids[i].object==b.ids[i].object && a.ids[i].primitive==b.ids[i].primitive,
                "shadow changed IDs");
}
// x,y -> [0,1], z -> [0,1]. The z term shifts the projected shadow away from
// the caster so the receiver's shadowed and lit regions are both visible.
mat4 light_matrix() {
    mat4 m=mat4::identity();
    m(0,0)=.5f; m(0,2)=.6f; m(0,3)=.5f;
    m(1,1)=.5f; m(1,2)=.6f; m(1,3)=.5f;
    m(2,2)=.5f; m(2,3)=.5f;
    return m;
}
int main(int argc,char**) {
    try {
        const bool validation = argc > 1;
        auto device=create_vulkan_device(validation, validation);
        const mat4 light=light_matrix();

        const auto helper=easy3d::graphics::shape::draw_shadow(64,64,light,.6f,.1f,.1f,10.f,128);
        require(helper.overlay && helper.shadow && !helper.scene_depth_texture &&
                !helper.eye_dome_lighting && !helper.ambient_occlusion &&
                !helper.overlay_depth_test && !helper.overlay_independent_depth,
                "invalid shadow command flags");
        require(helper.color==vec4(.6f,.1f,.1f,10.f),"shadow parameters are not carried by the command");
        for (unsigned col=0;col<4;++col) for (unsigned row=0;row<4;++row)
            require(helper.shadow_matrix(row,col)==light(row,col),"shadow command lost its matrix");
        require(helper.shadow_map_size==128,"shadow command lost its map size");
        require(helper.geometry && helper.geometry->topology()==Topology::triangles &&
                helper.geometry->vertices().size()==4,"shadow command is not a fullscreen quad");
        for (unsigned mode=0;mode<6;++mode) {
            bool rejected=false;
            try {
                if (mode==0) easy3d::graphics::shape::draw_shadow(64,64,light,1.5f,0,0.1f,10.f,128);
                else if (mode==1) easy3d::graphics::shape::draw_shadow(64,64,light,.6f,-.1f,0.1f,10.f,128);
                else if (mode==2) easy3d::graphics::shape::draw_shadow(64,64,light,.6f,0,0.f,10.f,128);
                else if (mode==3) easy3d::graphics::shape::draw_shadow(64,64,light,.6f,0,10.f,1.f,128);
                else if (mode==4) easy3d::graphics::shape::draw_shadow(64,64,light,.6f,0,0.1f,10.f,0);
                else easy3d::graphics::shape::draw_shadow(0,64,light,.6f,0,0.1f,10.f,128);
            } catch (const std::invalid_argument&) {rejected=true;}
            require(rejected,"invalid shadow parameters were accepted");
        }

        DrawCommand receiver=quad(-1,-1,1,1,0,vec4(1));
        receiver.object_id=UINT32_MAX;
        receiver.shadow_background=true;
        receiver.shadow_matrix=light;
        receiver.shadow_map_size=128;
        DrawCommand caster=quad(-.5f,-.5f,.5f,.5f,-.4f,vec4(1));
        caster.object_id=7;
        caster.shadow_caster=true;
        caster.shadow_matrix=light;
        caster.shadow_map_size=128;

        for (unsigned samples:{1u,4u}) {
            if (!(device->supported_sample_counts()&samples)) continue;
            const unsigned width=128,height=128;
            const std::vector<DrawCommand> scene{receiver,caster};
            const auto base=device->render(width,height,scene,vec4(0),samples);
            // Shadowed receiver region, far from the caster's screen footprint.
            const size_t shadowed=pixel(base,16,112);
            // Lit receiver region on the opposite side.
            const size_t lit=pixel(base,112,16);
            // Caster's own pixels must stay lit.
            const size_t center=pixel(base,width/2,height/2);
            for (unsigned c=0;c<3;++c) require(base.rgba[shadowed+c]==255,"receiver is not white");

            for (float softness:{0.f,.5f}) {
                auto commands=scene;
                commands.insert(commands.begin(),
                                easy3d::graphics::shape::draw_shadow(width,height,light,.6f,softness,.1f,10.f,128));
                const auto shaded=device->render(width,height,commands,vec4(0),samples);
                same_picking(base,shaded);
                require(shaded.ids[shadowed/4].object==UINT32_MAX,
                        "shadow background receiver changed the picking ID");
                require(std::abs(shaded.depth[shadowed/4]-.5f)<1e-5f,
                        "shadow changed receiver depth");
                for (unsigned c=0;c<3;++c) {
                    require(shaded.rgba[shadowed+c]<base.rgba[shadowed+c]-20,
                            "shadow did not darken the occluded receiver");
                    require(shaded.rgba[shadowed+c]>=base.rgba[shadowed+c]*0.35f,
                            "shadow is darker than its configured darkness");
                    require(std::abs(int(shaded.rgba[lit+c])-int(base.rgba[lit+c]))<=1,
                            "shadow darkened a lit receiver");
                    require(std::abs(int(shaded.rgba[center+c])-int(base.rgba[center+c]))<=2,
                            "shadow darkened its caster");
                }
                require(shaded.rgba[shadowed+3]==255,"shadow changed receiver alpha");
                if (softness>0)
                    require(shaded.rgba[shadowed]>=base.rgba[shadowed]*0.35f,
                            "soft shadow lost its shadowed darkness");
            }

            // Darkness zero is a no-op.
            auto clear=scene;
            clear.insert(clear.begin(),
                         easy3d::graphics::shape::draw_shadow(width,height,light,0.f,0.f,.1f,10.f,128));
            require(device->render(width,height,clear,vec4(0),samples).rgba==base.rgba,
                    "zero darkness changed the frame");
        }

        // Device-level validation of shadow command combinations.
        DrawCommand invalid=easy3d::graphics::shape::draw_shadow(64,64,light,.6f,0,.1f,10.f,128);
        for (unsigned mode=0;mode<7;++mode) {
            auto command=invalid;
            command.overlay=mode!=0;
            command.texture=mode==1 ? std::make_shared<Image>(1,1,std::vector<unsigned char>(4,255)) : nullptr;
            command.overlay_depth_test=mode==2;
            command.overlay_independent_depth=mode==3;
            if (mode==4) command.shadow_map_size=1000;
            if (mode==5) command.color.x=2;
            if (mode==6) command.shadow_matrix(0,0)=NAN;
            bool rejected=false;
            try {device->render(64,64,{command});} catch (const std::invalid_argument&) {rejected=true;}
            require(rejected,"invalid shadow command flags were accepted");
        }
        auto lines=caster; lines.geometry=std::make_shared<Geometry>(
            std::vector<Vertex>{Vertex{},Vertex{}},std::vector<uint32_t>{0,1},Topology::lines);
        bool rejected=false;
        try {device->render(64,64,{lines});} catch (const std::invalid_argument&) {rejected=true;}
        require(rejected,"line geometry was accepted as a shadow caster");
        auto foreground=receiver; foreground.object_id=3;
        rejected=false;
        try {device->render(64,64,{foreground});} catch (const std::invalid_argument&) {rejected=true;}
        require(rejected,"shadow background with a foreground ID was accepted");

        require(device->validation_errors()==0,"shadow validation errors");
        std::cout<<"PASS: shadow map hard/soft darkening, lit regions, caster, background, IDs, depth and validation\n";
    } catch(const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
