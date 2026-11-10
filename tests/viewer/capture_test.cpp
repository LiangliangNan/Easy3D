// SPDX-License-Identifier: GPL-3.0-or-later
#include <tests/test.h>
#include <tutorials/Tutorial_404_VirtualScanner/viewer.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/shape.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/point_cloud.h>
#ifndef EASY3D_HAS_VULKAN
#include <easy3d/renderer/opengl.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#endif
#include <algorithm>
#include <cmath>
using namespace easy3d;
using test::require;
namespace {
std::shared_ptr<SurfaceMesh> quad() {
    auto m=std::make_shared<SurfaceMesh>();
    m->add_quad(m->add_vertex(vec3(-1,-1,0)),m->add_vertex(vec3(1,-1,0)),m->add_vertex(vec3(1,1,0)),m->add_vertex(vec3(-1,1,0)));return m;
}
void material(const std::shared_ptr<SurfaceMesh>& m) {
    for(auto d:m->renderer()->lines_drawables())d->set_visible(false);
    for(auto d:m->renderer()->points_drawables())d->set_visible(false);
    auto* d=m->renderer()->get_triangles_drawable("faces");d->set_lighting(false);d->set_uniform_coloring(vec4(1,0,0,1));
}
template<class Host> void camera(Host& v) {v.camera()->setViewDirection(vec3(0,0,-1));v.camera()->setUpVector(vec3(0,1,0));v.camera()->setType(Camera::ORTHOGRAPHIC);v.fit_screen();}
struct Scanner:TutorialVirtualScanner {using TutorialVirtualScanner::key_press_event;};
}
int main(int argc,char**) {return test::run("shared capture and tutorial scanner",[&]{
#ifdef EASY3D_HAS_VULKAN
    VulkanOptions options;options.validation=options.sync_validation=argc>1;
#else
    OpenGLOptions options;
#endif
    OffScreen host(151,113,options);auto mesh=quad();host.add_model(mesh);material(mesh);camera(host);
    const auto matrix=host.camera()->modelViewProjectionMatrix();
#ifndef EASY3D_HAS_VULKAN
    // Cold capture must tolerate pixel-transfer buffers, including lazy texture uploads.
    GLuint buffers[2];glGenBuffers(2,buffers);
    glBindBuffer(GL_PIXEL_PACK_BUFFER,buffers[0]);glBufferData(GL_PIXEL_PACK_BUFFER,4096,nullptr,GL_STREAM_READ);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER,buffers[1]);glBufferData(GL_PIXEL_UNPACK_BUFFER,4096,nullptr,GL_STREAM_DRAW);
    glPixelStorei(GL_PACK_ROW_LENGTH,19);glPixelStorei(GL_UNPACK_ROW_LENGTH,23);glDisable(GL_MULTISAMPLE);
#endif
    const auto one=host.capture(1),four=host.capture(4);
    require(one.width==151 && one.height==113 && one.rgba.size()==151*113*4,"capture dimensions or color storage differ");
    require(one.depth==four.depth,"color MSAA changed scene depth");
    const size_t center=56*151+75;
    require(one.ids[center].object==0 && one.rgba[4*center]>250 && one.rgba[4*center+1]<3,"capture lost model identity/color");
    for(size_t i=0;i<one.ids.size();++i)require(one.ids[i].object==four.ids[i].object,"MSAA changed capture object IDs");
#ifndef EASY3D_HAS_VULKAN
    GLint binding=0;glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING,&binding);require(GLuint(binding)==buffers[0],"capture lost pack buffer");
    glGetIntegerv(GL_PIXEL_UNPACK_BUFFER_BINDING,&binding);require(GLuint(binding)==buffers[1],"capture lost unpack buffer");
    glGetIntegerv(GL_PACK_ROW_LENGTH,&binding);require(binding==19,"capture lost pack layout");
    glGetIntegerv(GL_UNPACK_ROW_LENGTH,&binding);require(binding==23,"capture lost unpack layout");
    require(!glIsEnabled(GL_MULTISAMPLE) && glGetError()==GL_NO_ERROR,"capture changed multisampling or generated GL errors");
    require(one.ids[center].primitive==UINT32_MAX,"GL capture invented primitive provenance");
    glBindBuffer(GL_PIXEL_PACK_BUFFER,0);glBindBuffer(GL_PIXEL_UNPACK_BUFFER,0);glDeleteBuffers(2,buffers);
    glPixelStorei(GL_PACK_ROW_LENGTH,0);glPixelStorei(GL_UNPACK_ROW_LENGTH,0);glEnable(GL_MULTISAMPLE);
#endif
    const auto hit=host.pick(75,56);require(hit.model==mesh && std::abs(hit.depth-one.depth[center])<1e-6f,"capture and picking disagree");
    auto overlay=graphics::shape::draw_quad_filled(Rect(0,151,0,113),vec4(0,1,0,.5f),151,113);host.set_overlays({overlay});
    const auto annotated=host.capture(4);require(annotated.depth==one.depth && annotated.ids[center].object==0,"annotation contaminated scene metadata");
    require(std::abs(int(annotated.rgba[4*center])-127)<=2 && std::abs(int(annotated.rgba[4*center+1])-128)<=2,"capture overlay compositing differs");
    for(unsigned i=0;i<16;++i)require(host.camera()->modelViewProjectionMatrix()[i]==matrix[i],"capture mutated the camera");
    auto point=std::make_shared<PointsDrawable>("capture sphere");host.add_drawable(point);
    point->update_vertex_buffer({vec3(0,0,.3f)});point->set_point_size(31);point->set_impostor_type(PointsDrawable::SPHERE);
    point->set_lighting(false);point->set_uniform_coloring(vec4(0,0,1,1));host.set_overlays({});
    const auto sphere=host.capture(4);const auto sphere_hit=host.pick(75,56);
    require(sphere.ids[center].object==1 && sphere_hit.drawable==point && sphere.depth[center]<one.depth[center] && std::abs(sphere.depth[center]-sphere_hit.depth)<1e-6f,"capture sphere depth or standalone ID differs from picking");
    host.clear_scene();const auto empty=host.capture(1);
    require(std::all_of(empty.ids.begin(),empty.ids.end(),[](const graphics::FrameImage::Id& p){return p.object==UINT32_MAX;}),"clear_scene retained captured objects");
    Scanner scanner;scanner.resize(101,79);scanner.set_usage("");scanner.key_press_event(Scanner::KEY_SPACE,0);require(scanner.models().empty(),"empty tutorial scan added points");
    auto source=quad();scanner.add_model(source);material(source);camera(scanner);scanner.camera()->setPivotPoint(vec3(.1f,.2f,.3f));
    const auto expected=scanner.capture(1);const auto acquisition=scanner.camera()->modelViewProjectionMatrix();const auto pivot=scanner.camera()->pivotPoint();
    scanner.key_press_event(Scanner::KEY_SPACE,0);
    auto cloud=std::dynamic_pointer_cast<PointCloud>(scanner.models().back());
    size_t count=0;for(unsigned y=0;y<expected.height;++y)for(unsigned x=0;x<expected.width;++x) {
        const auto p=size_t(y)*expected.width+x;if(expected.ids[p].object==UINT32_MAX)continue;
        require(cloud && count<cloud->points().size() && distance(cloud->points()[count++],scanner.pixel_to_world(x,y,expected.depth[p]))<1e-4f,"tutorial scan lost framebuffer pixel centers");
    }
    require(count>100 && cloud->n_vertices()==count,"tutorial scan omitted/added scene samples");
    require(scanner.camera()->pivotPoint()==pivot,"tutorial scan changed pivot");
    for(unsigned i=0;i<16;++i)require(scanner.camera()->modelViewProjectionMatrix()[i]==acquisition[i],"tutorial scan changed acquisition camera");
#ifdef EASY3D_HAS_VULKAN
    require(host.device().validation_errors()==0 && scanner.device().validation_errors()==0,"capture validation errors");
#endif
});}
