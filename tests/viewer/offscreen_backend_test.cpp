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
#ifndef EASY3D_HAS_VULKAN
#include <easy3d/renderer/framebuffer_object.h>
#include <easy3d/renderer/opengl.h>
#endif
#include <cmath>
#include <algorithm>
#include <iostream>
#include <limits>
#include <type_traits>
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
    require(faces->has_cpu_buffers(),"common drawable did not select portable CPU geometry");
    require(!static_cast<const Drawable &>(*faces).buffer_data().vertices.empty(),"common drawable CPU geometry was not realized");
    renderer.camera()->set_view_direction(vec3(0,0,-1));renderer.camera()->set_up_vector(vec3(0,1,0));renderer.fit_screen();
    renderer.set_background_color(vec4(.12f,.23f,.34f,1));
    const auto matrix=renderer.camera()->model_view_projection_matrix();
    require(renderer.render(prefix+".png",1,1,0),"common render failed");
    const auto rgba=image(prefix+".png",161,127);const auto center=(size_t(63)*161+80)*4;
    require(std::abs(int(rgba[center])-204)<=1 && std::abs(int(rgba[center+1])-51)<=1,"common scene did not render its material color");
    require(std::abs(int(rgba[0])-31)<=1 && std::abs(int(rgba[1])-59)<=1,"common background did not render");
    const auto frame=renderer.capture(1);
    const size_t center_pixel=size_t(63)*frame.width+80;
    require(frame.width==161 && frame.height==127,"capture dimensions differ from the shared viewport");
    require(frame.ids[center_pixel].object==0 &&
            std::isfinite(frame.depth[center_pixel]) && frame.depth[center_pixel]>=0 && frame.depth[center_pixel]<1,
            "center capture lost its scene depth or object ID");
#ifdef EASY3D_HAS_VULKAN
    require(frame.ids[center_pixel].primitive==0,"Vulkan capture lost its primitive ID");
#else
    require(frame.ids[center_pixel].primitive==UINT32_MAX,"OpenGL capture unexpectedly reported a primitive ID");
#endif
    for (unsigned channel=0;channel<4;++channel)
        require(std::abs(int(frame.rgba[4*center_pixel+channel])-int(rgba[center+channel]))<=1,
                "file snapshot color differs from the common capture");
    for(unsigned i=0;i<16;++i) require(std::abs(renderer.camera()->model_view_projection_matrix()[i]-matrix[i])<1e-6f,"file rendering changed live camera matrices");
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
    const auto resized_frame=renderer.capture(1);
    bool resized_hit=false;
    for (const auto& id:resized_frame.ids) resized_hit=resized_hit || id.object!=UINT32_MAX;
    require(resized_frame.width==212 && resized_frame.height==144 && resized_hit,
            "resize changed capture extent or removed scene IDs");
    require(renderer.render(prefix+"-msaa.png",1.25f,4,2),"scaled multisample render failed");
    const auto transparent=image(prefix+"-msaa.png",265,180);require(transparent[3]==0,"transparent background was lost");
    require(renderer.current_model()==mesh.get() && renderer.models().size()==1,"current model ownership changed");
    auto lines=std::make_shared<LinesDrawable2D>("common overlay");renderer.add_drawable(lines);
    lines->update_vertex_buffer({vec2(10,10),vec2(190,10)},212,144);lines->set_line_width(4);
    require(renderer.drawables().size()==1 && renderer.render(prefix+"-overlay.png",1,1,0),"common drawable attachment failed");
    require(renderer.delete_drawable(lines.get()) && renderer.delete_model(mesh.get()),"common scene deletion failed");
    renderer.clear_scene();require(renderer.models().empty() && renderer.drawables().empty(),"common scene clearing failed");
    require(renderer.device().supported_sample_counts() & 1,"common renderer did not expose a usable device");
#ifdef EASY3D_HAS_VULKAN
    require(renderer.device().validation_errors()==0,"common renderer validation errors");
