// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/viewer.h>
#include <easy3d/viewer/offscreen.h>
#include <easy3d/gui/picker.h>
#include <easy3d/gui/picker_point_cloud.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/render_device.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/manipulated_frame.h>
#include <easy3d/util/initializer.h>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace easy3d;
using namespace easy3d::graphics;

// Exposes the protected query hooks so the test can compare a picker's shared
// command against a direct device sample of that exact command.
struct SharedQueryProbe : public Picker {
    explicit SharedQueryProbe(const Camera *camera) : Picker(camera) {}
    using Picker::draw_command;
    using Picker::render_device;
};

// OffScreen hides Viewer's protected query hooks; this fixture only exposes them.
struct ClippingHost : public OffScreen {
    ClippingHost(int width, int height, const DeviceOptions &options)
        : OffScreen(width, height, options) {}
    using Viewer::draw_commands;
    using Viewer::read_scene_depth;
#ifndef EASY3D_HAS_VULKAN
    using Viewer::device_query_commands;
    using Viewer::supports_device_query;
#endif
};

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
        int max_color_diff=0;
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
                    max_color_diff=std::max(max_color_diff,
                        std::abs(int(cut.rgba[4*i+c])-int(baseline.rgba[4*i+c])));
            }
        }
        // Clipping retriangulates the surviving polygon, so texture derivatives and
        // interpolated attributes can differ by a few 8-bit levels near the cut on
        // some rasterizers. Allow that rounding slack but reject a changed shader path.
        require(max_color_diff<=5,"clipped UV/color/vertex-light interpolation changed surviving colors");
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
    rejects([&]{ auto invalid=d; invalid.clipping_planes={vec4(std::numeric_limits<float>::infinity(),0,0,0)}; device.render(65,65,{invalid}); });
    rejects([&]{ auto invalid=d; invalid.clipping_planes=std::vector<vec4>(3,vec4(1,0,0,0)); device.render(65,65,{invalid}); });
    rejects([&]{ auto invalid=d; invalid.model_view(3,0)=1; device.render(65,65,{invalid}); });
    require(normalize_clipping_planes({vec4(1e-20f,0,0,0)})[0].x==1,"small finite plane normal was rejected");
    // The adapter, not just the helper, must normalize tiny finite normals before
    // uploading them to the impostor/geometry shaders.
    d.discard_clipped_primitives=false;
    d.clipping_planes={vec4(1e-20f,0,0,-0.037f*1e-20f)};
    const auto scaled=device.render(65,65,{d},vec4(0));
    d.clipping_planes={vec4(1,0,0,-0.037f)};
    const auto normalized=device.render(65,65,{d},vec4(0));
    same_picking(scaled,normalized);
    require(scaled.rgba==normalized.rgba,"tiny finite plane was not normalized by the adapter");
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
                // The impostor shader ramps its coverage down across the analytic cut and
                // alpha-to-coverage turns that ramp into a sample mask, so the resolved
                // boundary is no longer a hard per-pixel edge.
                require(partial>0,"analytic clipping boundary has no MSAA coverage");
            }
        }
    }
}

