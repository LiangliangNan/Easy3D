// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/viewer.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/manipulated_frame.h>
#include <easy3d/renderer/texture.h>
#include <easy3d/util/initializer.h>
#include <easy3d/util/setting.h>
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace easy3d;
void require(bool value,const char* message) { if (!value) throw std::runtime_error(message); }
void same_picking(const graphics::FrameImage& a,const graphics::FrameImage& b) {
    require(a.depth==b.depth && a.ids.size()==b.ids.size(),"selection changed depth");
    for (size_t i=0;i<a.ids.size();++i)
        require(a.ids[i].object==b.ids[i].object && a.ids[i].primitive==b.ids[i].primitive,"selection changed IDs");
}
std::shared_ptr<SurfaceMesh> quad(float z=0) {
    auto mesh=std::make_shared<SurfaceMesh>();
    auto a=mesh->add_vertex(vec3(-1,-1,z)),b=mesh->add_vertex(vec3(1,-1,z));
    auto c=mesh->add_vertex(vec3(1,1,z)),d=mesh->add_vertex(vec3(-1,1,z));
    mesh->add_quad(a,b,c,d); return mesh;
}
void shader_highlights(const graphics::DeviceOptions& options) {
    using namespace graphics;
    auto device=create_vulkan_device(options);
    for (int style=0;style<8;++style) {
        DrawCommand draw; draw.object_id=77; draw.lighting=true;
        draw.texture=std::make_shared<Image>(1,1,std::vector<unsigned char>{0,200,80,255});
        std::vector<Vertex> v(style==0 ? 3 : style<=3 ? 1 : 2);
        if (style==0) { v[0].position=vec3(-.6f,-.4f,0); v[1].position=vec3(.6f,-.4f,0); v[2].position=vec3(0,.6f,0); }
        else if (style>=4) { v[0].position=vec3(-.4f,0,0); v[1].position=vec3(.4f,.1f,0); }
        else v[0].position=vec3(0);
        const auto topology=style==0 ? Topology::triangles : style<=3 ? Topology::points : Topology::lines;
        if (style<=3 && style>0) {
            draw.point_size=12; draw.sphere_radius=.2f;
            draw.point_style=style==1 ? PointStyle::plain : style==2 ? PointStyle::sphere : PointStyle::surfel;
        }
        if (style>=4) {
            draw.line_width=style==5 ? 7 : 1; draw.line_radius=.13f;
            draw.line_style=style==6 ? LineStyle::cylinder : style==7 ? LineStyle::cone : LineStyle::plain;
        }
        const auto mask=style>=4 ? device->supported_sample_counts(draw.line_style) : device->supported_sample_counts(draw.point_style);
        for (unsigned count:{1u,4u}) {
            if (!(mask&count)) continue;
            for (auto& p:v) p.highlight=vec4(0);
            draw.geometry=std::make_shared<Geometry>(v,std::vector<uint32_t>{},topology);
            const auto baseline=device->render(65,65,{draw},vec4(0),count);
            for (auto& p:v) p.highlight=vec4(1,0,0,1);
            draw.geometry=std::make_shared<Geometry>(v,std::vector<uint32_t>{},topology);
            const auto selected=device->render(65,65,{draw},vec4(0),count);
            same_picking(baseline,selected);
            unsigned hits=0;
            for (size_t i=0;i<baseline.ids.size();++i) {
                require(baseline.rgba[4*i+3]==selected.rgba[4*i+3],"highlight changed MSAA coverage/alpha");
                if (baseline.ids[i].object==77) {
                    ++hits;
                    require(selected.rgba[4*i]>0 && selected.rgba[4*i+1]==0 && selected.rgba[4*i+2]==0,
                            "highlight failed after texture/lighting on a native style");
                }
            }
            require(hits>0,"highlight style test had no hits");
        }
    }
    require(device->validation_errors()==0,"highlight pipeline validation errors");
}
void scene_picking(const graphics::DeviceOptions& options) {
    VulkanOffScreen viewer(161,121,options);
    viewer.camera()->setType(Camera::ORTHOGRAPHIC);
    viewer.camera()->setViewDirection(vec3(0,0,-1)); viewer.camera()->setUpVector(vec3(0,1,0));
    viewer.camera()->showEntireScene(); // Position the eye along the explicitly chosen viewing direction.
    auto back=quad(-.5f),front=quad(0);
    viewer.add_model(back); viewer.add_model(front);
    const int x=viewer.width()/2,y=viewer.height()/2;
    const auto hit=viewer.pick(x,y);
    require(hit && hit.model==front && hit.drawable==front->renderer()->triangles_drawables()[0] && hit.face_index==0,
            "visible model/drawable/original polygon picking failed");
    require(std::abs(hit.position.z)<1e-4f && hit.depth<1,"picked world point is not on the visible face");
    const auto baseline=viewer.capture();
    front->renderer()->set_selected(true);
    const auto selected=viewer.capture();
    require(selected.rgba!=baseline.rgba,"model selection did not change color"); same_picking(baseline,selected);
    auto* faces=front->renderer()->get_triangles_drawable("faces");
    faces->set_highlight(true); faces->set_highlight_range({0,0});
    const auto highlighted=viewer.capture();
    require(highlighted.rgba!=selected.rgba,"primitive-range highlight did not change selected color");
    same_picking(selected,highlighted);
    require(viewer.pick(x,y).face_index==0,"highlight changed source picking");
    faces->set_highlight(false); front->renderer()->set_selected(false);
    require(viewer.capture().rgba==baseline.rgba,"clearing selection did not restore color");
    front->renderer()->set_visible(false);
    require(viewer.pick(x,y).model==back,"hidden model occluded picking");
    front->renderer()->set_visible(true);
    front->set_manipulator(std::make_shared<Manipulator>(front.get()));
    front->manipulator()->frame()->translate(vec3(0,0,.15f));
    require(std::abs(viewer.pick(x,y).position.z-.15f)<1e-4f,"manipulated model picked point is not in world coordinates");
    front->manipulator()->reset();
    faces->set_visible(false);
    require(viewer.pick(x,y).model==back,"hidden drawable occluded picking"); faces->set_visible(true);
    require(!viewer.pick(-1,y) && !viewer.pick(viewer.width(),y) && !viewer.pick(0,0),"outside/background pixel produced a hit");
    graphics::DrawCommand overlay;
    std::vector<graphics::Vertex> screen(3);
    screen[0].position=vec3(-1,-1,0); screen[1].position=vec3(3,-1,0); screen[2].position=vec3(-1,3,0);
    overlay.geometry=std::make_shared<graphics::Geometry>(screen); overlay.color=vec4(1,0,0,.8f);
    viewer.set_overlays({overlay});
    require(viewer.pick(x,y).model==front,"overlay hid source picking"); viewer.set_overlays({});
    // Two drawables on one model need a draw-local source ID, independent of raw
    // frame model IDs and of each other's primitive numbering.
    auto* custom=front->renderer()->add_triangles_drawable("custom");
    custom->update_vertex_buffer({vec3(-.5f,-.5f,.2f),vec3(.5f,-.5f,.2f),vec3(0,.5f,.2f)});
    auto custom_hit=viewer.pick(x,y);
    require(custom_hit.drawable.get()==custom && custom_hit.model==front && custom_hit.face_index==-1,
            "custom draw-local source was confused with default face provenance");
    custom->set_source_faces({0});
    require(viewer.pick(x,y).face_index==0,"explicit custom provenance was ignored");
    custom->update_vertex_buffer({vec3(-.5f,-.5f,.2f),vec3(.5f,-.5f,.2f),vec3(0,.5f,.2f)});
    require(viewer.pick(x,y).face_index==-1,"manual upload retained stale face provenance"); custom->set_visible(false);
    auto standalone=std::make_shared<PointsDrawable>("standalone");
    viewer.add_drawable(standalone); standalone->update_vertex_buffer({vec3(0,0,.4f)});
    standalone->set_impostor_type(PointsDrawable::SPHERE); standalone->set_point_size(16);
    auto point=viewer.pick(x,y);
    require(point && !point.model && point.drawable==standalone && point.face_index==-1 && point.position.z>.4f,
            "standalone sphere source/analytic world-point picking failed");
    viewer.delete_drawable(standalone.get());
    // Retained results own their sources even after removal and object-ID shifts.
    viewer.delete_model(front.get());
    require(hit.model==front && hit.drawable && viewer.pick(x,y).model==back,"removal invalidated a result or remapped stale IDs");
    viewer.resize(181,141);
    require(viewer.pick(90,70).model==back,"picking failed after resize");
    viewer.clear_scene(); require(!viewer.pick(90,70),"cleared scene retained a picking source");
    // A deleted face leaves a handle gap. Rendered triangle zero must map to the
    // remaining original face, not to its compacted primitive number.
    auto mesh=std::make_shared<SurfaceMesh>();
    auto a=mesh->add_vertex(vec3(-2,-1,0)),b=mesh->add_vertex(vec3(-1,-1,0)),c=mesh->add_vertex(vec3(-1,1,0));
    const auto removed=mesh->add_triangle(a,b,c);
    auto d=mesh->add_vertex(vec3(-1,-1,0)),e=mesh->add_vertex(vec3(1,-1,0));
    auto f=mesh->add_vertex(vec3(1,1,0)),g=mesh->add_vertex(vec3(-1,1,0));
    const auto remaining=mesh->add_quad(d,e,f,g); mesh->delete_face(removed);
    viewer.add_model(mesh); viewer.fit_screen();
    auto p=viewer.camera()->projectedCoordinatesOf(vec3(.1f,.1f,0));
    const auto polygon=viewer.pick(int(p.x),int(p.y));
    require(polygon.face_index==remaining.idx(),"polygon/deleted-handle source mapping failed");
    auto* drawable=mesh->renderer()->get_triangles_drawable("faces");
    auto uv=mesh->vertex_property<vec2>("v:texcoord",vec2(.5f));
    std::unique_ptr<Texture> texture(Texture::create_cpu({0,255,0,255},1,1,4));
    drawable->set_texture_coloring(State::VERTEX,"v:texcoord",texture.get()); mesh->renderer()->update();
    require(viewer.pick(int(p.x),int(p.y)).face_index==remaining.idx(),"textured extraction lost polygon provenance");
    drawable->set_smooth_shading(true);
    require(viewer.pick(int(p.x),int(p.y)).face_index==remaining.idx(),"smooth shading lost polygon provenance");
    auto halfedge_uv=mesh->halfedge_property<vec2>("h:texcoord",vec2(.5f));
    drawable->set_texture_coloring(State::HALFEDGE,"h:texcoord",texture.get()); mesh->renderer()->update();
    require(viewer.pick(int(p.x),int(p.y)).face_index==remaining.idx(),"halfedge UV extraction lost provenance");
    auto face_color=mesh->face_property<vec3>("f:color",vec3(0,1,0));
    drawable->set_property_coloring(State::FACE,"f:color"); mesh->renderer()->update();
    require(viewer.pick(int(p.x),int(p.y)).face_index==remaining.idx(),"face colors lost provenance");
    auto scalar=mesh->face_property<float>("f:value",2);
    drawable->set_scalar_coloring(State::FACE,"f:value",texture.get()); mesh->renderer()->update();
    require(viewer.pick(int(p.x),int(p.y)).face_index==remaining.idx(),"face scalar extraction lost provenance");
    drawable->set_texture_coloring(State::VERTEX,"v:texcoord",texture.get()); mesh->renderer()->update();
    drawable->set_lighting(false);
    drawable->set_selected(true);
    setting::highlight_color=vec4(1,0,0,1); auto red=viewer.capture();
    const auto pixel=size_t(int(p.y))*red.width+int(p.x);
    require(std::abs(int(red.rgba[4*pixel])-153)<=1 && std::abs(int(red.rgba[4*pixel+1])-102)<=1,
            "selection tint is not mixed after texturing with strength 0.6");
    setting::highlight_color=vec4(0,0,1,1); auto blue=viewer.capture();
    require(red.rgba!=blue.rgba,"highlight color did not invalidate cached geometry"); same_picking(red,blue);
    if (viewer.device().supported_sample_counts()&4) same_picking(blue,viewer.capture(4));
    require(viewer.device().validation_errors()==0,"source picking validation errors");
}
int main(int argc,char**) {
    try {
        initialize(false,false,false);
        graphics::DeviceOptions options; options.validation=options.synchronization_validation=argc>1;
        shader_highlights(options); scene_picking(options);
        std::cout << "PASS: selection on all native styles, texture/lighting and MSAA invariance, model/drawable/polygon picking, provenance, world points, visibility, overlays, lifetime, resize\n";
    } catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
