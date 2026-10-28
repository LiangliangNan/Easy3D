// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/vulkan_viewer.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/util/initializer.h>
#include <easy3d/util/resource.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace easy3d;
void require(bool ok,const char* message) { if (!ok) throw std::runtime_error(message); }
bool same_orientation(const quat& a,const quat& b) {
    float error=0;
    for (unsigned i=0;i<4;++i) error+=std::abs(a[i]-b[i]);
    return error<1e-6f;
}
struct InputViewer : VulkanViewer {
    using VulkanViewer::VulkanViewer;
    using VulkanViewer::mouse_press_event;
    using VulkanViewer::mouse_drag_event;
    using VulkanViewer::mouse_release_event;
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
        VulkanViewerOptions options; options.visible=false; options.width=241; options.height=181;
        options.device.validation=options.device.synchronization_validation=argc>1;
        InputViewer v("Input parity",options);
        auto key=glfwSetKeyCallback(v.window(),nullptr); glfwSetKeyCallback(v.window(),key);
        auto press=[&](int k,int mods=0){key(v.window(),k,0,GLFW_PRESS,mods);};
        auto release=[&](int k){key(v.window(),k,0,GLFW_RELEASE,0);};
        auto* first=v.add_model(resource::directory()+"/data/bunny.ply");
        auto* second=v.add_model(resource::directory()+"/data/bunny.ply");
        require(first && second && first!=second && v.current_model()==second,"model setup failed");
        press(GLFW_KEY_COMMA); require(v.current_model()==first,"previous-model shortcut failed");
        press(GLFW_KEY_PERIOD); require(v.current_model()==second,"next-model shortcut failed");
        press(GLFW_KEY_PERIOD); require(v.current_model()==first,"model cycling did not wrap");
        v.fit_screen();
        const auto image=v.capture();
        size_t hit=0; while (hit<image.ids.size() && image.ids[hit].object==UINT32_MAX) ++hit;
        require(hit<image.ids.size(),"pivot test scene is empty");
        const int px=int(hit%image.width),py=int(hit/image.width);
        bool found=false; const auto picked=v.point_under_pixel(px,py,found);
        require(found,"pivot depth pick failed");
        v.mouse_press_event(px,py,VulkanViewer::BUTTON_LEFT,VulkanViewer::MODIF_SHIFT);
        require((v.camera()->pivotPoint()-picked).norm()<1e-6f,"Shift-click selected wrong surface pivot");
        v.mouse_release_event(px,py,VulkanViewer::BUTTON_LEFT,VulkanViewer::MODIF_SHIFT);
        const auto before_focus=v.camera()->position(); press(GLFW_KEY_Z);
        v.mouse_press_event(px,py,VulkanViewer::BUTTON_LEFT,0);
        require((v.camera()->position()-picked).norm()<(before_focus-picked).norm(),"Z-click did not approach picked surface");
        v.mouse_release_event(px,py,VulkanViewer::BUTTON_LEFT,0); release(GLFW_KEY_Z); v.fit_screen();
        auto* renderer=first->renderer();
        auto* faces=renderer->get_triangles_drawable("faces");
        auto* vertices=renderer->get_points_drawable("vertices");
        auto* edges=renderer->get_lines_drawable("edges");
        const bool smooth=faces->smooth_shading(); press(GLFW_KEY_M); require(faces->smooth_shading()!=smooth,"shading toggle failed");
        press(GLFW_KEY_M,GLFW_MOD_SHIFT); require(faces->smooth_shading()!=smooth,"modified M triggered plain shortcut");
        bool visible=vertices->is_visible(); press(GLFW_KEY_V); require(vertices->is_visible()!=visible,"vertex visibility failed");
        visible=edges->is_visible(); press(GLFW_KEY_E); require(edges->is_visible()!=visible,"edge visibility failed");
        const float size=vertices->point_size(),width=edges->line_width();
        press(GLFW_KEY_EQUAL); press(GLFW_KEY_RIGHT_BRACKET);
        require(vertices->point_size()==size+1 && edges->line_width()==width+1,"size shortcuts failed");
        for (unsigned i=0;i<10;++i) {press(GLFW_KEY_MINUS);press(GLFW_KEY_LEFT_BRACKET);}
        require(vertices->point_size()==1 && edges->line_width()==1,"size shortcuts went below one");
        const auto type=v.camera()->type(); press(GLFW_KEY_P,GLFW_MOD_SHIFT);
        require(v.camera()->type()==type,"modified P changed projection");
        press(GLFW_KEY_P); require(v.camera()->type()!=type,"projection shortcut failed"); press(GLFW_KEY_P);
        const auto orientation=v.camera()->orientation(); press(GLFW_KEY_LEFT);
        require(!same_orientation(v.camera()->orientation(),orientation),"arrow turn failed");
        auto pos=v.camera()->position(); press(GLFW_KEY_UP);
        require(v.camera()->position()!=pos,"arrow movement failed");
        pos=v.camera()->position(); press(GLFW_KEY_RIGHT,VulkanViewer::MODIF_CTRL);
        require(v.camera()->position()!=pos,"modified arrow movement failed");
        v.fit_screen(); pos=v.camera()->position();
        v.mouse_press_event(20,20,VulkanViewer::BUTTON_LEFT,VulkanViewer::MODIF_CTRL);
        v.mouse_drag_event(120,100,100,80,VulkanViewer::BUTTON_LEFT,VulkanViewer::MODIF_CTRL);
        require(v.camera()->position()==pos,"region drag moved the camera before release");
        v.mouse_release_event(120,100,VulkanViewer::BUTTON_LEFT,VulkanViewer::MODIF_CTRL);
        require(v.camera()->position()!=pos,"region zoom did not fit selected region");
        v.fit_screen();
        v.mouse_press_event(20,20,VulkanViewer::BUTTON_RIGHT,0); press(GLFW_KEY_X);
        pos=v.camera()->position(); v.mouse_drag_event(20,50,0,30,VulkanViewer::BUTTON_RIGHT,0);
        require(v.camera()->position()==pos,"horizontal constraint allowed vertical pan");
        release(GLFW_KEY_X); v.mouse_drag_event(20,80,0,30,VulkanViewer::BUTTON_RIGHT,0);
        require(v.camera()->position()!=pos,"key release left the axis constraint active");
        press(GLFW_KEY_Y); pos=v.camera()->position();
        v.mouse_drag_event(50,80,30,0,VulkanViewer::BUTTON_RIGHT,0);
        require(v.camera()->position()==pos,"vertical constraint allowed horizontal pan");
        release(GLFW_KEY_Y); v.mouse_release_event(50,80,VulkanViewer::BUTTON_RIGHT,0);
        v.mouse_press_event(20,20,VulkanViewer::BUTTON_MIDDLE,0);
        pos=v.camera()->position(); auto ori=v.camera()->orientation();
        v.mouse_drag_event(20,50,0,30,VulkanViewer::BUTTON_MIDDLE,0);
        require(v.camera()->position()!=pos && same_orientation(v.camera()->orientation(),ori),"middle drag did not zoom");
        v.mouse_release_event(20,50,VulkanViewer::BUTTON_MIDDLE,0);
        v.camera()->setPivotPoint(vec3(1,2,3));
        v.mouse_press_event(20,20,VulkanViewer::BUTTON_RIGHT,VulkanViewer::MODIF_SHIFT);
        require(v.camera()->pivotPoint()==v.camera()->sceneCenter(),"Shift-right pivot reset failed");
        v.mouse_release_event(20,20,VulkanViewer::BUTTON_RIGHT,VulkanViewer::MODIF_SHIFT);
        v.camera()->setPivotPoint(vec3(1,2,3));
        v.mouse_press_event(-10,-10,VulkanViewer::BUTTON_LEFT,VulkanViewer::MODIF_SHIFT);
        require(v.camera()->pivotPoint()==v.camera()->sceneCenter(),"empty Shift-click did not reset pivot");
        v.mouse_release_event(-10,-10,VulkanViewer::BUTTON_LEFT,VulkanViewer::MODIF_SHIFT);
        auto focus=glfwSetWindowFocusCallback(v.window(),nullptr);glfwSetWindowFocusCallback(v.window(),focus);
        press(GLFW_KEY_X); focus(v.window(),GLFW_FALSE); pos=v.camera()->position();
        v.mouse_press_event(20,20,VulkanViewer::BUTTON_RIGHT,0);
        v.mouse_drag_event(20,50,0,30,VulkanViewer::BUTTON_RIGHT,0);
        require(v.camera()->position()!=pos,"focus loss left a held constraint active");
        v.mouse_release_event(20,50,VulkanViewer::BUTTON_RIGHT,0);
        {
            ClipboardRestore clipboard(v.window());
            const auto copied=v.camera()->position(); press(GLFW_KEY_C,VulkanViewer::MODIF_CTRL);
            v.camera()->setPosition(copied+vec3(1)); press(GLFW_KEY_V,VulkanViewer::MODIF_CTRL);
            require((v.camera()->position()-copied).norm()<1e-5f,"clipboard camera roundtrip failed");
            glfwSetClipboardString(v.window(),"bad camera"); press(GLFW_KEY_V,VulkanViewer::MODIF_CTRL);
            require((v.camera()->position()-copied).norm()<1e-5f,"invalid clipboard changed camera");
        }
        press(GLFW_KEY_O,VulkanViewer::MODIF_CTRL); press(GLFW_KEY_S,VulkanViewer::MODIF_CTRL); press(GLFW_KEY_S);
        require(v.opened==1 && v.saved==1 && v.snapshots==1,"file shortcuts did not dispatch virtual hooks");
        bool bound=false;
        v.bind([&](VulkanViewer* viewer,Model* model){bound=viewer==&v && model==first;return true;},
               first,VulkanViewer::KEY_P,VulkanViewer::MODIF_SHIFT);
        press(GLFW_KEY_P,GLFW_MOD_SHIFT); require(bound,"custom modifier binding not dispatched");
        release(GLFW_KEY_P);
        press(GLFW_KEY_DELETE); require(v.models().size()==1 && v.current_model()==second,"delete-current-model failed");
        v.clear_scene(); require(v.current_model()==nullptr,"clearing scene left dangling current model");
        require(v.device().validation_errors()==0,"input validation errors");
        press(GLFW_KEY_F4,GLFW_MOD_ALT); require(v.should_close(),"Alt-F4 did not close");
        std::cout << "PASS: camera/model shortcuts, modifiers, axes, region/middle zoom, focus, bindings and dialogs\n";
    } catch (const std::exception& e) { std::cerr << "FAIL: " << e.what() << '\n'; return 1; }
}