// Exercises the host query paths while the process-wide ClippingPlane (rather
// than RenderSettings) supplies the planes. This is the case that previously
// forced Viewer and PointCloudPicker onto exact native fallbacks on OpenGL.
void process_wide_clipping(ClippingHost& viewer, RenderDevice& device) {
    auto* camera = viewer.camera();
    viewer.clear_scene();
    viewer.set_clipping_planes({});
    camera->set_type(Camera::ORTHOGRAPHIC);
    camera->set_view_direction(vec3(0,0,-1));
    camera->set_up_vector(vec3(0,1,0));

    auto* plane = ClippingPlane::instance();
    plane->set_enabled(false);

    const auto projected = [&](const vec3& p) {
        const auto s = camera->projected_coordinates_of(p);
        return std::make_pair(int(s.x), int(s.y));
    };
    const auto depth_at = [&](const vec3& p) {
        const auto xy = projected(p);
        Camera snapshot(*camera);
        snapshot.projection_matrix();
        snapshot.model_view_matrix();
        snapshot.set_projection_matrix(camera->projection_matrix());
        snapshot.set_model_view_matrix(camera->model_view_matrix());
        float depth = 1;
        // A fully clipped pixel has no surviving surface, so the clear depth is
        // expected. The return flag differs by backend: OpenGL always returns true,
        // while Vulkan reports whether an object was sampled. This test uses depth.
        viewer.read_scene_depth(xy.first, xy.second, snapshot, depth);
        return depth;
    };
    const auto shared_depth_at = [&](const vec3& p) {
        const auto xy = projected(p);
        const auto commands = viewer.draw_commands(*camera, viewer.framebuffer_width(), viewer.framebuffer_height());
        return device.sample_pixel(viewer.framebuffer_width(), viewer.framebuffer_height(),
                                   commands, xy.first, xy.second).depth;
    };

    auto front = std::make_shared<SurfaceMesh>();
    auto a = front->add_vertex(vec3(-1,-1,.25f));
    auto b = front->add_vertex(vec3(1,-1,.25f));
    auto c = front->add_vertex(vec3(1,1,.25f));
    auto e = front->add_vertex(vec3(-1,1,.25f));
    front->add_quad(a,b,c,e);
    viewer.add_model(front, true, false);
    viewer.fit_screen();
    // fit_screen() also refits the clipping-plane manipulator, so set the plane
    // only after the camera has settled.
    plane->fit_scene(vec3(0),1);
    plane->set_enabled(true);
    const vec3 front_center(0,0,.25f);
    auto xy = projected(front_center);

    // With only a supported drawable in the scene, clipping must not push the
    // OpenGL viewer onto its exact native query/capture path. Vulkan has no such
    // fallback and always queries through the shared device.
#ifndef EASY3D_HAS_VULKAN
    require(viewer.supports_device_query(*camera, viewer.framebuffer_width(), viewer.framebuffer_height(), false),
            "clipped supported scene was rejected by the shared viewer query");
    std::vector<DrawCommand> probe_commands;
    require(viewer.device_query_commands(*camera, viewer.framebuffer_width(), viewer.framebuffer_height(),
                                         false, probe_commands),
            "clipped supported scene did not produce shared query commands");
    bool command_is_clipped=false;
    for (const auto& command : probe_commands)
        command_is_clipped = command_is_clipped || !command.clipping_planes.empty();
    require(command_is_clipped,"shared query commands omitted the process-wide clipping plane");
#endif

    require(!viewer.pick(xy.first,xy.second),"Viewer::pick ignored a process-wide clipping plane");
    require(depth_at(front_center)==1,"read_scene_depth ignored a process-wide clipping plane");
    require(std::abs(depth_at(front_center)-shared_depth_at(front_center))<1e-6f,
            "read_scene_depth disagreed with the shared clipped command");

    // Capture must also honor the process-wide plane on the same shared path.
    const auto clipped_capture = viewer.capture();
    const auto capture_index = size_t(xy.second)*clipped_capture.width + size_t(xy.first);
    require(clipped_capture.ids[capture_index].object==UINT32_MAX && clipped_capture.depth[capture_index]==1,
            "capture ignored a process-wide clipping plane");
    plane->set_enabled(false);
    const auto intact_capture = viewer.capture();
    require(intact_capture.ids[capture_index].object!=UINT32_MAX,
            "disabling the process-wide plane did not restore capture");
    plane->set_enabled(true);

    auto rear = std::make_shared<SurfaceMesh>();
    auto ra = rear->add_vertex(vec3(-1,-1,-.25f));
    auto rb = rear->add_vertex(vec3(1,-1,-.25f));
    auto rc = rear->add_vertex(vec3(1,1,-.25f));
    auto re = rear->add_vertex(vec3(-1,1,-.25f));
    const auto rear_face = rear->add_quad(ra,rb,rc,re);
    viewer.add_model(rear, true, false);
    require(viewer.pick(xy.first,xy.second).face_index==rear_face.idx(),
            "clipped Viewer::pick lost the surviving surface");
    require(depth_at(front_center)<1,"read_scene_depth did not see the surface behind the clip");
    require(std::abs(depth_at(front_center)-shared_depth_at(front_center))<1e-6f,
            "read_scene_depth disagreed with the shared clipped command after the cut");

    // A non-opaque drawable is outside the OpenGL shared-query contract and must
    // keep the Viewer exact pass while the process-wide plane is active. Vulkan
    // has no exact fallback, so it must still resolve the nearest surface.
    auto unsupported = std::make_shared<SurfaceMesh>();
    auto ua = unsupported->add_vertex(vec3(.4f,.4f,-.15f));
    auto ub = unsupported->add_vertex(vec3(1, .4f,-.15f));
    auto uc = unsupported->add_vertex(vec3(1, 1,-.15f));
    auto ue = unsupported->add_vertex(vec3(.4f,1,-.15f));
    unsupported->add_quad(ua,ub,uc,ue);
    viewer.add_model(unsupported, true, false);
    auto* unsupported_faces = unsupported->renderer()->get_triangles_drawable("faces");
    unsupported_faces->set_uniform_coloring(vec4(0,1,0,.5f));
    xy = projected(vec3(.7f,.7f,-.15f));
#ifndef EASY3D_HAS_VULKAN
    require(!viewer.supports_device_query(*camera, viewer.framebuffer_width(), viewer.framebuffer_height(), false),
            "unsupported translucent drawable incorrectly entered the shared viewer query");
#endif
    const auto fallback_hit = viewer.pick(xy.first,xy.second);
    require(fallback_hit.drawable.get()==unsupported_faces,
            "unsupported translucent drawable lost its nearest-surface query");

    // With only supported drawables again, compare PointCloudPicker against a
    // direct sample of the command it submits through the shared device.
    viewer.clear_scene();
    auto cloud = std::make_shared<PointCloud>();
    const auto near_point = cloud->add_vertex(vec3(0,0,.25f));
    const auto far_point = cloud->add_vertex(vec3(0,0,-.25f));
    auto normals = cloud->add_vertex_property<vec3>("v:normal", vec3(0,0,1));
    normals[near_point] = vec3(0,0,1);
    normals[far_point] = vec3(0,0,1);
    viewer.add_model(cloud, true, false);
    auto* drawable = cloud->renderer()->get_points_drawable("vertices");
    std::shared_ptr<Drawable> drawable_owner;
    for (const auto& d : cloud->renderer()->points_drawables())
        if (d.get()==drawable)
            drawable_owner=d;
    require(drawable_owner.get()==drawable,"point cloud lost its vertices drawable");
    drawable->set_point_size(12);
    viewer.fit_screen();
    plane->fit_scene(vec3(0),1);
    plane->set_enabled(true);
    xy = projected(cloud->position(far_point));

    PointCloudPicker picker(camera);
    SharedQueryProbe probe(camera);
    for (auto style : {PointsDrawable::SPHERE, PointsDrawable::SURFEL}) {
        drawable->set_impostor_type(style);
        const auto picked = picker.pick_vertex(cloud.get(), xy.first, xy.second);
        const auto command = probe.draw_command(drawable_owner, 0);
        const auto pixel = probe.render_device().sample_pixel(
            camera->screen_width(), camera->screen_height(), {command}, xy.first, xy.second);
        const auto& ids = command.geometry->point_data()->source_ids();
        require(pixel.id.object==0 && pixel.id.primitive<ids.size(),
                "shared clipped impostor command returned no point");
        require(picked==PointCloud::Vertex(ids[pixel.id.primitive]),
                "PointCloudPicker disagreed with the shared clipped impostor command");
        require(picked==far_point,"clipped impostor pick selected the removed surface");
    }
    plane->set_enabled(false);
    require(device.validation_errors()==0,"process-wide clipping validation errors");
}
void scene_clipping(ClippingHost& viewer, RenderDevice& device) {
    viewer.clear_scene();
    viewer.set_clipping_planes({});
    ClippingPlane::instance()->set_enabled(false);
    viewer.camera()->set_type(Camera::ORTHOGRAPHIC); viewer.camera()->set_view_direction(vec3(0,0,-1));
    viewer.camera()->set_up_vector(vec3(0,1,0));
    auto mesh=std::make_shared<SurfaceMesh>();
    auto a=mesh->add_vertex(vec3(-1,-1,0)),b=mesh->add_vertex(vec3(1,-1,0)),c=mesh->add_vertex(vec3(1,1,0)),e=mesh->add_vertex(vec3(-1,1,0));
    const auto face=mesh->add_quad(a,b,c,e); viewer.add_model(mesh);
    mesh->set_manipulator(std::make_shared<Manipulator>(mesh.get())); mesh->manipulator()->frame()->translate(vec3(.3f,0,0));
    viewer.set_clipping_planes({vec4(2,0,0,-.2f)});
    require(viewer.clipping_planes()[0].x==1,"world clipping planes are not normalized");
    for (auto type:{Camera::ORTHOGRAPHIC,Camera::PERSPECTIVE}) {
        viewer.camera()->set_type(type);
        auto p=viewer.camera()->projected_coordinates_of(vec3(-.2f,0,0));
        require(!viewer.pick(int(p.x),int(p.y)),"world clipping ignored model transformation");
        p=viewer.camera()->projected_coordinates_of(vec3(.5f,0,0)); const auto hit=viewer.pick(int(p.x),int(p.y));
        require(hit.face_index==face.idx() && hit.position.x>.1f,"clipped tessellation lost source face/world position");
    }
    viewer.camera()->set_view_direction(normalize(vec3(-1,.4f,-1))); viewer.camera()->show_entire_scene();
    const auto p=viewer.camera()->projected_coordinates_of(vec3(.5f,0,0));
    require(viewer.pick(int(p.x),int(p.y)).face_index==face.idx(),"camera motion changed world clipping plane");
    const auto saved=viewer.clipping_planes();
    rejects([&]{ viewer.set_clipping_planes({vec4(0)}); }); require(viewer.clipping_planes()==saved,"failed plane setter changed state");
    viewer.set_clipping_planes({}); require(hits(viewer.capture())>0,"disabling clipping did not restore scene");
    viewer.set_clipping_planes(saved); viewer.clear_scene(); require(viewer.clipping_planes()==saved,"scene clear lost clipping configuration");
    require(device.validation_errors()==0,"scene clipping validation errors");
}
int main(int argc,char**) {
    try {
        initialize(false,false,false);
        (void)argc;
        DeviceOptions options;
#ifdef EASY3D_HAS_VULKAN
        options.validation=options.sync_validation=argc>1;
#endif
        // The host owns the OpenGL context that the shared device borrows. Vulkan
        // builds need no presentation surface, but the same setup serves both.
        ClippingHost viewer(181,141,options);
        auto device=create_device(options);
        polygon_clipping(*device); point_line_clipping(*device); analytic_clipping(*device);
        require(device->validation_errors()==0,"clipping device validation errors");
        scene_clipping(viewer,*device);
        process_wide_clipping(viewer,*device);
        std::cout << "PASS: halfspaces/slabs, interpolation, original IDs, whole primitives, line/point policy, analytic surface clipping, MSAA, world transforms, face picking, process-wide clipping and shared impostor picks\n";
    } catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
