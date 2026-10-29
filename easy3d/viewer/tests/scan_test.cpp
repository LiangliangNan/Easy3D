// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/vulkan_offscreen.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/manipulated_frame.h>
#include <easy3d/util/initializer.h>
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace easy3d;
void require(bool ok,const char* text) { if (!ok) throw std::runtime_error(text); }
std::vector<vec3> check_scan(VulkanOffScreen& viewer,unsigned samples) {
    const auto reference=viewer.capture(samples);
    const auto mvp=viewer.camera()->modelViewProjectionMatrix();
    const auto points=viewer.scan_visible_points(samples);
    size_t k=0;
    for (size_t i=0;i<reference.ids.size();++i) {
        if (reference.ids[i].object==UINT32_MAX || reference.depth[i]>=1) continue;
        require(k<points.size(),"scan dropped a visible depth sample");
        const auto clip=mvp*vec4(points[k++],1);
        const float x=(clip.x/clip.w+1)*.5f*reference.width,y=(1-clip.y/clip.w)*.5f*reference.height;
        require(std::abs(x-float(i%reference.width)-.5f)<.002f &&
                std::abs(y-float(i/reference.width)-.5f)<.002f,"scan point is not at the framebuffer pixel center");
        require(std::abs(.5f*(clip.z/clip.w+1)-reference.depth[i])<1e-5f,"scan used incorrect world depth");
    }
    require(k==points.size(),"background or overlay produced scan points");
    const auto after=viewer.camera()->modelViewProjectionMatrix();
    for (unsigned i=0;i<16;++i) require(std::abs(after[i]-mvp[i])<1e-6f,"scan changed the camera");
    return points;
}
int main(int argc,char**) {
    try {
        initialize(false,false,false);
        graphics::DeviceOptions options; options.validation=options.synchronization_validation=argc>1;
        VulkanOffScreen viewer(161,127,options);
        require(viewer.scan_visible_points().empty(),"empty scene scanned background");
        auto front=std::make_shared<SurfaceMesh>();
        auto a=front->add_vertex(vec3(-1,-1,-.3f)),b=front->add_vertex(vec3(1,-1,.1f));
        auto c=front->add_vertex(vec3(1,1,.3f)),d=front->add_vertex(vec3(-1,1,-.1f));
        front->add_quad(a,b,c,d); viewer.add_model(front);
        front->set_manipulator(std::make_shared<Manipulator>(front.get()));
        front->manipulator()->frame()->translate(vec3(.2f,-.1f,.3f));
        auto hidden=std::make_shared<SurfaceMesh>();
        auto ha=hidden->add_vertex(vec3(-.2f,-.2f,-.8f)),hb=hidden->add_vertex(vec3(.2f,-.2f,-.8f));
        auto hc=hidden->add_vertex(vec3(.2f,.2f,-.8f)),hd=hidden->add_vertex(vec3(-.2f,.2f,-.8f));
        hidden->add_quad(ha,hb,hc,hd); viewer.add_model(hidden); hidden->renderer()->set_visible(false);
        viewer.camera()->setUpVector(vec3(0,1,0)); viewer.camera()->setViewDirection(vec3(0,0,-1)); viewer.fit_screen();
        for (auto projection:{Camera::PERSPECTIVE,Camera::ORTHOGRAPHIC}) {
            viewer.camera()->setType(projection);
            const auto baseline=check_scan(viewer,1); require(baseline.size()>100,"test surface scan is empty");
            hidden->renderer()->set_visible(true);
            require(check_scan(viewer,1)==baseline,"occluded geometry produced scan points");
            hidden->renderer()->set_visible(false);
            for (const auto& p:baseline) require(std::abs(p.z-.2f*p.x-.1f*p.y-.27f)<1e-4f,"transformed scan missed the tilted plane");
            if (viewer.device().supported_sample_counts()&4)
                require(check_scan(viewer,4)==baseline,"MSAA changed center-sample scan positions");
            std::vector<graphics::Vertex> corners(4);
            corners[0].position=vec3(-1,-1,0); corners[1].position=vec3(1,-1,0);
            corners[2].position=vec3(1,1,0); corners[3].position=vec3(-1,1,0);
            graphics::DrawCommand overlay; overlay.geometry=std::make_shared<graphics::Geometry>(corners,std::vector<uint32_t>{0,1,2,0,2,3});
            overlay.color=vec4(1,0,0,.5f);
            for (bool depth_test:{false,true}) {
                overlay.overlay_depth_test=depth_test; viewer.set_overlays({overlay});
                require(check_scan(viewer,1)==baseline,"transparent guide/HUD altered scanning");
            }
            viewer.set_overlays({}); viewer.set_clipping_planes({vec4(1,0,0,-.2f)});
            const auto clipped=check_scan(viewer,1);
            require(!clipped.empty() && clipped.size()<baseline.size(),"scan did not honor clipping");
            for (const auto& p:clipped) require(p.x>=.2f-1e-4f,"scan returned clipped surface");
            viewer.set_clipping_planes({});
        }
        viewer.resize(137,99); require(!check_scan(viewer,1).empty(),"resized scan failed");
        auto custom_projection=viewer.camera()->projectionMatrix(),custom_view=viewer.camera()->modelViewMatrix();
        custom_projection(0,2)+=.13f; custom_view(0,3)+=.1f;
        viewer.camera()->set_projection_matrix(custom_projection); viewer.camera()->set_modelview_matrix(custom_view);
        require(!check_scan(viewer,1).empty(),"custom camera matrices were lost during scanning");
        viewer.clear_scene();
        auto points=std::make_shared<PointsDrawable>(); viewer.add_drawable(points);
        points->update_vertex_buffer({vec3(-.3f,0,0),vec3(.3f,0,0)});
        points->update_normal_buffer({vec3(0,0,1),vec3(0,0,1)}); points->set_point_size(24); viewer.fit_screen();
        viewer.camera()->setPivotPoint(vec3(.1f,0,.15f));
        for (auto style:{PointsDrawable::PLAIN,PointsDrawable::SPHERE,PointsDrawable::SURFEL}) {
            points->set_impostor_type(style);
            const auto scan=check_scan(viewer,1); require(scan.size()>20,"point-style scan is empty");
            const float radius=points->point_size()*viewer.camera()->pixelGLRatio(viewer.camera()->pivotPoint())*.5f;
            for (const auto& p:scan) {
                if (style==PointsDrawable::SPHERE) {
                    const float distance=std::min((p-vec3(-.3f,0,0)).length(),(p-vec3(.3f,0,0)).length());
                    require(std::abs(distance-radius)<1e-4f,"sphere scan sampled proxy rather than analytic surface");
                } else require(std::abs(p.z)<1e-4f,"plain/surfel scan missed its plane");
            }
            const auto native_style=style==PointsDrawable::PLAIN?graphics::PointStyle::plain:
                                    style==PointsDrawable::SPHERE?graphics::PointStyle::sphere:graphics::PointStyle::surfel;
            if (viewer.device().supported_sample_counts(native_style)&4) require(check_scan(viewer,4)==scan,"point MSAA altered scan");
        }
        viewer.clear_scene();
        auto line=std::make_shared<LinesDrawable>(); viewer.add_drawable(line);
        line->update_vertex_buffer({vec3(-.4f,0,0),vec3(.4f,0,0)}); line->set_line_width(24); viewer.fit_screen();
        for (auto style:{LinesDrawable::CYLINDER,LinesDrawable::CONE}) {
            line->set_impostor_type(style);
            const auto scan=check_scan(viewer,1); require(scan.size()>20,"line impostor scan is empty");
            const float radius=24*viewer.camera()->pixelGLRatio(viewer.camera()->pivotPoint())*.5f;
            for (const auto& p:scan) {
                const float r=style==LinesDrawable::CONE?radius*(.4f-p.x)/.8f:radius;
                require(p.x>=-.4001f && p.x<=.4001f,"line scan is outside segment caps");
                require(std::abs(std::sqrt(p.y*p.y+p.z*p.z)-r)<1e-4f ||
                        std::abs(p.x+.4f)<1e-4f || std::abs(p.x-.4f)<1e-4f,"line scan sampled proxy geometry");
            }
        }
        bool rejected=false;
        try { viewer.scan_visible_points(3); } catch(const std::invalid_argument&) { rejected=true; }
        require(rejected,"invalid scan sample count accepted");
        require(viewer.device().validation_errors()==0,"scan validation errors");
        std::cout << "PASS: native world scans, pixel centers, projection/transform/clipping, background/overlays, MSAA, analytic surfaces and resize\n";
    } catch(const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
