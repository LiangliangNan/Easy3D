// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/vulkan_offscreen.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/manipulated_frame.h>
#include <easy3d/util/initializer.h>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace easy3d;
using namespace easy3d::graphics;
void require(bool value,const char* message) { if (!value) throw std::runtime_error(message); }
template<class F> void rejects(F f) {
    bool threw=false; try { f(); } catch (const std::exception&) { threw=true; }
    require(threw,"invalid clipping input accepted");
}
unsigned hits(const FrameImage& f) { unsigned n=0; for (auto id:f.ids) n+=id.object!=UINT32_MAX; return n; }
void same_picking(const FrameImage& a,const FrameImage& b) {
    require(a.depth==b.depth,"clipping MSAA changed center depth");
    for (size_t i=0;i<a.ids.size();++i)
        require(a.ids[i].object==b.ids[i].object && a.ids[i].primitive==b.ids[i].primitive,"clipping MSAA changed primitive IDs");
}
DrawCommand quad() {
    DrawCommand d; std::vector<Vertex> v(4);
    const vec3 p[]={vec3(-.8f,-.8f,0),vec3(.8f,-.8f,0),vec3(.8f,.8f,0),vec3(-.8f,.8f,0)};
    for (unsigned i=0;i<4;++i) {
        v[i].position=p[i]; v[i].normal=normalize(vec3(float(i)*.3f,.2f,1));
        v[i].color=vec4(.3f+float(i)*.2f,.8f-float(i)*.1f,.5f,1);
        v[i].texcoord=vec2((p[i].x+.8f)/1.6f,(p[i].y+.8f)/1.6f);
    }
    d.geometry=std::make_shared<Geometry>(v,std::vector<uint32_t>{0,1,2,0,2,3},Topology::triangles,std::vector<uint32_t>{17,29});
    d.object_id=43; d.lighting=true;
    Sampler sampler; sampler.filter=TextureFilter::linear;
    d.texture=std::make_shared<Image>(2,2,std::vector<unsigned char>{255,80,20,255,20,255,80,255,80,20,255,255,255,255,255,255},sampler);
    return d;
}
void polygon_clipping(RenderDevice& device) {
    auto d=quad(); const auto baseline=device.render(65,65,{d},vec4(0));
    for (const auto& planes: {std::vector<vec4>{vec4(1,.3f,0,-.037f)},
                             std::vector<vec4>{vec4(1,0,0,-.037f),vec4(-1,0,0,.3f)}}) {
        d.clipping_planes=planes;
        const auto cut=device.render(65,65,{d},vec4(0));
        unsigned expected=0;
        for (unsigned y=0;y<65;++y) for (unsigned x=0;x<65;++x) {
            const auto i=y*65+x; const vec4 point(2*(x+.5f)/65-1,1-2*(y+.5f)/65,0,1);
            bool inside=true,edge=false;
            for (const auto& plane:planes) { const float distance=dot(plane,point); inside=inside && distance>=0; edge=edge || std::abs(distance)<1e-5f; }
            if (edge) continue;
            const bool visible=inside && baseline.ids[i].object!=UINT32_MAX;
            require((cut.ids[i].object!=UINT32_MAX)==visible,"geometric plane/polygon coverage mismatch");
            if (visible) {
                ++expected;
                require(cut.ids[i].primitive==baseline.ids[i].primitive,"clipped fan triangulation renumbered source IDs");
                require(std::abs(cut.depth[i]-baseline.depth[i])<1e-6f,"plane clipping changed surviving surface depth");
                for (unsigned c=0;c<4;++c)
                    require(std::abs(int(cut.rgba[4*i+c])-int(baseline.rgba[4*i+c]))<=2,
                            "clipped UV/color/vertex-light interpolation changed surviving colors");
            }
        }
        require(expected>50 && expected<hits(baseline),"clipping oracle did not exercise partial geometry");
        if (device.supported_sample_counts()&4) {
            const auto aa=device.render(65,65,{d},vec4(0),4);
            same_picking(cut,aa);
            unsigned new_edges=0;
            for (size_t i=0;i<aa.ids.size();++i)
                new_edges+=baseline.rgba[4*i+3]==255 && aa.rgba[4*i+3]>0 && aa.rgba[4*i+3]<255;
            require(new_edges>0,"geometric clipping boundary has no MSAA coverage");
        }
    }
    // Multiple commands can share geometry but need independent clipped versions.
    auto a=d,b=d; a.object_id=7; b.object_id=9;
    a.clipping_planes={vec4(-1,0,0,-.15f)}; b.clipping_planes={vec4(1,0,0,-.15f)};
    const auto separate=device.render(65,65,{a,b},vec4(0));
    require(separate.ids[32*65+20].object==7 && separate.ids[32*65+44].object==9 &&
            separate.ids[32*65+32].object==UINT32_MAX,"shared geometry reused the wrong per-draw clip planes");
    require(device.render(65,65,{a,b},vec4(0)).rgba==separate.rgba,"cached clipping changed repeated draws");
    d.clipping_planes={vec4(1,0,0,0)}; d.discard_clipped_primitives=true;
    require(hits(device.render(65,65,{d}))==0,"whole-triangle clipping did not reject outside controls");
    d.clipping_planes={vec4(1,0,0,1)};
    const auto kept=device.render(65,65,{d},vec4(0));
    same_picking(baseline,kept);
    auto overlay=d; overlay.overlay=true; overlay.clipping_planes={vec4(1,0,0,-100)};
    overlay.lighting=false; overlay.color=vec4(1,0,0,.5f);
    require(device.render(65,65,{overlay},vec4(0)).rgba[32*65*4+32*4+3]>0,"overlay was clipped by scene planes");
    rejects([&]{ auto invalid=d; invalid.clipping_planes={vec4(0)}; device.render(65,65,{invalid}); });
    rejects([&]{ auto invalid=d; invalid.clipping_planes={vec4(std::numeric_limits<float>::quiet_NaN())}; device.render(65,65,{invalid}); });
    rejects([&]{ auto invalid=d; invalid.clipping_planes=std::vector<vec4>(3,vec4(1,0,0,0)); device.render(65,65,{invalid}); });
    rejects([&]{ auto invalid=d; invalid.model_view(3,0)=1; device.render(65,65,{invalid}); });
    require(normalize_clipping_planes({vec4(1e-20f,0,0,0)})[0].x==1,"small finite plane normal was rejected");
}
void point_line_clipping(RenderDevice& device) {
    for (float width:{1.f,7.f}) {
        DrawCommand d; d.line_width=width; d.object_id=12;
        std::vector<Vertex> v(2); v[0].position=vec3(-.8f,-.2f,0); v[1].position=vec3(.8f,.2f,0);
        v[0].color=vec4(1,0,0,1); v[1].color=vec4(0,0,1,1);
        d.geometry=std::make_shared<Geometry>(v,std::vector<uint32_t>{},Topology::lines,std::vector<uint32_t>{88});
        d.clipping_planes={vec4(1,0,0,0)};
        const auto clipped=device.render(65,65,{d},vec4(0));
        v[0].position=vec3(0); v[0].color=vec4(.5f,0,.5f,1);
        d.geometry=std::make_shared<Geometry>(v,std::vector<uint32_t>{},Topology::lines,std::vector<uint32_t>{88}); d.clipping_planes={};
        const auto reference=device.render(65,65,{d},vec4(0));
        same_picking(clipped,reference);
        for (size_t i=0;i<reference.rgba.size();++i)
            require(std::abs(int(reference.rgba[i])-int(clipped.rgba[i]))<=2,"clipped line endpoint color/coverage mismatch");
    }
    DrawCommand p; std::vector<Vertex> v(2); v[0].position=vec3(-.4f,0,0); v[1].position=vec3(.02f,0,0);
    p.geometry=std::make_shared<Geometry>(v,std::vector<uint32_t>{},Topology::points,std::vector<uint32_t>{103,107});
    p.point_size=12; p.clipping_planes={vec4(1,0,0,0)};
    const auto points=device.render(65,65,{p},vec4(0));
    unsigned count=0;
    for (auto id:points.ids) if (id.object!=UINT32_MAX) { ++count; require(id.primitive==107,"plain point clipping did not test center/preserve ID"); }
    require(count>100 && points.ids[32*65+30].primitive==107,"large point was clipped as a surface instead of by its center");
}
void analytic_clipping(RenderDevice& device) {
    for (unsigned style=0;style<4;++style) {
        DrawCommand d; std::vector<Vertex> v(style<2 ? 1 : 2);
        v[0].position=vec3(0); v[0].normal=vec3(.3f,.2f,1);
        if (style>=2) { v[0].position=vec3(-.6f,0,0); v[1].position=vec3(.6f,.1f,.1f); }
        d.geometry=std::make_shared<Geometry>(v,std::vector<uint32_t>{},style<2 ? Topology::points : Topology::lines,
                                             std::vector<uint32_t>{71});
        d.object_id=31; d.sphere_radius=.5f; d.line_radius=.25f; d.color=vec4(1,0,0,1);
        if (style<2) d.point_style=style==0 ? PointStyle::sphere : PointStyle::surfel;
        else d.line_style=style==2 ? LineStyle::cylinder : LineStyle::cone;
        const auto baseline=device.render(65,65,{d},vec4(0));
        d.clipping_planes={vec4(1,.3f,.4f,.05f),vec4(-1,0,0,.3f)};
        const auto cut=device.render(65,65,{d},vec4(0)); unsigned expected=0;
        for (unsigned y=0;y<65;++y) for (unsigned x=0;x<65;++x) {
            const auto i=y*65+x;
            if (baseline.ids[i].object==UINT32_MAX) { require(cut.ids[i].object==UINT32_MAX,"analytic proxy leaked during clipping"); continue; }
            const vec4 surface(2*(x+.5f)/65-1,1-2*(y+.5f)/65,baseline.depth[i]*2-1,1);
            bool inside=true,edge=false;
            for (const auto& plane:d.clipping_planes) {
                const float distance=dot(plane,surface); inside=inside && distance>=0; edge=edge || std::abs(distance)<1e-5f;
            }
            if (edge) continue;
            require((cut.ids[i].object!=UINT32_MAX)==inside,"impostor clipping did not use actual surface intersections");
            if (inside) { ++expected; require(cut.ids[i].primitive==71 && cut.depth[i]==baseline.depth[i],"analytic clipping changed depth/source ID"); }
        }
        require(expected>10 && expected<hits(baseline),"analytic clipping did not retain a partial surface");
        const auto mask=style<2 ? device.supported_sample_counts(d.point_style) : device.supported_sample_counts(d.line_style);
        if (mask&4) same_picking(cut,device.render(65,65,{d},vec4(0),4));
        if (style==0) {
            v[0].position=vec3(-.2f,0,0);
            d.geometry=std::make_shared<Geometry>(v,std::vector<uint32_t>{},Topology::points);
            d.clipping_planes={vec4(1,0,0,0)};
            require(hits(device.render(65,65,{d},vec4(0)))>0,"outside sphere center incorrectly culled a surviving surface");
            d.discard_clipped_primitives=true;
            require(hits(device.render(65,65,{d},vec4(0)))==0,"whole-impostor clipping did not reject outside control");
            d.discard_clipped_primitives=false; v[0].position=vec3(0);
            d.geometry=std::make_shared<Geometry>(v,std::vector<uint32_t>{},Topology::points);
            d.clipping_planes={vec4(1,0,0,-.037f)};
            if (mask&4) {
                const auto edge=device.render(65,65,{d},vec4(0),4); unsigned partial=0;
                for (unsigned y=20;y<45;++y) for (unsigned x=32;x<36;++x) {
                    const auto alpha=edge.rgba[(y*65+x)*4+3]; partial+=alpha>0 && alpha<255;
                }
                require(partial>0,"analytic clipping boundary has no per-sample MSAA coverage");
            }
        }
    }
}
void scene_clipping(const DeviceOptions& options) {
    VulkanOffScreen viewer(181,141,options);
    viewer.camera()->setType(Camera::ORTHOGRAPHIC); viewer.camera()->setViewDirection(vec3(0,0,-1));
    viewer.camera()->setUpVector(vec3(0,1,0));
    auto mesh=std::make_shared<SurfaceMesh>();
    auto a=mesh->add_vertex(vec3(-1,-1,0)),b=mesh->add_vertex(vec3(1,-1,0)),c=mesh->add_vertex(vec3(1,1,0)),e=mesh->add_vertex(vec3(-1,1,0));
    const auto face=mesh->add_quad(a,b,c,e); viewer.add_model(mesh);
    mesh->set_manipulator(std::make_shared<Manipulator>(mesh.get())); mesh->manipulator()->frame()->translate(vec3(.3f,0,0));
    viewer.set_clipping_planes({vec4(2,0,0,-.2f)});
    require(viewer.clipping_planes()[0].x==1,"world clipping planes are not normalized");
    for (auto type:{Camera::ORTHOGRAPHIC,Camera::PERSPECTIVE}) {
        viewer.camera()->setType(type);
        auto p=viewer.camera()->projectedCoordinatesOf(vec3(-.2f,0,0));
        require(!viewer.pick(int(p.x),int(p.y)),"world clipping ignored model transformation");
        p=viewer.camera()->projectedCoordinatesOf(vec3(.5f,0,0)); const auto hit=viewer.pick(int(p.x),int(p.y));
        require(hit.face_index==face.idx() && hit.position.x>.1f,"clipped tessellation lost source face/world position");
    }
    viewer.camera()->setViewDirection(normalize(vec3(-1,.4f,-1))); viewer.camera()->showEntireScene();
    const auto p=viewer.camera()->projectedCoordinatesOf(vec3(.5f,0,0));
    require(viewer.pick(int(p.x),int(p.y)).face_index==face.idx(),"camera motion changed world clipping plane");
    const auto saved=viewer.clipping_planes();
    rejects([&]{ viewer.set_clipping_planes({vec4(0)}); }); require(viewer.clipping_planes()==saved,"failed plane setter changed state");
    viewer.set_clipping_planes({}); require(hits(viewer.capture())>0,"disabling clipping did not restore scene");
    viewer.set_clipping_planes(saved); viewer.clear_scene(); require(viewer.clipping_planes()==saved,"scene clear lost clipping configuration");
    require(viewer.device().validation_errors()==0,"scene clipping validation errors");
}
int main(int argc,char**) {
    try {
        initialize(false,false,false); DeviceOptions options; options.validation=options.synchronization_validation=argc>1;
        auto device=create_vulkan_device(options);
        polygon_clipping(*device); point_line_clipping(*device); analytic_clipping(*device);
        require(device->validation_errors()==0,"clipping device validation errors"); scene_clipping(options);
        std::cout << "PASS: halfspaces/slabs, interpolation, original IDs, whole primitives, line/point policy, analytic surface clipping, MSAA, world transforms and face picking\n";
    } catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
