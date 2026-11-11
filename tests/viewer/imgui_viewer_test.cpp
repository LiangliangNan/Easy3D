// SPDX-License-Identifier: GPL-3.0-or-later
#include <tutorials/Tutorial_202_Viewer_imgui/viewer.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/util/initializer.h>
#include <3rd_party/imgui/imgui.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include "test_window.h"
#include <cmath>
#include <cstring>
#include <iostream>
#include <stdexcept>
using namespace easy3d;
void require(bool ok,const char* text) { if (!ok) throw std::runtime_error(text); }
struct ImGuiViewer : ViewerImGui {
    using ViewerImGui::ViewerImGui;
    bool probe = true, input = false, large_probe = false, cover_logo = false;
    char text[32] = {};
    ImGuiContext* context() const { return context_; }
    float bar_height() const { return menu_height_; }

    void draw_gui() override {
        if (input) {
            ImGui::SetNextWindowPos(ImVec2(140,60),ImGuiCond_Always);
            ImGui::SetNextWindowSize(ImVec2(170,100),ImGuiCond_Always);
            ImGui::Begin("Keyboard regression");
            if (!ImGui::IsAnyItemActive()) ImGui::SetKeyboardFocusHere();
            ImGui::InputText("Text",text,sizeof(text));
            ImGui::End();
        }
        if (cover_logo)
            ImGui::GetForegroundDrawList()->AddRectFilled(ImVec2(20,23),ImVec2(125,40),IM_COL32(255,0,255,255));
        if (probe) {
            auto* list=ImGui::GetForegroundDrawList();
            list->PushClipRect(ImVec2(40,90),ImVec2(100,130),true);
            list->AddRectFilled(ImVec2(0,0),ImVec2(500,500),IM_COL32(255,0,0,255));
            if (large_probe) {
                // Cross the 16-bit index boundary. Initial geometry is clipped out;
                // only the final sentinel should appear after the VtxOffset change.
                for(int i=0;i<17000;++i)
                    list->AddRectFilled(ImVec2(0,0),ImVec2(1,1),IM_COL32(255,0,0,255));
                list->AddRectFilled(ImVec2(41,91),ImVec2(99,129),IM_COL32(0,255,0,255));
            }
            list->PopClipRect();
        }
        ViewerImGui::draw_gui();
    }
};
bool same(const mat4& a,const mat4& b) {
    for(unsigned i=0;i<16;++i) if(std::abs(a[i]-b[i])>1e-6f) return false;
    return true;
}
void run(bool validation) {
#ifdef EASY3D_HAS_VULKAN
    VulkanOptions options;options.validation=options.sync_validation=validation;
#else
    OpenGLOptions options;
#endif
    ImGuiViewer v("ImGui backend regression",800,600,1,false,true,true,options);
    v.show(); // OpenGL Viewer normally shows its window in run(), not draw_frame().
#ifdef EASY3D_HAS_VULKAN
    TestWindow size;size.width=320;size.height=240;set_size(v,size);
#else
    v.resize(320,240);for(int i=0;i<5;++i) Viewer::poll_events();
#endif
    auto mesh=std::make_shared<SurfaceMesh>();
    auto a=mesh->add_vertex(vec3(-1,-1,0)),b=mesh->add_vertex(vec3(1,-1,0)),c=mesh->add_vertex(vec3(0,1,0));
    mesh->add_triangle(a,b,c);v.add_model(mesh);
    auto* faces=mesh->renderer()->get_triangles_drawable("faces");
    faces->set_lighting(false);faces->set_uniform_coloring(vec4(0,0,1,1));
    v.camera()->setViewDirection(vec3(0,0,-1));v.camera()->setUpVector(vec3(0,1,0));v.fit_screen();
    v.set_background_color(vec4(.1f,.2f,.3f,1));
    require(v.draw_frame(),"ImGui first frame failed");
    require(v.context(),"ImGui context was not created");
    ImGui::SetCurrentContext(v.context());
    ImGui::GetIO().ConfigInputTrickleEventQueue=false; // synthetic events have explicit frame boundaries
    require(ImGui::GetDrawData() && ImGui::GetDrawData()->TotalVtxCount>0,"ImGui did not produce menu geometry");
    const int closed_vertices=ImGui::GetDrawData()->TotalVtxCount;
    const float menu_y=v.bar_height()*.5f;
    glfwFocusWindow(v.window());glfwSetCursorPos(v.window(),15,menu_y);
    auto cursor=glfwSetCursorPosCallback(v.window(),nullptr);glfwSetCursorPosCallback(v.window(),cursor);
    auto enter=glfwSetCursorEnterCallback(v.window(),nullptr);glfwSetCursorEnterCallback(v.window(),enter);
    auto focus=glfwSetWindowFocusCallback(v.window(),nullptr);glfwSetWindowFocusCallback(v.window(),focus);
    require(cursor && enter && focus,"GUI pointer/focus callbacks missing");
    for(int i=0;i<100;++i) {
        Viewer::poll_events();
        // Feed a complete synthetic pointer sequence. OS focus/enter notifications
        // can lag replacement windows; do not mix a synthetic click with stale hover.
        focus(v.window(),GLFW_TRUE);glfwSetCursorPos(v.window(),15,menu_y);
        enter(v.window(),GLFW_TRUE);cursor(v.window(),15,menu_y);
        require(v.draw_frame(),"GUI hover frame failed");
        if(ImGui::GetIO().WantCaptureMouse && std::abs(ImGui::GetIO().MousePos.x-15)<.5f && std::abs(ImGui::GetIO().MousePos.y-menu_y)<.5f) break;
        glfwWaitEventsTimeout(.01);
    }
    require(ImGui::GetIO().WantCaptureMouse,"GUI menu hover did not settle");
    auto button=glfwSetMouseButtonCallback(v.window(),nullptr);glfwSetMouseButtonCallback(v.window(),button);
    require(button,"GUI button callback missing");
    glfwSetCursorPos(v.window(),15,menu_y);cursor(v.window(),15,menu_y);
    button(v.window(),Viewer::BUTTON_LEFT,GLFW_PRESS,0);v.draw_frame();
    require(ImGui::GetIO().MouseDown[0],"GUI button press did not reach ImGui");
    glfwSetCursorPos(v.window(),15,menu_y);cursor(v.window(),15,menu_y);
    button(v.window(),Viewer::BUTTON_LEFT,GLFW_RELEASE,0);v.draw_frame();
    if (!ImGui::IsPopupOpen(nullptr,ImGuiPopupFlags_AnyPopup))
        std::cerr<<"GUI pointer: "<<ImGui::GetIO().MousePos.x<<","<<ImGui::GetIO().MousePos.y
                 <<" menu height="<<v.bar_height()<<" focus="<<glfwGetWindowAttrib(v.window(),GLFW_FOCUSED)
                 <<" capture="<<ImGui::GetIO().WantCaptureMouse<<" down="<<ImGui::GetIO().MouseDown[0]<<'\n';
    require(ImGui::IsPopupOpen(nullptr,ImGuiPopupFlags_AnyPopup),"File menu did not open through the GLFW callback chain");
    require(v.draw_frame(),"GUI popup layout frame failed");
    require(ImGui::GetDrawData()->TotalVtxCount>closed_vertices,"File menu did not produce popup geometry");
    auto escape=glfwSetKeyCallback(v.window(),nullptr);glfwSetKeyCallback(v.window(),escape);
    escape(v.window(),Viewer::KEY_ESCAPE,0,GLFW_PRESS,0);escape(v.window(),Viewer::KEY_ESCAPE,0,GLFW_RELEASE,0);
    require(v.draw_frame() && !v.should_close(),"closing the menu escaped to Viewer");
    v.input=true;
    for(int i=0;i<5;++i) {require(v.draw_frame(),"text input focus frame failed");if(ImGui::GetIO().WantTextInput) break;}
    require(ImGui::GetIO().WantTextInput,"GUI text field did not take focus");
    auto character=glfwSetCharCallback(v.window(),nullptr);glfwSetCharCallback(v.window(),character);
    require(character,"GUI character callback missing");
    character(v.window(),'A');character(v.window(),0x03a9);v.draw_frame();
    require(std::strstr(v.text,"A") && std::strstr(v.text,"\xce\xa9"),"UTF-8 text entry did not reach ImGui");
    v.input=false;v.draw_frame();
    const auto matrix=v.camera()->modelViewProjectionMatrix();
    auto key=glfwSetKeyCallback(v.window(),nullptr);glfwSetKeyCallback(v.window(),key);
    auto scroll=glfwSetScrollCallback(v.window(),nullptr);glfwSetScrollCallback(v.window(),scroll);
    require(key && scroll,"ImGui callback chain missing");
    const auto projection=v.camera()->type();
    ImGui::GetIO().WantCaptureKeyboard=true;
    key(v.window(),Viewer::KEY_P,0,GLFW_PRESS,0);key(v.window(),Viewer::KEY_P,0,GLFW_RELEASE,0);
    require(v.camera()->type()==projection,"GUI keyboard capture leaked to Viewer");
    ImGui::GetIO().WantCaptureMouse=true;scroll(v.window(),0,1);
    require(same(matrix,v.camera()->modelViewProjectionMatrix()),"GUI mouse capture moved the camera");
    ImGui::GetIO().WantCaptureKeyboard=false;
    key(v.window(),Viewer::KEY_P,0,GLFW_PRESS,0);key(v.window(),Viewer::KEY_P,0,GLFW_RELEASE,0);
    require(v.camera()->type()!=projection,"uncaptured Viewer shortcut was lost");
    v.camera()->setType(projection);
    v.large_probe=true;require(v.draw_frame(),"large GUI draw list failed");
#ifdef EASY3D_HAS_VULKAN
    const std::string large_file="vulkan-imgui-large.png";
#else
    const std::string large_file="opengl-imgui-large.png";
#endif
    require(v.snapshot(large_file),"large GUI snapshot failed");
    int iw=0,ih=0,channels=0;std::vector<unsigned char> pixels;
    require(ImageIO::load(large_file,pixels,iw,ih,channels,4,false),"GUI snapshot could not be decoded");
    const auto sentinel=(size_t(100*ih/v.height())*iw+size_t(50*iw/v.width()))*4;
    require(pixels[sentinel]<3 && pixels[sentinel+1]>250 && pixels[sentinel+2]<3,
            "large GUI vertex offsets corrupted the sentinel geometry");
    const auto clipped=(size_t(100*ih/v.height())*iw+size_t(110*iw/v.width()))*4;
    require(!(pixels[clipped]<3 && pixels[clipped+1]>250 && pixels[clipped+2]<3),
            "large GUI geometry escaped clipping");
    const auto before_snapshot=v.camera()->modelViewProjectionMatrix();
    for(float scale:{.5f,1.5f,2.0f}) {
        const auto name="imgui-scaled-"+std::to_string(scale)+".png";
        require(v.snapshot(name,scale,1),"scaled GUI snapshot failed");
        require(ImageIO::load(name,pixels,iw,ih,channels,4,false),"scaled GUI output could not be decoded");
        const auto p=(size_t(100*ih/v.height())*iw+size_t(50*iw/v.width()))*4;
        require(pixels[p]<3 && pixels[p+1]>250 && pixels[p+2]<3,"GUI clip viewport did not scale with file output");
        const auto q=(size_t(100*ih/v.height())*iw+size_t(110*iw/v.width()))*4;
        require(!(pixels[q]<3 && pixels[q+1]>250 && pixels[q+2]<3),"scaled GUI clip rectangle leaked");
        require(same(before_snapshot,v.camera()->modelViewProjectionMatrix()),"GUI file output changed the live camera");
    }
    v.large_probe=false;
#ifdef EASY3D_HAS_VULKAN
    v.cover_logo=true;require(v.draw_frame(),"GUI foreground frame failed");
    const auto front=v.capture(1);
    const double logo_sx=double(front.width)/v.width(),logo_sy=double(front.height)/v.height();
    for(int y=int(26*logo_sy);y<int(34*logo_sy);++y)
        for(int x=int(25*logo_sx);x<int(115*logo_sx);++x) {
            const auto i=(size_t(y)*front.width+x)*4;
            require(front.rgba[i]>250 && front.rgba[i+1]<3 && front.rgba[i+2]>250,
                    "built-in logo overpainted foreground GUI content");
        }
    v.cover_logo=false;
    for(unsigned samples:{1u,4u}) {
        if(!(v.device().supported_sample_counts()&samples)) continue;
        v.probe=true;require(v.draw_frame(),"ImGui probe frame failed");const auto with=v.capture(samples);
        v.probe=false;require(v.draw_frame(),"ImGui normal frame failed");const auto without=v.capture(samples);
        require(with.depth==without.depth,"ImGui changed scene depth");
        for(size_t i=0;i<with.ids.size();++i)
            require(with.ids[i].object==without.ids[i].object && with.ids[i].primitive==without.ids[i].primitive,
                    "ImGui changed scene IDs");
        const double sx=double(with.width)/v.width(),sy=double(with.height)/v.height();
        const auto inside=(size_t(100*sy)*with.width+size_t(50*sx))*4;
        const auto outside=(size_t(100*sy)*with.width+size_t(110*sx))*4;
        require(with.rgba[inside]>250 && with.rgba[inside+1]<3 && with.rgba[inside+2]<3,"GUI clip rectangle lost its contents");
        for(unsigned channel=0;channel<4;++channel)
            require(with.rgba[outside+channel]==without.rgba[outside+channel],"GUI drawing escaped its clip rectangle");
    }
    v.probe=true;size.width=400;size.height=280;set_size(v,size);require(v.draw_frame(),"resized GUI frame failed");
    require(v.snapshot("vulkan-imgui.png"),"GUI snapshot failed");
    require(v.device().validation_errors()==0,"ImGui produced Vulkan validation errors");
#else
    require(v.draw_frame(),"ImGui second frame failed");
    require(v.snapshot("opengl-imgui.png"),"ImGui snapshot failed");
#endif
}
int main(int argc,char**) {
    try {
        initialize(false,false,false);
        { ImGuiViewer uninitialized("Unused ImGui viewer"); }
        run(argc>1);run(false);
        std::cout<<"PASS: ImGui menus, input capture, draw clipping, resize, overlays and context lifetime\n";
    } catch(const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
