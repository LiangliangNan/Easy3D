#include "test_window.h"
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
void require(bool ok, const char* text) { if (!ok) throw std::runtime_error(text); }
bool same(const mat4& a, const mat4& b) {
    for (unsigned i=0; i<16; ++i) if (std::abs(a[i]-b[i])>1e-5f) return false;
    return true;
}
// Records the host callbacks so the test can inject raw events, as a window
// system would, and exercise the shared input router.
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
        ensure_test_glfw();
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
// Counts frames and input through the event hooks, which are the single
// customization surface now that ViewerCallbacks is gone.
struct EventViewer : Viewer {
    using Viewer::Viewer;
    TrianglesDrawable* faces = nullptr;
    unsigned before=0, after=0;
    vec4 color{0,0,0,1};
    bool consume=true, consume_release=false, swallow_gesture=false;
    bool throw_in_pre_draw=false, throw_on_key=false;
    unsigned keys=0,releases=0,chars=0,moves=0,drags=0,buttons=0,scrolls=0,focus=0,drops=0,resizes=0;
    int mx=0,my=0,dx=0,dy=0,button_mods=0,last_scroll=0,resize_w=0,resize_h=0;

    void pre_draw() override {
        if (throw_in_pre_draw)
            throw std::runtime_error("portable frame failure");
        color = ++before%2 ? vec4(.8f,.2f,.1f,1) : vec4(.1f,.8f,.2f,1);
        if (faces)
            faces->set_uniform_coloring(color);
    }
    void post_draw() override { ++after; }
    void post_resize(int w, int h) override {
        ++resizes; resize_w=w; resize_h=h;
        require(w==width() && h==height(),"resize hook preceded host state update");
    }
    bool key_press_event(int key, int mods) override {
        if (throw_on_key)
            throw std::runtime_error("portable input failure");
        ++keys;
        return consume ? true : Viewer::key_press_event(key,mods);
    }
    bool key_release_event(int, int) override { ++releases; return true; }
    bool char_input_event(unsigned code) override {
        require(code==0x20ac,"Unicode code point changed"); ++chars; return true;
    }
    bool mouse_free_move_event(int x, int y, int a, int b, int) override {
        ++moves; mx=x; my=y; dx=a; dy=b; return true;
    }
    bool mouse_press_event(int x, int y, int button, int mods) override {
        ++buttons; mx=x; my=y; button_mods=mods;
        // A consumer of the press must also consume the rest of that gesture,
        // because the router records the held button before invoking the hook.
        swallow_gesture=consume;
        return consume ? true : Viewer::mouse_press_event(x,y,button,mods);
    }
    bool mouse_drag_event(int x, int y, int a, int b, int button, int mods) override {
        ++drags; mx=x; my=y; dx=a; dy=b;
        return swallow_gesture ? true : Viewer::mouse_drag_event(x,y,a,b,button,mods);
    }
    bool mouse_release_event(int x, int y, int button, int mods) override {
        ++buttons; return consume_release ? true : Viewer::mouse_release_event(x,y,button,mods);
    }
    bool mouse_scroll_event(int x, int y, int a, int sy) override {
        ++scrolls; last_scroll=sy; return true;
    }
    bool focus_event(bool) override { ++focus; return true; }
    bool drop_event(const std::vector<std::string>& files) override {
        require(files==std::vector<std::string>{"callbacks-drop.ply"},"drop filenames changed");
        ++drops;
        return consume ? true : Viewer::drop_event(files);
    }
};
std::shared_ptr<EventViewer> host(bool validation) {
#ifdef EASY3D_HAS_VULKAN
    {
        TestWindow o;
        o.width=181; o.height=151; o.visible=false;
        o.device.validation = validation;
        o.device.sync_validation = validation;
        auto v = std::make_shared<EventViewer>("Portable events", o.width, o.height, o.samples, false, o.visible, o.resizable, o.device);
        v->set_usage("");
        v->camera()->set_view_direction(vec3(0,0,-1)); v->camera()->set_up_vector(vec3(0,1,0));
        return v;
    }
#else
    auto v = std::make_shared<EventViewer>("Portable events", 181, 151, 4);
    v->hide();
    v->set_usage("");
    v->camera()->set_view_direction(vec3(0,0,-1)); v->camera()->set_up_vector(vec3(0,1,0));
    return v;
#endif
}
std::shared_ptr<SurfaceMesh> scene(EventViewer& v) {
    auto m=std::make_shared<SurfaceMesh>();
    const auto a=m->add_vertex(vec3(-1,-1,0)), b=m->add_vertex(vec3(1,-1,0)), c=m->add_vertex(vec3(0,1,0));
    m->add_triangle(a,b,c); v.add_model(m); v.fit_screen();
    m->renderer()->get_triangles_drawable("faces")->set_lighting(false); return m;
}
void frames_and_input(bool validation) {
    auto v=host(validation); auto mesh=scene(*v); Events e(v->window());
    auto faces=mesh->renderer()->get_triangles_drawable("faces");
    v->faces=faces;
    faces->set_uniform_coloring(vec4(.8f,.2f,.1f,1));
    // Render the farther blue surface last: missing depth initialization would
    // incorrectly overwrite the foreground in a snapshot before draw_frame().
    auto behind=std::make_shared<SurfaceMesh>();
    const auto a=behind->add_vertex(vec3(-1,-1,-.25f)), b=behind->add_vertex(vec3(1,-1,-.25f)), c=behind->add_vertex(vec3(0,1,-.25f));
    behind->add_triangle(a,b,c);v->add_model(behind);v->fit_screen();
    auto far_faces=behind->renderer()->get_triangles_drawable("faces");
    far_faces->set_lighting(false);far_faces->set_uniform_coloring(vec4(0,0,1,1));
    require(v->snapshot("portable-before-frame.png",1,4,0) && v->before==0 && v->after==0,"initial snapshot advanced frame hooks");
    int initial_w,initial_h,initial_channels;std::vector<unsigned char> initial;
    require(ImageIO::load("portable-before-frame.png",initial,initial_w,initial_h,initial_channels,4,false),"initial snapshot missing");
    const auto initial_center=(size_t(initial_h/2)*initial_w+initial_w/2)*4;
    require(std::abs(int(initial[initial_center])-204)<=2 && std::abs(int(initial[initial_center+1])-51)<=2,
            "snapshot before first frame lost foreground depth");
    require(v->run_frames(2)==0 && v->after==2 && v->before>=2,"frame hooks lost order or frame budget");
    const auto old_before=v->before,old_after=v->after;
    require(v->snapshot("portable-events.png",1,4,0),"hook-updated snapshot failed");
    int w,h,channels; std::vector<unsigned char> rgba;
    require(ImageIO::load("portable-events.png",rgba,w,h,channels,4,false),"hook output missing");
    const auto center=(size_t(h/2)*w+w/2)*4;
    for(unsigned i=0;i<3;++i) require(std::abs(int(rgba[center+i])-int(std::lround(v->color[i]*255)))<=2,"pre_draw changes missed the rendered frame");
    require(v->before==old_before && v->after==old_after,"snapshot invoked interactive frame hooks");

    unsigned bound=0,actions=0;
    v->bind_shortcut([&](Viewer*,Model*) { ++bound;return true; },mesh.get(),ViewerBase::KEY_J);
    v->bind_shortcut([&](Viewer*) { ++actions;return true; },ViewerBase::KEY_K);
    const auto projection=v->camera()->type();
    e.key(e.window,Viewer::KEY_P,0,GLFW_PRESS,0); e.key(e.window,Viewer::KEY_P,0,GLFW_RELEASE,0);
    require(v->keys==1 && v->releases==1 && v->camera()->type()==projection,"consumed key reached builtin controls");
    e.key(e.window,Viewer::KEY_J,0,GLFW_PRESS,0); require(bound==0,"consumed key reached bound command");
    e.key(e.window,Viewer::KEY_K,0,GLFW_PRESS,0); require(actions==0,"consumed key reached model-free action");
    v->consume=false;
    e.key(e.window,Viewer::KEY_P,0,GLFW_PRESS,0); require(v->camera()->type()!=projection,"unhandled key did not reach builtin controls");
    e.key(e.window,Viewer::KEY_J,0,GLFW_PRESS,0); require(bound==1,"unhandled key missed bound command");
    e.key(e.window,Viewer::KEY_K,0,GLFW_PRESS,0); require(actions==1,"unhandled key missed model-free action");
    e.character(e.window,0x20ac); require(v->chars==1,"character hook missing");
    e.cursor(e.window,21.75,31.25); e.cursor(e.window,24.75,36.25);
    require(v->mx==24 && v->my==36 && v->dx==3 && v->dy==5 && v->moves>=2,"mouse movement was not in logical window units");

    v->consume=true;
    double px,py; glfwGetCursorPos(e.window,&px,&py);
    const auto camera=v->camera()->model_view_projection_matrix();
    e.button(e.window,Viewer::BUTTON_LEFT,GLFW_PRESS,Viewer::MODIF_SHIFT);
    require(v->mx==int(px) && v->my==int(py) && v->button_mods==Viewer::MODIF_SHIFT,"button event lost actual logical cursor/modifiers");
    e.cursor(e.window,30.75,40.25); e.cursor(e.window,33.75,44.25);
    require(v->mx==33 && v->my==44 && v->dx==3 && v->dy==4 && v->drags==2,"consumed drag coordinates/deltas changed");
    e.button(e.window,Viewer::BUTTON_LEFT,GLFW_RELEASE,0);
    require(v->buttons==2 && same(camera,v->camera()->model_view_projection_matrix()),"consumed gesture moved builtin camera");
    // Cancel a builtin region zoom by consuming its release.
    v->consume=false; v->consume_release=true;
    e.button(e.window,Viewer::BUTTON_LEFT,GLFW_PRESS,ViewerBase::MODIF_CTRL);
    e.cursor(e.window,70,80);
    const auto zoom_camera=v->camera()->model_view_projection_matrix();
    e.button(e.window,Viewer::BUTTON_LEFT,GLFW_RELEASE,ViewerBase::MODIF_CTRL);
    require(same(zoom_camera,v->camera()->model_view_projection_matrix()),"consumed release performed builtin zoom");
    v->consume_release=false;
    const auto rotating=v->camera()->model_view_projection_matrix();
    e.button(e.window,Viewer::BUTTON_LEFT,GLFW_PRESS,0);
    e.cursor(e.window,80,80);bool moved=!same(rotating,v->camera()->model_view_projection_matrix());
    e.cursor(e.window,86,84);moved=moved || !same(rotating,v->camera()->model_view_projection_matrix());
    e.button(e.window,Viewer::BUTTON_LEFT,GLFW_RELEASE,0);
    require(moved,"default drag did not resume after a consumed release");
    v->consume=true;
    e.button(e.window,Viewer::BUTTON_LEFT,GLFW_PRESS,0);
    const auto drag_count=v->drags;
    e.focus(e.window,GLFW_FALSE); e.cursor(e.window,40,40);
    require(v->focus>=1 && v->drags==drag_count,"focus loss retained a drag");
    e.scroll(e.window,0,.4); e.scroll(e.window,0,.4); e.scroll(e.window,0,.4);
    require(v->scrolls==1 && v->last_scroll==1,"fractional scroll was lost or dispatched prematurely");
    e.scroll(e.window,0,.8); require(v->scrolls==2 && v->last_scroll==1,"fractional scroll remainder was not retained");
    { std::ofstream file("callbacks-drop.ply"); file<<"ply\nformat ascii 1.0\nelement vertex 3\nproperty float x\nproperty float y\nproperty float z\nelement face 1\nproperty list uchar int vertex_indices\nend_header\n-1 -1 0\n1 -1 0\n0 1 0\n3 0 1 2\n";require(bool(file),"drop fixture could not be written"); }
    const char* file="callbacks-drop.ply"; const auto models=v->models().size();
    e.drop(e.window,1,&file); require(v->drops==1 && v->models().size()==models,"consumed drop reached model loading");
    v->consume=false; e.drop(e.window,1,&file); require(v->drops==2 && v->models().size()==models+1,"unhandled drop did not load model");
    v->resize(193,157);
    // The logical size can trail the framebuffer by one window-system round trip
    // (X11 with a window manager), so let the host report the settled value.
    for (int i = 0; i < 100 && v->resize_w != 193; ++i)
        require(v->run_frames(1)==0,"render after hook resize failed");
    require(v->resizes>=1 && v->resize_w==193 && v->resize_h==157,"logical resize notification missing");
    const auto resize_count=v->resizes;
    auto framebuffer=glfwSetFramebufferSizeCallback(e.window,nullptr); glfwSetFramebufferSizeCallback(e.window,framebuffer);
    require(framebuffer!=nullptr,"framebuffer resize callback missing");
    framebuffer(e.window,v->framebuffer_width(),v->framebuffer_height());
    require(v->resizes==resize_count,"duplicate framebuffer event repeated the resize hook");
    require(v->validation_errors()==0,"portable event rendering validation errors");
}
void failures(bool validation) {
    auto v=host(validation);scene(*v);Events e(v->window());
    const auto window=v->window();
    v->throw_in_pre_draw=true;
    require(v->run_frames(1)!=0 && v->window()==window,"frame hook error changed window");
    v->throw_in_pre_draw=false;
    require(v->run_frames(1)==0,"cleared frame hook error did not restore rendering");
    v->throw_on_key=true;
    e.key(e.window,Viewer::KEY_J,0,GLFW_PRESS,0); // must not unwind through C callback
    require(v->should_close() && v->run_frames(1)!=0 && v->window()==window,"input hook failure was hidden or switched hosts");
}
}
int main(int argc,char** argv) {
    try {
        initialize(false,false,false);bool validation=false;
        for(int i=1;i<argc;++i) { if(std::string(argv[i])=="--validation")validation=true; }
        frames_and_input(validation);failures(validation);
        std::cout<<"PASS: portable event hooks, logical coordinates, consumption, default controls, resize and exceptions\n";
    }catch(const std::exception& e){std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