#endif
}
#ifndef EASY3D_HAS_VULKAN
// What issue #215 asked the OpenGL backend to be able to answer: "this framebuffer
// cannot give you pixels" as a false return, before a caller reads untouched memory or
// the resolve path of an incomplete framebuffer is entered at all. These cases need a
// current context (the OffScreen host activates its own on construction and resize) but
// no damaged driver, so they run on every OpenGL backend.
void exercise_framebuffer_object() {
    FramebufferObject empty(24,18,0);
    unsigned char rgba[4]={7,8,9,10};
    require(!empty.read_color(rgba,1,1),"a framebuffer without attachments read a pixel");
    require(rgba[0]==7 && rgba[1]==8 && rgba[2]==9 && rgba[3]==10,"a refused pixel read rewrote the caller's bytes");
    float depth=-1;
    require(!empty.read_depth(depth,1,1),"a framebuffer without attachments read a depth");
    require(depth==-1,"a refused depth read rewrote the caller's value");
    std::vector<unsigned char> pixels;
    require(!empty.read_color(0,pixels,GL_RGBA),"a framebuffer without attachments read the color buffer");
    require(!empty.snapshot_color(0,"attachment-less-framebuffer.png"),"a framebuffer without attachments saved an image");
    std::vector<float> depths;
    require(!empty.read_depth(depths),"a framebuffer without attachments read the depth buffer");

    // The resolve target is cached and mirrors the attachment set, so it has to be
    // rebuilt when that set grows - the one mismatch the rebuild loop exists for. A
    // bound that is too tight fails here instead of in the field.
    FramebufferObject msaa(32,32,4);
    if(msaa.samples()>0) {
        require(msaa.add_color_buffer() && msaa.add_depth_buffer(),"multisample attachments were refused");
        require(msaa.is_valid() && msaa.read_color(rgba,1,1),"the first multisample resolve read failed");
        require(msaa.add_color_buffer(),"a second color attachment was refused");
        std::vector<unsigned char> second;
        require(msaa.read_color(1,second,GL_RGBA),"resolving after the attachment set grew failed");
        require(second.size()==32*32*4,"the rebuilt resolve target returned another size");
    }
}
#endif
int main() {
    static_assert(std::is_base_of<Viewer,OffScreen>::value, "OffScreen must reuse Viewer");
    static_assert(!std::is_convertible<OffScreen*,Viewer*>::value, "OffScreen inheritance must be protected");
    try {
        initialize(false,false,false);
#ifndef EASY3D_HAS_VULKAN
        for (int option = 0; option < 4; ++option) {
            graphics::DeviceOptions invalid;
            if (option == 0) invalid.validation = true;
            if (option == 1) invalid.sync_validation = true;
            if (option == 2) invalid.profiling = true;
            if (option == 3) invalid.device_name = "requested-device";
            bool rejected = false;
            try { OffScreen unsupported(1, 1, invalid); }
            catch (const std::invalid_argument&) { rejected = true; }
            require(rejected, "OpenGL silently ignored an unsupported DeviceOptions request");
        }
#endif
        for (const auto& size : {ivec2(0,1), ivec2(-1,1), ivec2(1,-1),
                                 ivec2(std::numeric_limits<int>::min(),1)}) {
            bool rejected = false;
            try { OffScreen invalid(size.x,size.y,graphics::DeviceOptions{}); }
            catch (const std::invalid_argument&) { rejected = true; }
            require(rejected, "invalid signed OffScreen dimensions were accepted");
        }
        OffScreen renderer(161,127,graphics::DeviceOptions{});
#ifdef EASY3D_HAS_VULKAN
        // Offscreen output is scene-only; a window's default logo/axes must not leak in.
        renderer.set_background_color(vec4(1));
        auto blank = renderer.capture();
        require(std::all_of(blank.rgba.begin(),blank.rgba.end(),[](unsigned char c) { return c == 255; }),
                "empty OffScreen captured viewer HUD content");
        renderer.set_background_color(setting::background_color);
        { OffScreen other(79,63,graphics::DeviceOptions{}); require(other.capture().width==79, "second OffScreen failed"); }
#endif
        exercise(renderer,"configured-offscreen");
#ifndef EASY3D_HAS_VULKAN
        exercise_framebuffer_object();
#endif
        std::cout << "PASS: configured OffScreen rendering, ownership, resize and MSAA\n";
    } catch(const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
