// SPDX-License-Identifier: GPL-3.0-or-later
#include <tutorials/Tutorial_404_VirtualScanner/viewer_vulkan.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/util/initializer.h>
#include <iostream>
#include <stdexcept>
using namespace easy3d;
void require(bool ok,const char* text) { if (!ok) throw std::runtime_error(text); }
void frame(VulkanViewer& viewer) {
    for (unsigned i=0;i<100;++i) {
        VulkanViewer::poll_events(); if (viewer.draw_frame()) return;
        glfwWaitEventsTimeout(.01);
    }
    throw std::runtime_error("scanner viewer did not present");
}
void same_camera(const Camera& a,const Camera& b) {
    const auto m=a.modelViewProjectionMatrix(),n=b.modelViewProjectionMatrix();
    for (unsigned i=0;i<16;++i) require(std::abs(m[i]-n[i])<1e-6f,"scan changed the acquisition camera");
    require(a.screenWidth()==b.screenWidth() && a.screenHeight()==b.screenHeight(),"scan changed framebuffer dimensions");
    require(a.pivotPoint()==b.pivotPoint(),"scan changed the interaction pivot");
    require(a.sceneCenter()==b.sceneCenter() && a.sceneRadius()==b.sceneRadius(),"scan changed camera scene bounds");
}
int main(int argc,char**) {
    try {
        initialize(false,false,false);
        VulkanViewerOptions options; options.visible=false; options.width=151; options.height=113;
        options.device.validation=options.device.synchronization_validation=argc>1;
        TutorialVirtualScanner viewer("Native virtual scanner test",options);
        auto key=glfwSetKeyCallback(viewer.window(),nullptr); glfwSetKeyCallback(viewer.window(),key);
        auto press=[&](int code,int mods=0) { key(viewer.window(),code,0,GLFW_PRESS,mods); };
        frame(viewer); press(GLFW_KEY_SPACE);
        require(viewer.models().empty(),"empty scan added a model");
        auto mesh=std::make_shared<SurfaceMesh>();
        auto a=mesh->add_vertex(vec3(-1,-1,0)),b=mesh->add_vertex(vec3(1,-1,0));
        auto c=mesh->add_vertex(vec3(1,1,0)),d=mesh->add_vertex(vec3(-1,1,0)); mesh->add_quad(a,b,c,d);
        viewer.add_model(mesh); viewer.camera()->setViewDirection(vec3(0,0,-1)); viewer.fit_screen(); frame(viewer);
        viewer.camera()->setPivotPoint(vec3(.2f,.1f,.15f));
        for (unsigned samples:{1u,4u}) {
            if (!(viewer.device().supported_sample_counts()&samples)) continue;
            viewer.set_samples(samples); frame(viewer);
            auto custom=viewer.camera()->projectionMatrix(); custom(0,2)+=.05f;
            viewer.camera()->set_projection_matrix(custom);
            const auto expected=viewer.scan_visible_points(samples);
            Camera camera(*viewer.camera());
            camera.frame()->setPivotPoint(viewer.camera()->pivotPoint());
            camera.set_projection_matrix(viewer.camera()->projectionMatrix());
            camera.set_modelview_matrix(viewer.camera()->modelViewMatrix());
            const auto before=viewer.models().size();
            press(GLFW_KEY_SPACE,GLFW_MOD_CONTROL);
            require(viewer.models().size()==before,"modified Space unexpectedly scanned");
            press(GLFW_KEY_N,GLFW_MOD_CONTROL); press(GLFW_KEY_SPACE); frame(viewer);
            require(viewer.models().size()==before+1,"Space did not add scan result");
            auto cloud=std::dynamic_pointer_cast<PointCloud>(viewer.models().back());
            require(cloud && cloud->points()==expected,"scan cloud does not match framebuffer world points");
            same_camera(camera,*viewer.camera());
            cloud->renderer()->set_visible(false);
            press(GLFW_KEY_N); press(GLFW_KEY_SPACE); frame(viewer);
            auto noisy=std::dynamic_pointer_cast<PointCloud>(viewer.models().back());
            require(noisy && noisy->n_vertices()==expected.size(),"noise altered scan sample count");
            unsigned changed=0;
            for (size_t i=0;i<expected.size();++i) {
                changed+=noisy->points()[i]!=expected[i];
                for (unsigned j=0;j<3;++j) require(std::isfinite(noisy->points()[i][j]),"noise created nonfinite points");
            }
            require(changed>10,"N did not enable scan noise"); same_camera(camera,*viewer.camera());
            noisy->renderer()->set_visible(false); press(GLFW_KEY_N);
            viewer.resize(173+int(samples),127+int(samples)); frame(viewer);
            int w,h; viewer.framebuffer_size(w,h);
            const auto resized=viewer.scan_visible_points(samples);
            press(GLFW_KEY_SPACE); frame(viewer);
            auto result=std::dynamic_pointer_cast<PointCloud>(viewer.models().back());
            require(result && result->points()==resized && viewer.width()==w && viewer.height()==h,
                    "resized/HiDPI acquisition used logical or stale framebuffer coordinates");
            result->renderer()->set_visible(false);
        }
        viewer.clear_scene(); press(GLFW_KEY_SPACE);
        require(viewer.models().empty(),"cleared scene scanned old geometry");
        require(viewer.device().validation_errors()==0,"scanner viewer validation errors");
        std::cout << "PASS: native scanner shortcuts, noise toggle, exact scan clouds, stable camera, MSAA, HiDPI/resize and empty-scene handling\n";
    } catch(const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
