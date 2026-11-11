#include "test_window.h"
// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/viewer.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/fileio/surface_mesh_io.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/util/initializer.h>
#include <easy3d/util/resource.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <chrono>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <thread>
using namespace easy3d;
void require(bool ok,const char* message) { if (!ok) throw std::runtime_error(message); }
bool same_orientation(const quat& a,const quat& b) {
    float error=0;
    for (unsigned i=0;i<4;++i) error+=std::abs(a[i]-b[i]);
    return error<1e-6f;
}
struct InputViewer : Viewer {
    using Viewer::Viewer;
    using Viewer::mouse_press_event;
    using Viewer::mouse_drag_event;
    using Viewer::mouse_release_event;
    unsigned opened=0;
    mutable unsigned saved=0, snapshots=0;
    bool open() override { ++opened; return true; }
    bool save() const override { ++saved; return true; }
    bool snapshot() const override { ++snapshots; return true; }
};
struct ClipboardRestore {
    GLFWwindow* window;
    std::string value;
    explicit ClipboardRestore(GLFWwindow* w) : window(w) {
        const char* text=glfwGetClipboardString(w); if (text) value=text;
    }
    ~ClipboardRestore() { glfwSetClipboardString(window,value.c_str()); }
};
int main(int argc,char**) {
    try {
        initialize(false,false,false);
#ifdef EASY3D_HAS_VULKAN
        TestWindow options; options.visible=false; options.width=241; options.height=181;
        options.device.validation=options.device.sync_validation=argc>1;
        InputViewer v("Input parity", options.width, options.height, options.samples, false, options.visible, options.resizable, options.device);
#else
        (void)argc;
        InputViewer v("Input parity", 241, 181, 1);
        v.hide();
#endif
        auto key=glfwSetKeyCallback(v.window(),nullptr); glfwSetKeyCallback(v.window(),key);
        auto press=[&](int k,int mods=0){key(v.window(),k,0,GLFW_PRESS,mods);};
        auto release=[&](int k){key(v.window(),k,0,GLFW_RELEASE,0);};
        // Two distinct in-memory meshes: the file overload deduplicates by name on OpenGL,
        // while add_model(shared_ptr) deduplicates by pointer identity on both backends.
        const std::string bunny=resource::directory()+"/data/bunny.ply";
        std::shared_ptr<Model> mesh_a(SurfaceMeshIO::load(bunny));
        std::shared_ptr<Model> mesh_b(SurfaceMeshIO::load(bunny));
        auto* first=v.add_model(mesh_a);
        auto* second=v.add_model(mesh_b);
        require(first && second && first!=second && v.current_model()==second,"model setup failed");
        press(InputViewer::KEY_COMMA); require(v.current_model()==first,"previous-model shortcut failed");
        press(InputViewer::KEY_PERIOD); require(v.current_model()==second,"next-model shortcut failed");
        press(InputViewer::KEY_PERIOD); require(v.current_model()==first,"model cycling did not wrap");
        v.fit_screen();
        const auto image=v.capture();
        // Choose a covered pixel nearest the image center: an interior surface point survives
        // the logical->framebuffer quantization that legacy hooks apply on HiDPI displays.
        size_t hit=0; double best=-1;
        const double cx=image.width*0.5, cy=image.height*0.5;
        for (size_t i=0;i<image.ids.size();++i) {
            if (image.ids[i].object==UINT32_MAX) continue;
            const double dx=double(i%image.width)-cx, dy=double(i/image.width)-cy, d=dx*dx+dy*dy;
            if (best<0 || d<best) { best=d; hit=i; }
        }
        require(best>=0,"pivot test scene is empty");
        const int px=int(hit%image.width),py=int(hit/image.width);
        // pick() and the legacy mouse hooks both use logical window coordinates on
        // both backends. Map the framebuffer hit pixel to its logical cursor once.
        const int mx=int(double(px)*v.width()/v.framebuffer_width());
        const int my=int(double(py)*v.height()/v.framebuffer_height());
        const auto surface=v.pick(mx,my); const bool found=bool(surface); const auto picked=surface.position;
        require(found,"pivot depth pick failed");
        const vec3 hook_target=picked;
        v.mouse_press_event(mx,my,Viewer::BUTTON_LEFT,Viewer::MODIF_SHIFT);
        require((v.camera()->pivotPoint()-hook_target).norm()<1e-6f,"Shift-click selected wrong surface pivot");
        v.mouse_release_event(mx,my,Viewer::BUTTON_LEFT,Viewer::MODIF_SHIFT);
        const auto before_focus=v.camera()->position(); press(InputViewer::KEY_Z);
        v.mouse_press_event(mx,my,Viewer::BUTTON_LEFT,0);
        const auto expected_focus=0.1f*before_focus+0.9f*hook_target;
        const auto animation_deadline=std::chrono::steady_clock::now()+std::chrono::seconds(2);
        bool approached=false;
        while ((v.camera()->position()-expected_focus).norm()>=1e-5f &&
               std::chrono::steady_clock::now()<animation_deadline) {
            approached=approached ||
                       (v.camera()->position()-hook_target).norm()<(before_focus-hook_target).norm();
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            v.draw_frame();
        }
        require(approached,"Z-click did not approach picked surface");
        require((v.camera()->position()-expected_focus).norm()<1e-5f,"Z-click camera animation did not reach its target");
        v.mouse_release_event(mx,my,Viewer::BUTTON_LEFT,0); release(InputViewer::KEY_Z); v.fit_screen();
        auto* renderer=first->renderer();
        auto* faces=renderer->get_triangles_drawable("faces");
        auto* vertices=renderer->get_points_drawable("vertices");
        auto* edges=renderer->get_lines_drawable("edges");
        const bool smooth=faces->smooth_shading(); press(InputViewer::KEY_M); require(faces->smooth_shading()!=smooth,"shading toggle failed");
        press(InputViewer::KEY_M,Viewer::MODIF_SHIFT); require(faces->smooth_shading()!=smooth,"modified M triggered plain shortcut");
        bool visible=vertices->is_visible(); press(InputViewer::KEY_V); require(vertices->is_visible()!=visible,"vertex visibility failed");
        visible=edges->is_visible(); press(InputViewer::KEY_E); require(edges->is_visible()!=visible,"edge visibility failed");
        const float size=vertices->point_size(),width=edges->line_width();
        press(InputViewer::KEY_EQUAL); press(InputViewer::KEY_RIGHT_BRACKET);
        require(vertices->point_size()==size+1 && edges->line_width()==width+1,"size shortcuts failed");
        for (unsigned i=0;i<10;++i) {press(InputViewer::KEY_MINUS);press(InputViewer::KEY_LEFT_BRACKET);}
        require(vertices->point_size()==1 && edges->line_width()==1,"size shortcuts went below one");
        const auto type=v.camera()->type(); press(InputViewer::KEY_P,Viewer::MODIF_SHIFT);
        require(v.camera()->type()==type,"modified P changed projection");
        press(InputViewer::KEY_P); require(v.camera()->type()!=type,"projection shortcut failed"); press(InputViewer::KEY_P);
        const auto orientation=v.camera()->orientation(); press(InputViewer::KEY_LEFT);
        require(!same_orientation(v.camera()->orientation(),orientation),"arrow turn failed");
        auto pos=v.camera()->position(); press(InputViewer::KEY_UP);
        require(v.camera()->position()!=pos,"arrow movement failed");
        pos=v.camera()->position(); press(InputViewer::KEY_RIGHT,Viewer::MODIF_CTRL);
        require(v.camera()->position()!=pos,"modified arrow movement failed");
        v.fit_screen(); pos=v.camera()->position();
        v.mouse_press_event(20,20,Viewer::BUTTON_LEFT,Viewer::MODIF_CTRL);
        v.mouse_drag_event(120,100,100,80,Viewer::BUTTON_LEFT,Viewer::MODIF_CTRL);
        require(v.camera()->position()==pos,"region drag moved the camera before release");
        v.mouse_release_event(120,100,Viewer::BUTTON_LEFT,Viewer::MODIF_CTRL);
        require(v.camera()->position()!=pos,"region zoom did not fit selected region");
        v.fit_screen();
        v.mouse_press_event(20,20,Viewer::BUTTON_RIGHT,0); press(InputViewer::KEY_X);
        pos=v.camera()->position(); v.mouse_drag_event(20,50,0,30,Viewer::BUTTON_RIGHT,0);
        require(v.camera()->position()==pos,"horizontal constraint allowed vertical pan");
        release(InputViewer::KEY_X); v.mouse_drag_event(20,80,0,30,Viewer::BUTTON_RIGHT,0);
        require(v.camera()->position()!=pos,"key release left the axis constraint active");
        press(InputViewer::KEY_Y); pos=v.camera()->position();
        v.mouse_drag_event(50,80,30,0,Viewer::BUTTON_RIGHT,0);
        require(v.camera()->position()==pos,"vertical constraint allowed horizontal pan");
        release(InputViewer::KEY_Y); v.mouse_release_event(50,80,Viewer::BUTTON_RIGHT,0);
        v.mouse_press_event(20,20,Viewer::BUTTON_MIDDLE,0);
        pos=v.camera()->position(); auto ori=v.camera()->orientation();
        v.mouse_drag_event(20,50,0,30,Viewer::BUTTON_MIDDLE,0);
        require(v.camera()->position()!=pos && same_orientation(v.camera()->orientation(),ori),"middle drag did not zoom");
        v.mouse_release_event(20,50,Viewer::BUTTON_MIDDLE,0);
        v.camera()->setPivotPoint(vec3(1,2,3));
        v.mouse_press_event(20,20,Viewer::BUTTON_RIGHT,Viewer::MODIF_SHIFT);
        require(v.camera()->pivotPoint()==v.camera()->sceneCenter(),"Shift-right pivot reset failed");
        v.mouse_release_event(20,20,Viewer::BUTTON_RIGHT,Viewer::MODIF_SHIFT);
        v.camera()->setPivotPoint(vec3(1,2,3));
        v.mouse_press_event(-10,-10,Viewer::BUTTON_LEFT,Viewer::MODIF_SHIFT);
        require(v.camera()->pivotPoint()==v.camera()->sceneCenter(),"empty Shift-click did not reset pivot");
        v.mouse_release_event(-10,-10,Viewer::BUTTON_LEFT,Viewer::MODIF_SHIFT);
        auto focus=glfwSetWindowFocusCallback(v.window(),nullptr);glfwSetWindowFocusCallback(v.window(),focus);
        press(InputViewer::KEY_X); focus(v.window(),GLFW_FALSE); pos=v.camera()->position();
        v.mouse_press_event(20,20,Viewer::BUTTON_RIGHT,0);
        v.mouse_drag_event(20,50,0,30,Viewer::BUTTON_RIGHT,0);
        require(v.camera()->position()!=pos,"focus loss left a held constraint active");
        v.mouse_release_event(20,50,Viewer::BUTTON_RIGHT,0);
        {
            ClipboardRestore clipboard(v.window());
            const auto copied=v.camera()->position(); press(InputViewer::KEY_C,Viewer::MODIF_CTRL);
            v.camera()->setPosition(copied+vec3(1)); press(InputViewer::KEY_V,Viewer::MODIF_CTRL);
            // Vulkan pastes immediately; OpenGL animates to the saved view over 0.5 s. Both
            // must restore it, so drive frames until arrival rather than assuming one timing.
            const auto paste_deadline=std::chrono::steady_clock::now()+std::chrono::seconds(2);
            while ((v.camera()->position()-copied).norm()>=1e-5f && std::chrono::steady_clock::now()<paste_deadline) {
                std::this_thread::sleep_for(std::chrono::milliseconds(20));
                v.draw_frame();
            }
            require((v.camera()->position()-copied).norm()<1e-5f,"clipboard camera roundtrip failed");
            glfwSetClipboardString(v.window(),"bad camera"); press(InputViewer::KEY_V,Viewer::MODIF_CTRL);
            require((v.camera()->position()-copied).norm()<1e-5f,"invalid clipboard changed camera");
        }
        press(InputViewer::KEY_O,Viewer::MODIF_CTRL); press(InputViewer::KEY_S,Viewer::MODIF_CTRL); press(InputViewer::KEY_S);
        require(v.opened==1 && v.saved==1 && v.snapshots==1,"file shortcuts did not dispatch virtual hooks");
        bool bound=false, action=false;
        v.bind_shortcut([&](Viewer* viewer,Model* model){bound=viewer==&v && model==first;return true;},
               first,Viewer::KEY_P,Viewer::MODIF_SHIFT);
        press(InputViewer::KEY_P,Viewer::MODIF_SHIFT); require(bound,"custom modifier binding not dispatched");
        release(InputViewer::KEY_P);
        bound=false;
        v.unbind_shortcut(Viewer::KEY_P,Viewer::MODIF_SHIFT);
        press(InputViewer::KEY_P,Viewer::MODIF_SHIFT); require(!bound,"unbind(key, modifier) left a binding active");
        release(InputViewer::KEY_P);
        // unbind(model) removes only the bindings that carry that model.
        v.bind_shortcut([&](Viewer* viewer){action=viewer==&v;return true;},Viewer::KEY_J,Viewer::MODIF_ALT);
        v.bind_shortcut([&](Viewer*,Model*){bound=true;return true;},second,Viewer::KEY_K,Viewer::MODIF_ALT);
        v.unbind_shortcut(second);
        press(InputViewer::KEY_K,Viewer::MODIF_ALT); require(!bound,"unbind(model) left a model binding active");
        press(InputViewer::KEY_J,Viewer::MODIF_ALT); require(action,"unbind(model) removed a model-free action");
        release(InputViewer::KEY_K); release(InputViewer::KEY_J);
        // A bound function may remove its own binding; the callable stays alive for the call.
        unsigned once=0;
        v.bind_shortcut([&](Viewer* self){++once;self->unbind_shortcut(Viewer::KEY_I,Viewer::MODIF_ALT);return true;},
               Viewer::KEY_I,Viewer::MODIF_ALT);
        press(InputViewer::KEY_I,Viewer::MODIF_ALT); press(InputViewer::KEY_I,Viewer::MODIF_ALT);
        release(InputViewer::KEY_I);
        require(once==1,"self-unbinding shortcut was not removed or crashed");
        // Deleting a model automatically removes the shortcuts that target it.
        bound=false;
        v.bind_shortcut([&](Viewer*,Model*){bound=true;return true;},first,Viewer::KEY_U,Viewer::MODIF_ALT);
        press(InputViewer::KEY_DELETE); require(v.models().size()==1 && v.current_model()==second,"delete-current-model failed");
        press(InputViewer::KEY_U,Viewer::MODIF_ALT); require(!bound,"delete_model kept a binding to the deleted model");
        release(InputViewer::KEY_U);
        // clear_scene() drops model-bound shortcuts but keeps model-free actions alive.
        v.bind_shortcut([&](Viewer*,Model*){bound=true;return true;},second,Viewer::KEY_L,Viewer::MODIF_ALT);
        v.clear_scene(); require(v.current_model()==nullptr,"clearing scene left dangling current model");
        press(InputViewer::KEY_L,Viewer::MODIF_ALT); require(!bound,"clear_scene kept a binding to a deleted model");
        action=false; press(InputViewer::KEY_J,Viewer::MODIF_ALT); require(action,"clear_scene removed a model-free action");
        release(InputViewer::KEY_L); release(InputViewer::KEY_J);
        require(v.validation_errors()==0,"input validation errors");
        press(InputViewer::KEY_F4,Viewer::MODIF_ALT); require(v.should_close(),"Alt-F4 did not close");
        std::cout << "PASS: camera/model shortcuts, modifiers, axes, region/middle zoom, focus, bindings and dialogs\n";
    } catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
