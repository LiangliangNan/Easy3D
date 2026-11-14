// SPDX-License-Identifier: GPL-3.0-or-later
#include <tests/test.h>
#include <easy3d/viewer/viewer.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/shape.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/fileio/image_io.h>
#include <tests/viewer/test_window.h>   // ensure_test_glfw()
#ifndef EASY3D_HAS_VULKAN
#include <easy3d/renderer/opengl.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#endif
#include <cmath>
#include <limits>
#include <algorithm>
using namespace easy3d;
using test::require;
std::shared_ptr<SurfaceMesh> triangle(float z) {
    auto m=std::make_shared<SurfaceMesh>();m->add_triangle(m->add_vertex(vec3(-1,-1,z)),m->add_vertex(vec3(1,-1,z)),m->add_vertex(vec3(0,1,z)));return m;
}
void setup(OffScreen& v,const std::shared_ptr<SurfaceMesh>& mesh) {
    v.add_model(mesh);v.camera()->set_view_direction(vec3(0,0,-1));v.camera()->set_up_vector(vec3(0,1,0));
    v.camera()->set_type(Camera::ORTHOGRAPHIC);v.fit_screen();
    for(auto d:mesh->renderer()->points_drawables())d->set_visible(false);
    for(auto d:mesh->renderer()->lines_drawables())d->set_visible(false);
    auto* faces=mesh->renderer()->get_triangles_drawable("faces");faces->set_lighting(false);faces->set_uniform_coloring(vec4(1,0,0,1));
}
struct Pixels {
    int w=0,h=0,c=0;std::vector<unsigned char> rgba;
    explicit Pixels(const char* path){require(ImageIO::load(path,rgba,w,h,c,4,false),"cannot read offscreen query fixture");}
    const unsigned char* at(int x,int y)const{return rgba.data()+4*(size_t(y)*w+x);}
};
int main(int argc,char**) {return test::run("shared OffScreen queries, selection and annotations",[&]{
#ifdef EASY3D_HAS_VULKAN
    VulkanOptions options;options.validation=options.sync_validation=argc>1;
#else
    OpenGLOptions options;
#endif
    Viewer window("Interleaved query host",151,113,4,false,false,true,options);window.set_usage("");
    OffScreen a(161,127,options),b(103,79,options);
    auto mesh=triangle(0),other=triangle(.4f);setup(a,mesh);setup(b,other);
    const auto matrix=a.camera()->model_view_projection_matrix();const auto pivot=a.camera()->pivot_point();
    const auto verify=[&](OffScreen& v,const std::shared_ptr<SurfaceMesh>& expected,float z) {
        const auto camera_matrix=v.camera()->model_view_projection_matrix();const auto saved_pivot=v.camera()->pivot_point();
        const int x=v.width()/2,y=v.height()/2;const auto hit=v.pick(x,y);
        require(hit.model==expected && hit.face_index==0 && hit.drawable==expected->renderer()->triangles_drawables().front(),"offscreen source ownership/provenance is wrong");
        require(std::abs(hit.position.z-z)<1e-5f,"offscreen world hit is wrong");
        bool found=false;const auto p=v.point_under_pixel(x,y,found);
        require(found && distance(p,hit.position)<1e-4f && distance(v.pixel_to_world(x,y,hit.depth),hit.position)<1e-5f,"offscreen query/unprojection contracts diverged");
        const auto q=v.world_to_window(hit.position);require(std::abs(q.x-x-.5f)<1e-3f && std::abs(q.y-y-.5f)<1e-3f,"offscreen projection lost the exact pixel center");
        require(distance(v.window_to_world(q),hit.position)<1e-4f,"offscreen coordinate round trip failed");
        require(!v.pick(-1,y) && !v.pick(v.width(),y),"outside offscreen query returned a source");
        for(unsigned i=0;i<16;++i)require(std::abs(v.camera()->model_view_projection_matrix()[i]-camera_matrix[i])<1e-6f,"offscreen query changed camera matrices");
        require(v.camera()->pivot_point()==saved_pivot,"offscreen query changed camera pivot");
    };
    verify(a,mesh,0);verify(b,other,.4f);verify(a,mesh,0);
    const auto before=a.pick(80,63);
    auto overlay=graphics::shape::draw_quad_filled(Rect(0,161,0,127),vec4(0,1,0,.5f),161,127);
    a.set_overlays({overlay});
    for(int samples:{1,4}) {
        require(a.render("overlay.png",1.5f,samples,2),"annotated offscreen file render failed");Pixels image("overlay.png");
        require(image.w==242 && image.h==191,"offscreen scaled canvas follows hidden-window DPI");
        const auto p=image.at(image.w/2,image.h/2);
        require(std::abs(int(p[0])-127)<=2 && std::abs(int(p[1])-128)<=2 && p[2]<3 && p[3]==255,"offscreen annotation alpha or scene color differs");
        const auto after=a.pick(80,63);require(after.model==before.model && std::abs(after.depth-before.depth)<1e-6f,"offscreen MSAA/annotation changed scene depth or source");
    }
    for(unsigned i=0;i<16;++i)require(std::abs(a.camera()->model_view_projection_matrix()[i]-matrix[i])<1e-6f,"offscreen annotated rendering changed the live camera");
    mesh->renderer()->set_visible(false);require(!a.pick(80,63),"offscreen query included a hidden model");mesh->renderer()->set_visible(true);
    auto points=std::make_shared<PointsDrawable>("shared offscreen points");a.add_drawable(points);
    points->update_vertex_buffer({vec3(-.4f,0,.2f),vec3(.4f,0,.2f)});points->set_impostor_type(PointsDrawable::SPHERE);points->set_point_size(21);
    graphics::PointSelectionRequest request;request.points=a.point_resource(points.get());
    request.width=200;request.height=100;request.polygon={{0,0},{100,0},{100,100},{0,100}};
    const auto original=request.points;require(a.select_points(request)==std::vector<uint32_t>{1},"offscreen GPU lasso selection failed");
    points->update_color_buffer({vec3(1,0,0),vec3(0,0,1)});require(a.point_resource(points.get())==original,"offscreen color update replaced shared positions");
#ifndef EASY3D_HAS_VULKAN
    // "The caller's context stays current" is only observable from inside that context:
    // glfwGetCurrentContext() reports the record kept by whichever GLFW copy reads it, and a
    // test executable links its own copy of the static library. A context-local binding says
    // the same thing where it can be seen - it is still there only if the calls below handed
    // this context back.
    ensure_test_glfw();
    glfwMakeContextCurrent(window.window());
    GLuint marker=0;glGenBuffers(1,&marker);glBindBuffer(GL_PIXEL_PACK_BUFFER,marker);
    verify(a,mesh,0);a.point_resource(points.get());a.select_points(request);a.set_overlays({overlay});
    GLint current=0;glGetIntegerv(GL_PIXEL_PACK_BUFFER_BINDING,&current);
    glBindBuffer(GL_PIXEL_PACK_BUFFER,0);glDeleteBuffers(1,&marker);
    require(GLuint(current)==marker,"offscreen query/selection changed the caller context");
#endif
    a.resize(173,133);require(window.run_frames(2)==0,"interleaved native viewer did not render");
    require(a.width()==173 && a.height()==133 && a.framebuffer_width()==173 && a.dpi_scaling()==1,"offscreen fixed pixel canvas changed after native callbacks");
    a.set_overlays({});verify(a,mesh,0);
    for(unsigned i=0;i<16;++i) require(std::isfinite(a.camera()->model_view_projection_matrix()[i]),"offscreen resize produced invalid matrices");
    bool rejected=false;try{a.pixel_to_world(0,0,std::numeric_limits<float>::quiet_NaN());}catch(const std::invalid_argument&){rejected=true;}
    require(rejected,"offscreen unprojection accepted nonfinite depth");
    a.set_overlays({overlay});a.clear_scene();
    require(!a.pick(86,66) && a.select_points(request)==std::vector<uint32_t>{1},"clear_scene retained geometry or invalidated a retained GPU point snapshot");
    require(a.render("cleared.png",1,4,1),"cleared offscreen file render failed");Pixels cleared("cleared.png");
    require(std::all_of(cleared.rgba.begin(),cleared.rgba.end(),[](unsigned char c){return c==255;}),"clear_scene retained offscreen annotations or HUD");
    points.reset();
#ifdef EASY3D_HAS_VULKAN
    require(a.device().validation_errors()==0 && b.device().validation_errors()==0,"offscreen shared API validation errors");
#endif
    require(pivot==a.camera()->pivot_point(),"offscreen query/render/resize changed the pivot");
});}
