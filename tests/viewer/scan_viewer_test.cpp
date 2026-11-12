#include "test_window.h"
// SPDX-License-Identifier: GPL-3.0-or-later
#include <tutorials/Tutorial_405_VirtualScanner/viewer.h>
#include <tests/native_tutorials/options.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/util/initializer.h>
#include <cmath>
#include <algorithm>
#include <iostream>
#include <stdexcept>
using namespace easy3d;
void require(bool ok,const char* text) { if (!ok) throw std::runtime_error(text); }
void frame(Viewer& viewer) {
    for (unsigned i=0;i<100;++i) {
        Viewer::poll_events(); if (viewer.draw_frame()) return;
        glfwWaitEventsTimeout(.01);
    }
    throw std::runtime_error("scanner viewer did not present");
}
void same_camera(const Camera& a,const Camera& b) {
    const auto m=a.model_view_projection_matrix(),n=b.model_view_projection_matrix();
    for (unsigned i=0;i<16;++i) require(std::abs(m[i]-n[i])<1e-6f,"scan changed the acquisition camera");
    require(a.screen_width()==b.screen_width() && a.screen_height()==b.screen_height(),"scan changed framebuffer dimensions");
    require(a.pivot_point()==b.pivot_point(),"scan changed the interaction pivot");
    require(a.scene_center()==b.scene_center() && a.scene_radius()==b.scene_radius(),"scan changed camera scene bounds");
}
int main(int argc,char**) {
    try {
        initialize(false,false,false);
        TestWindow options; options.visible=false; options.width=151; options.height=113;
        options.device.validation=options.device.sync_validation=argc>1;
        TutorialVirtualScanner viewer("Native virtual scanner test"); set_size(viewer, options);
        auto key=glfwSetKeyCallback(viewer.window(),nullptr); glfwSetKeyCallback(viewer.window(),key);
        auto press=[&](int code,int mods=0) { key(viewer.window(),code,0,GLFW_PRESS,mods); };
        frame(viewer); press(Viewer::KEY_SPACE);
        require(viewer.models().empty(),"empty scan added a model");
        auto mesh=std::make_shared<SurfaceMesh>();
        auto a=mesh->add_vertex(vec3(-1,-1,0)),b=mesh->add_vertex(vec3(1,-1,0));
        auto c=mesh->add_vertex(vec3(1,1,0)),d=mesh->add_vertex(vec3(-1,1,0)); mesh->add_quad(a,b,c,d);
        viewer.add_model(mesh); viewer.camera()->set_view_direction(vec3(0,0,-1)); viewer.fit_screen(); frame(viewer);
        viewer.camera()->set_pivot_point(vec3(.2f,.1f,.15f));
        for (unsigned samples:{1u,4u}) {
            if (!(viewer.device().supported_sample_counts()&samples)) continue;
            viewer.set_samples(samples); frame(viewer);
            auto custom=viewer.camera()->projection_matrix(); custom(0,2)+=.05f;
            viewer.camera()->set_projection_matrix(custom);
            const auto expected=viewer.scan_visible_points(samples);
            Camera camera(*viewer.camera());
            camera.frame()->set_pivot_point(viewer.camera()->pivot_point());
            camera.set_projection_matrix(viewer.camera()->projection_matrix());
            camera.set_model_view_matrix(viewer.camera()->model_view_matrix());
            const auto before=viewer.models().size();
            press(Viewer::KEY_SPACE,Viewer::MODIF_CTRL);
            require(viewer.models().size()==before,"modified Space unexpectedly scanned");
            press(Viewer::KEY_N,Viewer::MODIF_CTRL); press(Viewer::KEY_SPACE); frame(viewer);
            require(viewer.models().size()==before+1,"Space did not add scan result");
            auto cloud=std::dynamic_pointer_cast<PointCloud>(viewer.models().back());
            require(cloud && cloud->points()==expected,"scan cloud does not match framebuffer world points");
            same_camera(camera,*viewer.camera());
            cloud->renderer()->set_visible(false);
            press(Viewer::KEY_N); press(Viewer::KEY_SPACE); frame(viewer);
            auto noisy=std::dynamic_pointer_cast<PointCloud>(viewer.models().back());
            require(noisy && noisy->n_vertices()==expected.size(),"noise altered scan sample count");
            unsigned changed=0;
            for (size_t i=0;i<expected.size();++i) {
                changed+=noisy->points()[i]!=expected[i];
                for (unsigned j=0;j<3;++j) require(std::isfinite(noisy->points()[i][j]),"noise created nonfinite points");
            }
            require(changed>10,"N did not enable scan noise"); same_camera(camera,*viewer.camera());
            noisy->renderer()->set_visible(false); press(Viewer::KEY_N);
            viewer.resize(173+int(samples),127+int(samples)); frame(viewer);
            int w,h; viewer.framebuffer_size(w,h);
            const auto resized=viewer.scan_visible_points(samples);
            press(Viewer::KEY_SPACE); frame(viewer);
            auto result=std::dynamic_pointer_cast<PointCloud>(viewer.models().back());
            require(result && result->points()==resized && viewer.framebuffer_width()==w && viewer.framebuffer_height()==h,
                    "resized/HiDPI acquisition used logical or stale framebuffer coordinates");
            result->renderer()->set_visible(false);
        }
        viewer.clear_scene(); press(Viewer::KEY_SPACE);
        require(viewer.models().empty(),"cleared scene scanned old geometry");
        require(viewer.device().validation_errors()==0,"scanner viewer validation errors");
        std::cout << "PASS: native scanner shortcuts, noise toggle, exact scan clouds, stable camera, MSAA, HiDPI/resize and empty-scene handling\n";
    } catch(const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
