// SPDX-License-Identifier: GPL-3.0-or-later
#include "obj_fixture.h"
#include <tutorials/Tutorial_308_TexturedMesh/viewer.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/texture.h>
#include <easy3d/util/initializer.h>
#include <iostream>
#ifdef EASY3D_NO_OPENGL
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#else
#include <easy3d/renderer/opengl.h>
#endif
using namespace easy3d;
void require(bool value,const char* message) { if (!value) throw std::runtime_error(message); }
#ifdef EASY3D_NO_OPENGL
void frame(TexturedViewer& viewer) {
    for (unsigned i=0;i<100;++i) {
        VulkanViewer::poll_events(); if (viewer.draw_frame()) return;
        glfwWaitEventsTimeout(0.01);
    }
    throw std::runtime_error("textured viewer did not present");
}
#endif
int main(int argc,char**) {
    try {
        initialize(false,false,false);
#ifdef EASY3D_NO_OPENGL
        const std::string dir="vulkan-obj-viewer-fixture";
#else
        const std::string dir="opengl-obj-viewer-fixture";
#endif
        const auto file=obj_fixture::create(dir);
#ifdef EASY3D_NO_OPENGL
        VulkanViewerOptions options; options.width=480; options.height=360;
        options.device.validation=options.device.synchronization_validation=argc>1;
        TexturedViewer viewer("Native material viewer test",options);
        if (viewer.device().supported_sample_counts()&4) viewer.set_samples(4);
        const std::string output="vulkan-materials.png";
#else
        TexturedViewer viewer("OpenGL material viewer test");
        while (glGetError()!=GL_NO_ERROR) {}
        const std::string output="opengl-materials.png";
#endif
        viewer.camera()->setType(Camera::ORTHOGRAPHIC);
        viewer.camera()->setViewDirection(vec3(0,0,-1)); viewer.camera()->setUpVector(vec3(0,1,0));
        auto* model=viewer.add_model(file);
        require(model && viewer.models().size()==1,"textured viewer failed to load material mesh");
        for (const auto& d:model->renderer()->triangles_drawables()) {
            d->set_lighting(false);
            if (!d->texture()) continue;
#ifdef EASY3D_NO_OPENGL
            require(d->has_cpu_buffers() && d->texture()->id()==0,"native material viewer used OpenGL resources");
#else
            require(!d->has_cpu_buffers() && d->texture()->id() && glIsTexture(d->texture()->id()),"legacy material viewer did not use GL textures");
#endif
        }
#ifdef EASY3D_NO_OPENGL
        frame(viewer);
#endif
        require(viewer.snapshot(output),"material viewer snapshot failed");
        std::vector<unsigned char> rgba; int w=0,h=0,channels=0;
        require(ImageIO::load(output,rgba,w,h,channels,4,false),"could not read material snapshot");
        auto probe=[&](const vec3& world,int r,int g,int b) {
            const auto p=viewer.camera()->projectedCoordinatesOf(world);
            const int x=int(p.x*w/viewer.camera()->screenWidth()),y=int(p.y*h/viewer.camera()->screenHeight());
            require(x>=0 && y>=0 && x<w && y<h,"material viewer probe outside snapshot");
            const auto i=(std::size_t(y)*w+x)*4;
            require(std::abs(int(rgba[i])-r)<3 && std::abs(int(rgba[i+1])-g)<3 && std::abs(int(rgba[i+2])-b)<3,
                    "material viewer snapshot has incorrect colors/UVs");
        };
        probe(vec3(-1.5f,.5f,0),0,0,255); probe(vec3(-2.5f,1.5f,0),255,0,0);
        probe(vec3(2,1,0),0,255,0); probe(vec3(-2,-2,0),255,255,0); probe(vec3(2,-2,0),255,0,255);
        model->renderer()->update();
#ifdef EASY3D_NO_OPENGL
        frame(viewer);
#endif
        require(viewer.snapshot(output),"material refresh snapshot failed");
        std::vector<unsigned char> refreshed;
        require(ImageIO::load(output,refreshed,w,h,channels,4,false) && refreshed==rgba,"material refresh changed rendered groups");
#ifdef EASY3D_NO_OPENGL
        auto drop=glfwSetDropCallback(viewer.window(),nullptr); glfwSetDropCallback(viewer.window(),drop);
        const auto replacement_file=dir+"/plain.obj";
        const char* replacement=replacement_file.c_str();
        drop(viewer.window(),1,&replacement); frame(viewer);
        const auto* reloaded=dynamic_cast<SurfaceMesh*>(viewer.current_model());
        require(viewer.models().size()==1 && reloaded && reloaded->n_faces()==1,"OBJ drop did not replace material scene");
        viewer.resize(360,280); frame(viewer);
        const auto p=viewer.camera()->projectedCoordinatesOf(vec3(.2f,.2f,0));
        bool found=false; viewer.point_under_pixel(int(p.x),int(p.y),found);
        require(found,"material reload lost world-point picking");
        require(viewer.device().validation_errors()==0,"native material viewer validation errors");
#else
        require(viewer.add_model(dir+"/plain.obj") && viewer.models().size()==1,"legacy OBJ reload failed");
        require(glGetError()==GL_NO_ERROR,"legacy material viewer GL error");
#endif
        std::cout << "PASS: shared material viewer, backend resources, snapshot UV/colors, refresh and reload\n";
    } catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
