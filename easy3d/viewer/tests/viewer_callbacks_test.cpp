// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/viewer.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/util/initializer.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>
using namespace easy3d;
namespace {
enum class Backend { Vulkan, OpenGL };
void require(bool ok, const char* text) { if (!ok) throw std::runtime_error(text); }
bool same(const mat4& a, const mat4& b) {
    for (unsigned i=0; i<16; ++i) if (std::abs(a[i]-b[i])>1e-5f) return false;
    return true;
}
struct Events {
    GLFWwindow* window;
    GLFWcursorposfun cursor;
    GLFWmousebuttonfun button;
    GLFWscrollfun scroll;
    GLFWkeyfun key;
    GLFWcharfun character;
    GLFWdropfun drop;
    GLFWwindowfocusfun focus;
    explicit Events(GLFWwindow* w) : window(w) {
        cursor=glfwSetCursorPosCallback(w,nullptr); glfwSetCursorPosCallback(w,cursor);
        button=glfwSetMouseButtonCallback(w,nullptr); glfwSetMouseButtonCallback(w,button);
        scroll=glfwSetScrollCallback(w,nullptr); glfwSetScrollCallback(w,scroll);
        key=glfwSetKeyCallback(w,nullptr); glfwSetKeyCallback(w,key);
        character=glfwSetCharCallback(w,nullptr); glfwSetCharCallback(w,character);
        drop=glfwSetDropCallback(w,nullptr); glfwSetDropCallback(w,drop);
        focus=glfwSetWindowFocusCallback(w,nullptr); glfwSetWindowFocusCallback(w,focus);
        require(cursor && button && scroll && key && character && drop && focus,"host callbacks missing");
    }
};
std::shared_ptr<ViewerBase> host(Backend backend, bool validation) {
#ifdef EASY3D_HAS_VULKAN
    if (backend == Backend::Vulkan) {
        VulkanViewerOptions o;
        o.width=181; o.height=151; o.visible=false;
        o.device.validation = validation;
        o.device.synchronization_validation = validation;
        auto v = std::make_shared<VulkanViewer>("Portable callbacks",o);
        v->set_usage("");
        v->camera()->setViewDirection(vec3(0,0,-1)); v->camera()->setUpVector(vec3(0,1,0));
        return v;
    }
#else
    auto v = std::make_shared<OpenGLViewer>("Portable callbacks", 4, 3, 2, false, true, 24, 8, 181, 151);
    v->hide();
    v->set_usage("");
    v->camera()->setViewDirection(vec3(0,0,-1)); v->camera()->setUpVector(vec3(0,1,0));
    return v;
#endif
}
std::shared_ptr<SurfaceMesh> scene(ViewerBase& v) {
    auto m=std::make_shared<SurfaceMesh>();
    const auto a=m->add_vertex(vec3(-1,-1,0)), b=m->add_vertex(vec3(1,-1,0)), c=m->add_vertex(vec3(0,1,0));
    m->add_triangle(a,b,c); v.add_model(m); v.fit_screen();
    m->renderer()->get_triangles_drawable("faces")->set_lighting(false); return m;
}
void frames_and_input(Backend backend, bool validation) {
    auto v=host(backend,validation); auto mesh=scene(*v); Events e(v->window());
    auto faces=mesh->renderer()->get_triangles_drawable("faces");
    faces->set_uniform_coloring(vec4(.8f,.2f,.1f,1));
    // Render the farther blue surface last: missing depth initialization would
    // incorrectly overwrite the foreground in a snapshot before draw_frame().
    auto behind=std::make_shared<SurfaceMesh>();
    const auto a=behind->add_vertex(vec3(-1,-1,-.25f)), b=behind->add_vertex(vec3(1,-1,-.25f)), c=behind->add_vertex(vec3(0,1,-.25f));
    behind->add_triangle(a,b,c);v->add_model(behind);v->fit_screen();
    auto far_faces=behind->renderer()->get_triangles_drawable("faces");
    far_faces->set_lighting(false);far_faces->set_uniform_coloring(vec4(0,0,1,1));
    unsigned before=0, after=0, observed=0; vec4 color;
    int state=0;
    ViewerCallbacks callbacks;
    callbacks.pre_draw=[&,state](ViewerBase* viewer) mutable {
        require(viewer==v.get(),"frame callback received another host");
        observed=unsigned(++state); ++before;
        color=state%2?vec4(.8f,.2f,.1f,1):vec4(.1f,.8f,.2f,1);
        faces->set_uniform_coloring(color);
    };
    callbacks.post_draw=[&](ViewerBase*) { ++after; };
    v->set_callbacks(callbacks);
    require(v->snapshot("portable-before-frame.png",1,4,0) && before==0 && after==0,"initial snapshot advanced frame callbacks");
    int initial_w,initial_h,initial_channels;std::vector<unsigned char> initial;
    require(ImageIO::load("portable-before-frame.png",initial,initial_w,initial_h,initial_channels,4,false),"initial snapshot missing");
    const auto initial_center=(size_t(initial_h/2)*initial_w+initial_w/2)*4;
    require(std::abs(int(initial[initial_center])-204)<=2 && std::abs(int(initial[initial_center+1])-51)<=2,
            "snapshot before first frame lost foreground depth");
    require(v->run_frames(2)==0 && after==2 && before>=2 && observed==before,"frame callbacks lost state/order or frame budget");
    const auto old_before=before,old_after=after;
    require(v->snapshot("portable-callbacks.png",1,4,0),"callback-updated snapshot failed");
    int w,h,channels; std::vector<unsigned char> rgba;
    require(ImageIO::load("portable-callbacks.png",rgba,w,h,channels,4,false),"callback output missing");
    const auto center=(size_t(h/2)*w+w/2)*4;
    for(unsigned i=0;i<3;++i) require(std::abs(int(rgba[center+i])-int(std::lround(color[i]*255)))<=2,"pre_draw changes missed the rendered frame");
    require(before==old_before && after==old_after,"snapshot invoked interactive frame callbacks");

    bool consume=true,consume_release=false; unsigned keys=0,releases=0,bound=0,chars=0,moves=0,drags=0,buttons=0,scrolls=0,focus=0,drops=0;
    int mx=0,my=0,dx=0,dy=0,button_mods=0,last_scroll=0,resize_w=0,resize_h=0; unsigned resizes=0;
    callbacks={};
    callbacks.key_press_event=[&](ViewerBase*,int,int) { ++keys; return consume; };
    callbacks.key_release_event=[&](ViewerBase*,int,int) { ++releases; return true; };
    callbacks.char_input_event=[&](ViewerBase*,unsigned code) { require(code==0x20ac,"Unicode code point changed"); ++chars; return true; };
    callbacks.mouse_free_move_event=[&](ViewerBase*,int x,int y,int a,int b,int) { ++moves; mx=x;my=y;dx=a;dy=b;return true; };
    callbacks.mouse_press_event=[&](ViewerBase*,int x,int y,int,int mods) { ++buttons;mx=x;my=y;button_mods=mods;return consume; };
    callbacks.mouse_drag_event=[&](ViewerBase*,int x,int y,int a,int b,int,int) { ++drags;mx=x;my=y;dx=a;dy=b;return false; };
    callbacks.mouse_release_event=[&](ViewerBase*,int,int,int,int) { ++buttons;return consume_release; };
    callbacks.mouse_scroll_event=[&](ViewerBase*,int,int,int,int sy) { ++scrolls;last_scroll=sy;return true; };
    callbacks.focus_event=[&](ViewerBase*,bool) { ++focus;return true; };
    callbacks.drop_event=[&](ViewerBase*,const std::vector<std::string>& files) { require(files==std::vector<std::string>{"callbacks-drop.ply"},"drop filenames changed");++drops;return consume; };
    callbacks.post_resize=[&](ViewerBase* viewer,int x,int y) { ++resizes;resize_w=x;resize_h=y;require(x==viewer->width() && y==viewer->height(),"resize callback preceded host state update"); };
    v->set_callbacks(callbacks);
    v->bind([&](ViewerBase*,Model*) { ++bound;return true; },mesh.get(),ViewerBase::KEY_J);
    const auto projection=v->camera()->type();
    e.key(e.window,GLFW_KEY_P,0,GLFW_PRESS,0); e.key(e.window,GLFW_KEY_P,0,GLFW_RELEASE,0);
    require(keys==1 && releases==1 && v->camera()->type()==projection,"consumed key reached builtin controls");
    e.key(e.window,GLFW_KEY_J,0,GLFW_PRESS,0); require(bound==0,"consumed key reached bound command");
    consume=false;
    e.key(e.window,GLFW_KEY_P,0,GLFW_PRESS,0); require(v->camera()->type()!=projection,"unhandled key did not reach builtin controls");
    e.key(e.window,GLFW_KEY_J,0,GLFW_PRESS,0); require(bound==1,"unhandled key missed bound command");
    e.character(e.window,0x20ac); require(chars==1,"character callback missing");
    e.cursor(e.window,21.75,31.25); e.cursor(e.window,24.75,36.25);
    require(mx==24 && my==36 && dx==3 && dy==5 && moves>=2,"mouse movement was not in logical window units");

    consume=true;
    double px,py; glfwGetCursorPos(e.window,&px,&py);
    const auto camera=v->camera()->modelViewProjectionMatrix();
    e.button(e.window,GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS,GLFW_MOD_SHIFT);
    require(mx==int(px) && my==int(py) && button_mods==GLFW_MOD_SHIFT,"button event lost actual logical cursor/modifiers");
    e.cursor(e.window,30.75,40.25); e.cursor(e.window,33.75,44.25);
    require(mx==33 && my==44 && dx==3 && dy==4 && drags==2,"captured drag coordinates/deltas changed");
    e.button(e.window,GLFW_MOUSE_BUTTON_LEFT,GLFW_RELEASE,0);
    require(buttons==2 && same(camera,v->camera()->modelViewProjectionMatrix()),"consumed gesture moved builtin camera");
    // Cancel a builtin region zoom by consuming its release.
    consume=false; consume_release=true;
    e.button(e.window,GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS,ViewerBase::MODIF_CTRL);
    e.cursor(e.window,70,80);
    const auto zoom_camera=v->camera()->modelViewProjectionMatrix();
    e.button(e.window,GLFW_MOUSE_BUTTON_LEFT,GLFW_RELEASE,ViewerBase::MODIF_CTRL);
    require(same(zoom_camera,v->camera()->modelViewProjectionMatrix()),"consumed release performed builtin zoom");
    consume_release=false;
    const auto rotating=v->camera()->modelViewProjectionMatrix();
    e.button(e.window,GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS,0);
    e.cursor(e.window,80,80);bool moved=!same(rotating,v->camera()->modelViewProjectionMatrix());
    e.cursor(e.window,86,84);moved=moved || !same(rotating,v->camera()->modelViewProjectionMatrix());
    e.button(e.window,GLFW_MOUSE_BUTTON_LEFT,GLFW_RELEASE,0);
    require(moved,"default drag did not resume after capture/release");
    consume=true;
    e.button(e.window,GLFW_MOUSE_BUTTON_LEFT,GLFW_PRESS,0);
    const auto drag_count=drags;
    e.focus(e.window,GLFW_FALSE); e.cursor(e.window,40,40);
    require(focus>=1 && drags==drag_count,"focus loss retained a drag/capture");
    e.scroll(e.window,0,.4); e.scroll(e.window,0,.4); e.scroll(e.window,0,.4);
    require(scrolls==1 && last_scroll==1,"fractional scroll was lost or dispatched prematurely");
    e.scroll(e.window,0,.8); require(scrolls==2 && last_scroll==1,"fractional scroll remainder was not retained");
    { std::ofstream file("callbacks-drop.ply"); file<<"ply\nformat ascii 1.0\nelement vertex 3\nproperty float x\nproperty float y\nproperty float z\nelement face 1\nproperty list uchar int vertex_indices\nend_header\n-1 -1 0\n1 -1 0\n0 1 0\n3 0 1 2\n";require(bool(file),"drop fixture could not be written"); }
    const char* file="callbacks-drop.ply"; const auto models=v->models().size();
    e.drop(e.window,1,&file); require(drops==1 && v->models().size()==models,"consumed drop reached model loading");
    consume=false; e.drop(e.window,1,&file); require(drops==2 && v->models().size()==models+1,"unhandled drop did not load model");
    v->resize(193,157); require(v->run_frames(1)==0,"render after callback resize failed");
    require(resizes>=1 && resize_w==193 && resize_h==157,"logical resize notification missing");
    const auto resize_count=resizes;
    auto framebuffer=glfwSetFramebufferSizeCallback(e.window,nullptr); glfwSetFramebufferSizeCallback(e.window,framebuffer);
    require(framebuffer!=nullptr,"framebuffer resize callback missing");
    framebuffer(e.window,v->framebuffer_width(),v->framebuffer_height());
    require(resizes==resize_count,"duplicate framebuffer event repeated common resize callback");
    require(v->validation_errors()==0,"portable callback rendering validation errors");
}
void replacement_and_failures(Backend backend,bool validation) {
    auto v=host(backend,validation);scene(*v);Events e(v->window());
    auto lease=std::make_shared<int>(7); std::weak_ptr<int> weak=lease;
    ViewerCallbacks callbacks;
    callbacks.key_press_event=[&,lease](ViewerBase* viewer,int,int) {
        viewer->set_callbacks({});
        require(!weak.expired() && *lease==7,"callback destroyed its active callable during replacement");return true;
    };
    v->set_callbacks(std::move(callbacks));callbacks={};lease.reset();
    e.key(e.window,GLFW_KEY_J,0,GLFW_PRESS,0);require(weak.expired(),"replaced callback retained captures after dispatch");
    const auto window=v->window();
    callbacks={};callbacks.pre_draw=[](ViewerBase*) { throw std::runtime_error("portable frame failure"); };
    v->set_callbacks(callbacks);require(v->run_frames(1)!=0 && v->window()==window,"frame callback error changed window");
    v->set_callbacks({});require(v->run_frames(1)==0,"clearing failed frame callback did not restore rendering");
    callbacks={};callbacks.key_press_event=[](ViewerBase*,int,int)->bool { throw std::runtime_error("portable input failure"); };
    v->set_callbacks(callbacks);
    e.key(e.window,GLFW_KEY_J,0,GLFW_PRESS,0); // must not unwind through C callback
    require(v->should_close() && v->run_frames(1)!=0 && v->window()==window,"input callback failure was hidden or switched hosts");
}
}
int main(int argc,char** argv) {
    try {
        initialize(false,false,false);Backend backend=Backend::Vulkan;bool validation=false;
        for(int i=1;i<argc;++i) { if(std::string(argv[i])=="--opengl")backend=Backend::OpenGL;else if(std::string(argv[i])=="--validation")validation=true; }
        frames_and_input(backend,validation);replacement_and_failures(backend,validation);
        std::cout<<"PASS: portable callbacks, logical coordinates, consumption/capture, default controls, resize, lifetime and exceptions\n";
    }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
