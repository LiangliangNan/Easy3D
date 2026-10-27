// SPDX-License-Identifier: GPL-3.0-or-later
#include "obj_fixture.h"
#include <easy3d/renderer/obj_materials.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/texture.h>
#include <easy3d/viewer/vulkan_offscreen.h>
#include <easy3d/util/initializer.h>
#include <cmath>
#include <iostream>
using namespace easy3d;
void require(bool value,const char* message) { if (!value) throw std::runtime_error(message); }
template<class F> void rejects(F call) {
    bool threw=false; try { call(); } catch (const std::exception&) { threw=true; }
    require(threw,"invalid OBJ operation accepted");
}
std::size_t pixel(const Camera* cam,const graphics::FrameImage& image,const vec3& world) {
    const auto p=cam->projectedCoordinatesOf(world);
    const int x=int(p.x),y=int(p.y);
    require(x>=0 && y>=0 && x<int(image.width) && y<int(image.height),"OBJ probe is outside viewport");
    return std::size_t(y)*image.width+x;
}
void probe(const Camera* cam,const graphics::FrameImage& image,const vec3& world,int r,int g,int b,bool hit=true) {
    const auto i=pixel(cam,image,world);
    require(std::abs(int(image.rgba[4*i])-r)<3 && std::abs(int(image.rgba[4*i+1])-g)<3 &&
            std::abs(int(image.rgba[4*i+2])-b)<3,"OBJ material/UV rendered color mismatch");
    require((image.ids[i].object!=UINT32_MAX)==hit,"OBJ probe picking mismatch");
}
int main(int argc,char**) {
    try {
        initialize(false,false,false);
        const std::string dir="vulkan-obj-material-fixture";
        const auto file=obj_fixture::create(dir);
        auto asset=load_obj_materials(file);
        require(asset.mesh->n_faces()==6 && asset.parts.size()==4,"material groups/incomplete UV split/negative indices lost faces");
        std::size_t textured=0;
        for (const auto& part:asset.parts) if (!part.texture_file.empty()) {
            ++textured;
            require(file_system::is_absolute_path(part.texture_file),"OBJ material retained a working-directory-dependent texture path");
            require(part.indices.size()==6 && part.positions.size()==6,"OBJ UV seam was merged across shared positions");
        }
        require(textured==1,"diffuse map selection/fallback mismatch");
        const auto compatibility=load_obj_materials(dir+"/compat.obj");
        require(compatibility.parts.size()==2 && !compatibility.parts[0].texture_file.empty() &&
                compatibility.parts[0].texture_file==compatibility.parts[1].texture_file,
                "legacy ambient/specular color-map fallback was lost");
        rejects([&]{ asset.create_drawables(); });
        graphics::DeviceOptions options; options.validation=options.synchronization_validation=argc>1;
        VulkanOffScreen renderer(321,221,options);
        renderer.camera()->setType(Camera::ORTHOGRAPHIC);
        renderer.camera()->setViewDirection(vec3(0,0,-1)); renderer.camera()->setUpVector(vec3(0,1,0));
        require(renderer.add_model(asset.mesh,false)!=nullptr,"could not attach OBJ mesh");
        asset.create_drawables();
        const auto count=asset.mesh->renderer()->triangles_drawables().size();
        rejects([&]{ asset.create_drawables(); });
        require(asset.mesh->renderer()->triangles_drawables().size()==count,"duplicate part check mutated scene");
        for (const auto& part:asset.mesh->renderer()->triangles_drawables()) {
            require(part->has_cpu_buffers(),"OBJ material created GL buffers without a context");
            part->set_lighting(false);
            if (part->texture()) require(part->texture()->id()==0,"OBJ material created a GL texture in CPU mode");
        }
        const auto single=renderer.capture();
        probe(renderer.camera(),single,vec3(-1.5f,.5f,0),0,0,255);
        probe(renderer.camera(),single,vec3(-2.5f,1.5f,0),255,0,0);
        probe(renderer.camera(),single,vec3(2,1,0),0,255,0);
        probe(renderer.camera(),single,vec3(-2,-2,0),255,255,0);
        probe(renderer.camera(),single,vec3(2,-2,0),255,0,255);
        probe(renderer.camera(),single,vec3(4.5f,1,0),0,255,0);
        probe(renderer.camera(),single,vec3(5.5f,1,0),255,255,255,false);
        const std::vector<vec3> face_probes={vec3(-1.5f,.5f,0),vec3(-2.5f,1.5f,0),vec3(2,1,0),
            vec3(-2,-2,0),vec3(2,-2,0),vec3(4.5f,1,0)};
        for (std::size_t face=0;face<face_probes.size();++face) {
            const auto point=renderer.camera()->projectedCoordinatesOf(face_probes[face]);
            const auto picked=renderer.pick(int(point.x),int(point.y));
            require(picked.face_index==int(face),"OBJ material part did not retain its original face index");
        }
        asset.mesh->renderer()->update();
        require(renderer.capture().rgba==single.rgba,"Renderer::update replaced material subsets with the entire mesh");
        if (renderer.device().supported_sample_counts()&4) {
            const auto msaa=renderer.capture(4);
            require(msaa.depth==single.depth,"OBJ MSAA changed picking depth");
            for (std::size_t i=0;i<single.ids.size();++i)
                require(single.ids[i].object==msaa.ids[i].object && single.ids[i].primitive==msaa.ids[i].primitive,
                        "OBJ MSAA changed picking IDs");
        }
        renderer.resize(221,181); require(renderer.capture().width==221,"OBJ scene resize failed");
        rejects([&]{ load_obj_materials(dir+"/invalid.obj"); });
        rejects([&]{ load_obj_materials(dir+"/empty.obj"); });
        rejects([&]{ load_obj_materials(dir+"/missing.obj"); });
        auto plain=load_obj_materials(dir+"/plain.obj");
        require(plain.parts.size()==1 && plain.parts[0].indices.size()==3 && plain.parts[0].texture_file.empty(),
                "OBJ without MTL/UVs did not produce default geometry");
        renderer.clear_scene(); renderer.add_model(plain.mesh,false); plain.create_drawables();
        require(renderer.capture().width==221,"OBJ scene reload failed");
        require(renderer.device().validation_errors()==0,"OBJ material rendering validation errors");
        std::cout << "PASS: OBJ material parts, UV seams, diffuse maps/fallbacks, concave polygons, negative indices, CPU textures, update/reload/resize, MSAA and picking\n";
    } catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
