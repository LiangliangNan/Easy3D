// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/vulkan_viewer.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/manipulated_frame.h>
#include <easy3d/util/initializer.h>
#include <easy3d/util/resource.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <cmath>
#include <iostream>
#include <stdexcept>
using namespace easy3d;
void require(bool ok,const char* message) { if (!ok) throw std::runtime_error(message); }
bool same_matrix(const mat4& a,const mat4& b) {
    for (unsigned i=0;i<16;++i) if (a[i]!=b[i]) return false;
    return true;
}
void same_picking(const graphics::FrameImage& a,const graphics::FrameImage& b) {
    require(a.depth==b.depth && a.ids.size()==b.ids.size(),"guide changed scene depth");
    for (size_t i=0;i<a.ids.size();++i)
        require(a.ids[i].object==b.ids[i].object && a.ids[i].primitive==b.ids[i].primitive,
                "guide changed picking IDs");
}
unsigned hits(const graphics::FrameImage& image) {
    unsigned n=0; for (auto id:image.ids) n+=id.object!=UINT32_MAX; return n;
}
struct ClipViewer : VulkanViewer {
    using VulkanViewer::VulkanViewer;
    using VulkanViewer::mouse_press_event;
    using VulkanViewer::mouse_drag_event;
    using VulkanViewer::mouse_release_event;
};
int main(int argc,char**) {
    auto* plane=ClippingPlane::instance();
    try {
        initialize(false,false,false);
        VulkanViewerOptions options;options.visible=false;options.width=360;options.height=280;
        options.device.validation=options.device.synchronization_validation=argc>1;
        ClipViewer viewer("Shared clipping plane",options);
        viewer.set_background_color(vec4(1,1,1,1));
        viewer.set_usage("default","Shared clipping plane regression");
        require(viewer.add_model(resource::directory()+"/data/sphere.obj"),"missing sphere fixture");
        std::vector<graphics::Vertex> marker(3);
        marker[0].position=vec3(.7f,.7f,0);marker[1].position=vec3(.95f,.7f,0);marker[2].position=vec3(.7f,.95f,0);
        graphics::DrawCommand overlay;overlay.geometry=std::make_shared<graphics::Geometry>(marker);overlay.color=vec4(1,0,0,1);
        viewer.set_overlays({overlay});
        plane->set_enabled(false);plane->set_visible(true);plane->set_cross_section(false);
        plane->set_color(vec4(.2f,.4f,.8f,.3f));plane->set_cross_section_width(.12f);
        plane->manipulator()->frame()->setOrientation(viewer.camera()->orientation()*quat(vec3(0,1,0),.5f));
        auto key=glfwSetKeyCallback(viewer.window(),nullptr);glfwSetKeyCallback(viewer.window(),key);
        auto press=[&](int k,int m=GLFW_MOD_ALT){key(viewer.window(),k,0,GLFW_PRESS,m);};
        press(GLFW_KEY_V);press(GLFW_KEY_X);
        require(plane->is_visible() && !plane->cross_section(),"disabled plane responded to visibility/section shortcut");
        for (unsigned samples:{1u,4u}) {
            if (!(viewer.device().supported_sample_counts()&samples)) continue;
            viewer.set_samples(samples);const auto original=viewer.capture();
            press(GLFW_KEY_C,GLFW_MOD_ALT|GLFW_MOD_SHIFT);
            require(!plane->is_enabled(),"modified shortcut enabled clipping");
            const auto camera=viewer.camera()->modelViewProjectionMatrix();
            press(GLFW_KEY_C);require(plane->is_enabled(),"Alt-C did not enable shared clipping");
            require(same_matrix(camera,viewer.camera()->modelViewProjectionMatrix()),"Alt-C moved camera");
            require((plane->center()-viewer.camera()->sceneCenter()).norm()<1e-6f,"Alt-C did not fit plane");
            require(viewer.clipping_planes().size()==1,"shared plane not exposed to selection");
            const auto guided=viewer.capture();
            require(viewer.snapshot("vulkan-shared-clipping.png",1.5f,int(samples),1,false),"clipping snapshot failed");
            press(GLFW_KEY_V);require(!plane->is_visible(),"Alt-V did not hide guide");
            const auto clipped=viewer.capture();same_picking(guided,clipped);
            require(clipped.depth!=original.depth && hits(clipped)>0,"shared plane did not clip geometry");
            require(guided.rgba!=clipped.rgba,"transparent plane guide did not render");
            unsigned transparent=0;
            for (size_t i=0;i<guided.ids.size();++i) {
                if (guided.ids[i].object!=UINT32_MAX) continue;
                const auto r=guided.rgba[4*i],g=guided.rgba[4*i+1],b=guided.rgba[4*i+2];
                transparent+=std::abs(int(r)-194)<=2 && std::abs(int(g)-209)<=2 && std::abs(int(b)-240)<=2;
            }
            require(transparent>20,"guide fill did not alpha-blend over background");
            press(GLFW_KEY_X);require(plane->cross_section() && viewer.clipping_planes().size()==2,"Alt-X did not enable slab");
            const auto slab=viewer.capture();require(hits(slab)<hits(clipped),"cross-section did not narrow geometry");
            const auto& equations=viewer.clipping_planes();
            require(std::abs(dot(equations[0],vec4(plane->center(),1)))<1e-5f,"wrong plane offset");
            require(dot(equations[1],vec4(plane->center(),1))>0,"slab faces wrong direction");
            press(GLFW_KEY_X);press(GLFW_KEY_V);
            same_picking(guided,viewer.capture());
            press(GLFW_KEY_C);require(!plane->is_enabled(),"Alt-C did not disable plane");
            require(viewer.capture().rgba==original.rgba,"disabling clipping did not restore scene");
        }
        press(GLFW_KEY_C);
        auto* frame=plane->manipulator()->frame();
        const auto camera=viewer.camera()->modelViewProjectionMatrix(),pose=frame->matrix();
        viewer.mouse_press_event(170,130,ViewerInput::BUTTON_LEFT,ViewerInput::MODIF_ALT);
        viewer.mouse_drag_event(195,150,25,20,ViewerInput::BUTTON_LEFT,ViewerInput::MODIF_ALT);
        viewer.mouse_release_event(195,150,ViewerInput::BUTTON_LEFT,ViewerInput::MODIF_ALT);
        require(!same_matrix(pose,frame->matrix()),"Alt-left drag did not rotate plane");
        const auto center=frame->position(),normal=plane->normal();
        viewer.mouse_press_event(170,130,ViewerInput::BUTTON_RIGHT,ViewerInput::MODIF_ALT);
        viewer.mouse_drag_event(190,155,20,25,ViewerInput::BUTTON_RIGHT,ViewerInput::MODIF_ALT);
        viewer.mouse_release_event(190,155,ViewerInput::BUTTON_RIGHT,ViewerInput::MODIF_ALT);
        const auto delta=frame->position()-center;
        require(delta.norm()>1e-5f && cross(delta,normal).norm()<1e-5f,"Alt-right drag violated normal constraint");
        require(same_matrix(camera,viewer.camera()->modelViewProjectionMatrix()),"plane drag moved camera");
        viewer.set_clipping_planes({vec4(1,0,0,0)});
        require(viewer.clipping_planes()[0]!=vec4(1,0,0,0),"explicit plane overrode enabled shared plane");
        plane->set_enabled(false);
        require(viewer.clipping_planes()==std::vector<vec4>{vec4(1,0,0,0)},"disabled shared plane lost explicit planes");
        viewer.set_clipping_planes({});
        bool bound=false;
        viewer.bind([&](VulkanViewer*,Model*){bound=true;return true;},nullptr,ViewerInput::KEY_C,ViewerInput::MODIF_ALT);
        press(GLFW_KEY_C);require(bound && !plane->is_enabled(),"custom binding did not override clipping control");
        plane->set_enabled(true);
        const auto with_app=viewer.capture();viewer.set_overlays({});const auto without_app=viewer.capture();
        same_picking(with_app,without_app);
        require(with_app.rgba!=without_app.rgba,"shared guide erased application overlay");
        viewer.clear_scene();require(plane->is_enabled() && viewer.clipping_planes().size()==1,"scene clearing lost shared plane");
        const auto empty_guide=viewer.capture();plane->set_visible(false);const auto empty_hidden=viewer.capture();
        require(hits(empty_guide)==0 && empty_guide.rgba!=empty_hidden.rgba,"guide did not survive scene clearing");
        same_picking(empty_guide,empty_hidden);
        require(viewer.device().validation_errors()==0,"Vulkan validation errors");
        std::cout<<"PASS: shared clipping controls, plane/slab equations, constrained drag, transparency, MSAA, depth/IDs and snapshots\n";
    } catch(const std::exception& e) {
        plane->set_enabled(false);std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;
    }
    plane->set_enabled(false);return 0;
}
