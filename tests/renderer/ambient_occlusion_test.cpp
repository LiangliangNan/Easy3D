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
void same_picking(const FrameImage& a,const FrameImage& b) {
    require(a.depth==b.depth && a.ids.size()==b.ids.size(),"ambient occlusion changed scene depth");
    for (size_t i=0;i<a.ids.size();++i)
        require(a.ids[i].object==b.ids[i].object && a.ids[i].primitive==b.ids[i].primitive,
                "ambient occlusion changed IDs");
}
size_t pixel(const FrameImage& frame,unsigned x,unsigned y) {
    return (size_t(y)*frame.width+x)*4;
}
std::vector<DrawCommand> shaded(std::vector<DrawCommand> scene,unsigned width,unsigned height,
                                float radius=0.2f,float bias=0.005f) {
    scene.insert(scene.begin(),easy3d::graphics::shape::draw_ambient_occlusion(
        width,height,radius,bias,mat4::identity()));
    return scene;
}
int main(int argc,char**) {
    try {
        const bool validation = argc > 1;
        auto device=create_vulkan_device(validation, validation);

        const auto helper=easy3d::graphics::shape::draw_ambient_occlusion(40,30,0.5f,0.01f,mat4::identity());
        require(helper.overlay && helper.ambient_occlusion && !helper.eye_dome_lighting &&
                !helper.scene_depth_texture && !helper.overlay_depth_test &&
                !helper.overlay_independent_depth,"invalid ambient-occlusion command flags");
        require(helper.color==vec4(0.5f,0.01f,0,1),"ambient-occlusion parameters are not carried by the command");
        for (unsigned i=0;i<4;++i)
            for (unsigned j=0;j<4;++j)
                require(helper.projection(i,j)==mat4::identity()(i,j),
                        "ambient-occlusion projection was not preserved");
        require(helper.geometry && helper.geometry->topology()==Topology::triangles,
                "ambient-occlusion command is not a triangle quad");
        for (unsigned mode=0;mode<4;++mode) {
            bool rejected=false;
            try {
                if (mode==0) easy3d::graphics::shape::draw_ambient_occlusion(40,30,0.0f,0.005f,mat4::identity());
                else if (mode==1) easy3d::graphics::shape::draw_ambient_occlusion(40,30,0.5f,-0.1f,mat4::identity());
                else if (mode==2) easy3d::graphics::shape::draw_ambient_occlusion(0,30,0.5f,0.005f,mat4::identity());
                else {
                    mat4 bad=mat4::identity();bad(0,0)=std::nanf("");
                    easy3d::graphics::shape::draw_ambient_occlusion(40,30,0.5f,0.005f,bad);
                }
            } catch (const std::invalid_argument&) {rejected=true;}
            require(rejected,"invalid ambient-occlusion frame parameters were accepted");
        }

        for (unsigned samples:{1u,4u}) {
            if (!(device->supported_sample_counts()&samples)) continue;
            for (unsigned width:{64u,81u}) {
                const unsigned height=width;

                // A near left half hides part of a farther right half. The depth
                // jump stays inside the ambient-occlusion radius, since geometry
                // beyond the spherical kernel must not occlude.
                auto near=quad(-1,-1,0,1,-.5f,vec4(.8f,.8f,.8f,1));near.object_id=3;
                auto far=quad(0,-1,1,1,-.35f,vec4(.8f,.8f,.8f,1));far.object_id=4;
                const std::vector<DrawCommand> scene{near,far};
                const auto base=device->render(width,height,scene,vec4(0),samples);
                const auto occluded=device->render(width,height,shaded(scene,width,height),vec4(0),samples);
                same_picking(base,occluded);
                require(occluded.rgba!=base.rgba,"ambient occlusion was not rendered");

                // The background stays untouched.
                for (size_t i=0;i<4;++i)
                    require(occluded.rgba[i]==base.rgba[i],"ambient occlusion shaded the background");
                // Away from the depth discontinuity the surface is unoccluded.
                const auto near_px=pixel(occluded,width/4,height/2);
                const auto near_base=pixel(base,width/4,height/2);
                for (unsigned c=0;c<4;++c)
                    require(std::abs(int(occluded.rgba[near_px+c])-int(base.rgba[near_base+c]))<=1,
                            "ambient occlusion shaded a flat unoccluded surface");
                // Next to the far surface the near surface is darkened.
                const auto edge_px=pixel(occluded,width/2-2,height/2);
                const auto edge_base=pixel(base,width/2-2,height/2);
                require(occluded.rgba[edge_px]<base.rgba[edge_base]-8,
                        "ambient occlusion did not darken the occluded silhouette");
                require(occluded.rgba[edge_px+3]==base.rgba[edge_base+3],
                        "ambient occlusion changed surface alpha");
                // Surfaces in front never occlude the farther one.
                const auto far_px=pixel(occluded,width/2+2,height/2);
                const auto far_base=pixel(base,width/2+2,height/2);
                for (unsigned c=0;c<4;++c)
                    require(std::abs(int(occluded.rgba[far_px+c])-int(base.rgba[far_base+c]))<=1,
                            "ambient occlusion shaded a surface in front of an occluder");

                // A flat full-screen surface and the background keep their colors.
                auto flat=quad(-1,-1,1,1,-.4f,vec4(.8f,.8f,.8f,1));flat.object_id=5;
                const std::vector<DrawCommand> flat_scene{flat};
                const auto flat_base=device->render(width,height,flat_scene,vec4(0),samples);
                const auto flat_shaded=device->render(width,height,shaded(flat_scene,width,height),vec4(0),samples);
                same_picking(flat_base,flat_shaded);
                const auto center=pixel(flat_shaded,width/2,height/2),center_base=pixel(flat_base,width/2,height/2);
                for (unsigned c=0;c<4;++c)
                    require(std::abs(int(flat_shaded.rgba[center+c])-int(flat_base.rgba[center_base+c]))<=1,
                            "ambient occlusion shaded a flat surface");
                for (size_t i=0;i<4;++i)
                    require(flat_shaded.rgba[i]==flat_base.rgba[i],"flat ambient-occlusion frame shaded the background");

                // Ambient occlusion composes with eye-dome lighting and a depth preview.
                auto preview=quad(2,2,width/2.f,height/2.f,0,vec4(1));
                preview.overlay=preview.scene_depth_texture=true;
                preview.model_view_projection=screen_space_projection(width,height);
                auto combined=scene;combined.push_back(preview);
                auto with_edl=combined;
                with_edl.insert(with_edl.begin(),easy3d::graphics::shape::draw_eye_dome_lighting(
                    width,height,0.1f,1.0f,1.0f));
                const auto edl_only=device->render(width,height,with_edl,vec4(0),samples);
                const auto both=device->render(width,height,shaded(with_edl,width,height),vec4(0),samples);
                same_picking(edl_only,both);
                require(both.rgba!=edl_only.rgba,"ambient occlusion was dropped when eye-dome lighting was present");
            }
        }

        auto invalid=easy3d::graphics::shape::draw_ambient_occlusion(33,33,0.5f,0.005f,mat4::identity());
        for (unsigned mode=0;mode<6;++mode) {
            invalid.overlay=mode!=0;
            invalid.overlay_depth_test=mode==1;
            invalid.overlay_independent_depth=mode==2;
            invalid.texture=mode==3 ? std::make_shared<Image>(1,1,std::vector<unsigned char>(4,255)) : nullptr;
            invalid.color=mode==4 ? vec4(0,0,0,1) : mode==5 ? vec4(0.5f,-1,0,1) : vec4(0.5f,0.005f,0,1);
            bool rejected=false;
            try {device->render(33,33,{invalid});} catch (const std::invalid_argument&) {rejected=true;}
            require(rejected,"invalid ambient-occlusion flags were accepted");
        }
        require(device->validation_errors()==0,"ambient-occlusion validation errors");
        std::cout<<"PASS: ambient occlusion background, flat surfaces, occlusion, alpha, MSAA, eye-dome lighting, IDs and depth\n";
    } catch(const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
