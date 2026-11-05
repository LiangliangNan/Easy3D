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
    require(a.depth==b.depth && a.ids.size()==b.ids.size(),"eye-dome lighting changed scene depth");
    for (size_t i=0;i<a.ids.size();++i)
        require(a.ids[i].object==b.ids[i].object && a.ids[i].primitive==b.ids[i].primitive,
                "eye-dome lighting changed IDs");
}
size_t pixel(const FrameImage& frame,unsigned x,unsigned y) {
    return (size_t(y)*frame.width+x)*4;
}
std::vector<DrawCommand> lit(std::vector<DrawCommand> scene,unsigned width,unsigned height) {
    scene.insert(scene.begin(),easy3d::graphics::shape::draw_eye_dome_lighting(
        width,height,0.1f,1.0f,1.0f));
    return scene;
}
int main(int argc,char**) {
    try {
        const bool validation = argc > 1;
        auto device=create_vulkan_device(validation, validation);

        const auto helper=easy3d::graphics::shape::draw_eye_dome_lighting(40,30,0.1f,1.0f,2.0f);
        require(helper.overlay && helper.eye_dome_lighting && !helper.scene_depth_texture &&
                !helper.overlay_depth_test && !helper.overlay_independent_depth,"invalid EDL command flags");
        require(helper.color==vec4(0.1f,1.0f,2.0f,1),"EDL parameters are not carried by the command");
        require(helper.geometry && helper.geometry->topology()==Topology::triangles,
                "EDL command is not a triangle quad");
        for (unsigned mode=0;mode<4;++mode) {
            bool rejected=false;
            try {
                if (mode==0) easy3d::graphics::shape::draw_eye_dome_lighting(40,30,0.0f,1.0f,1.0f);
                else if (mode==1) easy3d::graphics::shape::draw_eye_dome_lighting(40,30,1.0f,1.0f,1.0f);
                else if (mode==2) easy3d::graphics::shape::draw_eye_dome_lighting(40,30,0.1f,1.0f,0.0f);
                else easy3d::graphics::shape::draw_eye_dome_lighting(0,30,0.1f,1.0f,1.0f);
            } catch (const std::invalid_argument&) {rejected=true;}
            require(rejected,"invalid EDL frame parameters were accepted");
        }

        for (unsigned samples:{1u,4u}) {
            if (!(device->supported_sample_counts()&samples)) continue;
            for (unsigned width:{64u,81u}) {
                const unsigned height=width;

                // Two halves at very different depths meet at the vertical center.
                auto near=quad(-1,-1,0,1,-.8f,vec4(.8f,.8f,.8f,1));near.object_id=3;
                auto far=quad(0,-1,1,1,.6f,vec4(.8f,.8f,.8f,1));far.object_id=4;
                const std::vector<DrawCommand> scene{near,far};
                const auto base=device->render(width,height,scene,vec4(0),samples);
                const auto shaded=device->render(width,height,lit(scene,width,height),vec4(0),samples);
                same_picking(base,shaded);
                require(shaded.rgba!=base.rgba,"eye-dome lighting was not rendered");

                if (samples==1) {
                    // The lighter background stays untouched.
                    for (size_t i=0;i<4;++i)
                        require(shaded.rgba[i]==base.rgba[i],"eye-dome lighting shaded the background");
                    // The near surface is darkened next to the far one.
                    const auto near_px=pixel(shaded,width/2-2,height/2);
                    const auto base_px=pixel(base,width/2-2,height/2);
                    require(shaded.rgba[near_px]<base.rgba[base_px]-20,
                            "eye-dome lighting did not darken the near silhouette");
                    require(shaded.rgba[near_px+3]==255,"eye-dome lighting changed surface alpha");
                    // The far surface keeps its color: all neighbors are nearer or flat.
                    const auto far_px=pixel(shaded,width/2+2,height/2);
                    const auto far_base=pixel(base,width/2+2,height/2);
                    for (unsigned c=0;c<4;++c)
                        require(std::abs(int(shaded.rgba[far_px+c])-int(base.rgba[far_base+c]))<=1,
                                "eye-dome lighting shaded the far side of a silhouette");
                }
                else {
                    const auto near_px=pixel(shaded,width/2-2,height/2);
                    const auto base_px=pixel(base,width/2-2,height/2);
                    require(shaded.rgba[near_px]<=base.rgba[base_px],
                            "MSAA eye-dome lighting brightened the silhouette");
                    require(shaded.rgba[near_px+3]>=base.rgba[base_px+3]-1,
                            "MSAA eye-dome lighting changed coverage");
                }

                // A flat surface keeps its color away from its own background edge.
                auto flat=quad(-.6f,-.6f,.6f,.6f,-.2f,vec4(.8f,.8f,.8f,1));flat.object_id=5;
                const std::vector<DrawCommand> flat_scene{flat};
                const auto flat_base=device->render(width,height,flat_scene,vec4(0),samples);
                const auto flat_shaded=device->render(width,height,lit(flat_scene,width,height),vec4(0),samples);
                same_picking(flat_base,flat_shaded);
                const auto center=pixel(flat_shaded,width/2,height/2),center_base=pixel(flat_base,width/2,height/2);
                for (unsigned c=0;c<4;++c)
                    require(std::abs(int(flat_shaded.rgba[center+c])-int(flat_base.rgba[center_base+c]))<=1,
                            "eye-dome lighting shaded a flat surface");
                // Background and flat-surface interior are unchanged even with MSAA.
                for (size_t i=0;i<4;++i)
                    require(flat_shaded.rgba[i]==flat_base.rgba[i],"flat EDL frame shaded the background");

                // Depth preview and EDL share this frame's sampled depth in one pass.
                auto preview=quad(2,2,width/2.f,height/2.f,0,vec4(1));
                preview.overlay=preview.scene_depth_texture=true;
                preview.model_view_projection=screen_space_projection(width,height);
                auto combined=scene;combined.push_back(preview);
                const auto preview_only=device->render(width,height,combined,vec4(0),samples);
                const auto preview_lit=device->render(width,height,lit(combined,width,height),vec4(0),samples);
                same_picking(preview_only,preview_lit);
                require(preview_lit.rgba!=preview_only.rgba,"EDL was dropped when a depth preview was present");
            }
        }

        auto invalid=easy3d::graphics::shape::draw_eye_dome_lighting(33,33,0.1f,1.0f,1.0f);
        for (unsigned mode=0;mode<4;++mode) {
            invalid.overlay=mode!=0;
            invalid.overlay_depth_test=mode==1;
            invalid.overlay_independent_depth=mode==2;
            invalid.texture=mode==3 ? std::make_shared<Image>(1,1,std::vector<unsigned char>(4,255)) : nullptr;
            bool rejected=false;
            try {device->render(33,33,{invalid});} catch (const std::invalid_argument&) {rejected=true;}
            require(rejected,"invalid eye-dome-lighting flags were accepted");
        }
        require(device->validation_errors()==0,"eye-dome-lighting validation errors");
        std::cout<<"PASS: eye-dome lighting background, flat surfaces, silhouettes, alpha, MSAA, depth preview, IDs and depth\n";
    } catch(const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
