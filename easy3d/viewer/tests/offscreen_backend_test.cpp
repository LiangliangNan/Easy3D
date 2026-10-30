// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/viewer.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines_2D.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/util/initializer.h>
#include <easy3d/util/setting.h>
#include <cmath>
#include <iostream>
#include <limits>
#include <stdexcept>
using namespace easy3d;
void require(bool ok,const char* message) {if(!ok) throw std::runtime_error(message);}
std::shared_ptr<SurfaceMesh> triangle() {
    auto m=std::make_shared<SurfaceMesh>();auto a=m->add_vertex(vec3(-1,-1,0));auto b=m->add_vertex(vec3(1,-1,0));
    auto c=m->add_vertex(vec3(0,1,0));m->add_triangle(a,b,c);return m;
}
std::vector<unsigned char> image(const std::string& path,int width,int height) {
    std::vector<unsigned char> rgba;int w,h,c;
    require(ImageIO::load(path,rgba,w,h,c,4,false),"offscreen output could not be loaded");
    require(w==width && h==height,"offscreen output dimensions differ between backends");return rgba;
}
void exercise(OffScreen& renderer,const std::string& prefix) {
    require(renderer.width()==161 && renderer.height()==127,"constructor dimensions differ between backends");
    require(renderer.background_color()==setting::background_color,"shared background default missing");
    auto mesh=triangle();require(renderer.add_model(mesh)==mesh.get(),"common model attachment failed");
    auto faces=mesh->renderer()->get_triangles_drawable("faces");faces->set_lighting(false);faces->set_uniform_coloring(vec4(.8f,.2f,.1f,1));
    require(faces->has_cpu_buffers()==(bool(EASY3D_TUTORIAL_USES_VULKAN)),"backend did not select its own buffer policy");
    renderer.camera()->setViewDirection(vec3(0,0,-1));renderer.camera()->setUpVector(vec3(0,1,0));renderer.fit_screen();
    renderer.set_background_color(vec4(.12f,.23f,.34f,1));
    const auto matrix=renderer.camera()->modelViewProjectionMatrix();
    require(renderer.render(prefix+".png",1,1,0),"common render failed");
    const auto rgba=image(prefix+".png",161,127);const auto center=(size_t(63)*161+80)*4;
    require(std::abs(int(rgba[center])-204)<=1 && std::abs(int(rgba[center+1])-51)<=1,"common scene did not render its material color");
    require(std::abs(int(rgba[0])-31)<=1 && std::abs(int(rgba[1])-59)<=1,"common background did not render");
    for(unsigned i=0;i<16;++i) require(std::abs(renderer.camera()->modelViewProjectionMatrix()[i]-matrix[i])<1e-6f,"file rendering changed live camera matrices");
    require(!renderer.render(prefix+"-invalid.png",-1),"invalid render request accepted");
#ifdef EASY3D_HAS_VULKAN
    {
        auto invalid=std::make_shared<PointsDrawable>("surfel without normals");renderer.add_drawable(invalid);
        invalid->update_vertex_buffer({vec3(0)});invalid->set_impostor_type(PointsDrawable::SURFEL);
        require(!renderer.render(prefix+"-unsupported.png",1,1),"invalid drawable rendering was accepted");
        renderer.delete_drawable(invalid.get());
    }
#endif
    bool rejected=false;try{renderer.resize(0,127);}catch(const std::invalid_argument&){rejected=true;}
    require(rejected && renderer.width()==161,"invalid resize changed the scene");
    renderer.resize(212,144);require(renderer.width()==212 && renderer.height()==144,"resize was not synchronous");
    require(renderer.render(prefix+"-msaa.png",1.25f,4,2),"scaled multisample render failed");
    const auto transparent=image(prefix+"-msaa.png",265,180);require(transparent[3]==0,"transparent background was lost");
    require(renderer.current_model()==mesh.get() && renderer.models().size()==1,"current model ownership changed");
    auto lines=std::make_shared<LinesDrawable2D>("common overlay");renderer.add_drawable(lines);
    lines->update_vertex_buffer({vec2(10,10),vec2(190,10)},212,144);lines->set_line_width(4);
    require(renderer.drawables().size()==1 && renderer.render(prefix+"-overlay.png",1,1,0),"common drawable attachment failed");
    require(renderer.delete_drawable(lines.get()) && renderer.delete_model(mesh.get()),"common scene deletion failed");
    renderer.clear_scene();require(renderer.models().empty() && renderer.drawables().empty(),"common scene clearing failed");
#ifdef EASY3D_HAS_VULKAN
    require(renderer.device().validation_errors()==0,"common renderer validation errors");
#endif
}
int main() {
    try {
        initialize(false,false,false);
        OffScreen renderer(161,127);
        exercise(renderer,"configured-offscreen");
        std::cout << "PASS: configured OffScreen rendering, ownership, resize and MSAA\n";
    } catch(const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
