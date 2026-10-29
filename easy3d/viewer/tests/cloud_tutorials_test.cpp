// SPDX-License-Identifier: GPL-3.0-or-later
#include <tutorials/viewer_backend.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/util/initializer.h>
#include <cmath>
#include <iostream>
#include <stdexcept>
#if !EASY3D_TUTORIAL_USES_VULKAN
#include <easy3d/renderer/opengl.h>
#endif
using namespace easy3d;
bool estimate(TutorialViewer*,Model*);
bool reorient(TutorialViewer*,Model*);
bool reconstruction(TutorialViewer*,Model*);
bool extract_plane(TutorialViewer*,Model*);
void require(bool ok,const char* text) {if(!ok) throw std::runtime_error(text);}
using CloudViewer = TutorialViewer;
std::shared_ptr<PointCloud> sphere(bool with_normals) {
    auto cloud=std::make_shared<PointCloud>();
    for(int y=1;y<24;++y) for(int x=0;x<48;++x) {
        const float phi=float(M_PI*y/24),theta=float(2*M_PI*x/48);
        cloud->add_vertex(vec3(std::sin(phi)*std::cos(theta),std::sin(phi)*std::sin(theta),std::cos(phi)));
    }
    if(with_normals) {
        auto normals=cloud->vertex_property<vec3>("v:normal");
        for(auto v:cloud->vertices()) normals[v]=cloud->position(v);
    }
    return cloud;
}
std::shared_ptr<PointCloud> cube() {
    auto cloud=std::make_shared<PointCloud>();auto normals=cloud->vertex_property<vec3>("v:normal");
    // Sample spacing must be below the tutorial's .02 * bounding-box bitmap resolution.
    const int resolution=64;
    for(int axis=0;axis<3;++axis) for(int side:{-1,1}) for(int y=0;y<resolution;++y) for(int x=0;x<resolution;++x) {
        vec3 p(0),n(0);p[axis]=float(side);n[axis]=float(side);
        p[(axis+1)%3]=-1+(x+.5f)*2/resolution;p[(axis+2)%3]=-1+(y+.5f)*2/resolution;
        normals[cloud->add_vertex(p)]=n;
    }
    return cloud;
}
std::vector<unsigned char> render(CloudViewer& v,const std::string& name) {
#if EASY3D_TUTORIAL_USES_VULKAN
    VulkanViewer::poll_events();require(v.draw_frame(),"cloud tutorial did not present");
    const std::string prefix="vulkan-";const int samples=int(v.samples());
#else
    const std::string prefix="opengl-";const int samples=4;
#endif
    const auto file=prefix+name+".png";
    require(v.snapshot(file,1,samples,1),"cloud tutorial snapshot failed");
    std::vector<unsigned char> rgba;int w,h,c;
    require(ImageIO::load(file,rgba,w,h,c,4,false),"cloud tutorial snapshot readback failed");
    return rgba;
}
int main(int argc,char**) {
    try {
        initialize(false,false,false);
#if EASY3D_TUTORIAL_USES_VULKAN
        VulkanViewerOptions options;options.visible=false;options.width=480;options.height=360;
        options.device.validation=options.device.synchronization_validation=argc>1;
        CloudViewer v("Point cloud algorithm tutorials",options);
        if(v.device().supported_sample_counts()&4) v.set_samples(4);
#else
        CloudViewer v("Point cloud algorithm tutorials");
        while(glGetError()!=GL_NO_ERROR) {}
#endif
        auto cloud=sphere(false);v.add_model(cloud);
        v.camera()->setUpVector(vec3(0,1,0));v.camera()->setViewDirection(vec3(1,0,0));v.fit_screen();
        auto points=cloud->renderer()->get_points_drawable("vertices");
        points->set_uniform_coloring(vec4(.6f,.6f,1,1));points->set_point_size(3);points->set_distinct_back_color(true);
        render(v,"cloud-before-normals");
#if EASY3D_TUTORIAL_USES_VULKAN
        const auto revision=points->cpu_buffers().revision;const auto positions=points->cpu_buffers().vertices;
#endif
        require(estimate(&v,cloud.get()),"normal estimation callback failed");
        auto normals=cloud->get_vertex_property<vec3>("v:normal");require(bool(normals),"estimated normals missing");
        for(auto n:normals.vector()) require(std::isfinite(n.x) && std::abs(length(n)-1)<.01f,"invalid estimated normal");
#if EASY3D_TUTORIAL_USES_VULKAN
        require(points->cpu_buffers().normals==normals.vector() && points->cpu_buffers().vertices==positions &&
                points->cpu_buffers().revision>revision,"normal estimation did not refresh native normals");
#endif
#ifdef EASY3D_TEST_HAS_BOOST
        require(reorient(&v,cloud.get()),"normal reorientation callback failed");
#else
        // Reorientation has always required the optional Boost Graph dependency.
        require(!reorient(&v,cloud.get()),"unavailable reorientation did not fail cleanly");
#endif
#if EASY3D_TUTORIAL_USES_VULKAN
        require(points->cpu_buffers().normals==normals.vector(),"reoriented normals were not uploaded");
#endif
        points->set_impostor_type(PointsDrawable::SURFEL);points->set_point_size(8);
        const auto normal_image=render(v,"cloud-estimated-normals");
        size_t nonwhite=0;
        for(size_t i=0;i<normal_image.size();i+=4)
            nonwhite+=normal_image[i]!=255 || normal_image[i+1]!=255 || normal_image[i+2]!=255;
        require(nonwhite>normal_image.size()/400,"estimated-normal cloud is outside the camera view");
#if EASY3D_TUTORIAL_USES_VULKAN
        const auto surfels=v.capture();size_t visible=0;
        for(auto id:surfels.ids) if(id.object!=UINT32_MAX) {
            require(id.primitive<cloud->n_vertices(),"normal refresh changed point provenance");++visible;
        }
        require(visible>1000,"estimated normals did not produce visible surfels");
#endif
        v.delete_model(cloud.get());cloud.reset();

        cloud=cube();v.add_model(cloud);
        v.camera()->setViewDirection(vec3(-1,-1,-1));v.fit_screen();
        points=cloud->renderer()->get_points_drawable("vertices");
        points->set_uniform_coloring(vec4(.6f,.6f,1,1));points->set_point_size(3);points->set_lighting(false);
        const auto plain=render(v,"cloud-before-planes");
#if EASY3D_TUTORIAL_USES_VULKAN
        const auto before=v.capture();
#endif
        require(extract_plane(&v,cloud.get()),"plane extraction callback failed");
        auto labels=cloud->get_vertex_property<int>("v:primitive_index");require(bool(labels),"plane labels missing");
        size_t assigned=0;for(int label:labels.vector()) assigned+=label>=0;
        require(assigned>cloud->n_vertices()/2,"plane extraction missed fixture planes");
        require(points->coloring_method()==State::COLOR_PROPERTY,"plane segmentation coloring not selected");
        require(render(v,"cloud-extracted-planes")!=plain,"plane segmentation did not refresh rendered colors");
#if EASY3D_TUTORIAL_USES_VULKAN
        const auto after=v.capture();require(before.depth==after.depth,"segmentation changed point depth");
        for(size_t i=0;i<before.ids.size();++i)
            require(before.ids[i].object==after.ids[i].object && before.ids[i].primitive==after.ids[i].primitive,"segmentation changed picking IDs");
#endif
        v.delete_model(cloud.get());cloud.reset();

        cloud=sphere(true);v.add_model(cloud);
        render(v,"cloud-before-reconstruction");
        // Retain the input to verify it leaves the scene without invalidating caller ownership.
        require(reconstruction(&v,cloud.get()),"Poisson reconstruction callback failed");
        require(v.models().size()==1 && v.current_model()!=cloud.get(),"reconstruction did not replace input cloud");
        auto* mesh=dynamic_cast<SurfaceMesh*>(v.current_model());require(mesh && mesh->n_faces()>100,"Poisson reconstruction produced no surface");
        render(v,"cloud-reconstructed-surface");
#if EASY3D_TUTORIAL_USES_VULKAN
        const auto image=v.capture();bool picked=false;
        for(unsigned y=0;y<image.height && !picked;y+=3) for(unsigned x=0;x<image.width && !picked;x+=3) {
            if(image.ids[size_t(y)*image.width+x].object==UINT32_MAX) continue;
            const auto hit=v.pick(int(x),int(y));
            require(hit && hit.model.get()==mesh && hit.face_index>=0,"reconstructed surface lost face provenance");picked=true;
        }
        require(picked,"reconstructed mesh did not produce picking IDs");
        require(v.device().validation_errors()==0,"cloud tutorial validation errors");
#else
        require(glGetError()==GL_NO_ERROR,"cloud tutorial OpenGL error");
#endif
        std::cout<<"PASS: original 701/702/703 callbacks, normal refresh, plane colors and reconstructed surface\n";
    } catch(const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
