// SPDX-License-Identifier: GPL-3.0-or-later
#include <easy3d/viewer/vulkan_viewer.h>
#include <easy3d/fileio/image_io.h>
#include <easy3d/util/initializer.h>
#include <easy3d/util/resource.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <thread>
using namespace easy3d;
void require(bool ok,const char* text) { if (!ok) throw std::runtime_error(text); }
struct HintViewer : VulkanViewer {
    using VulkanViewer::VulkanViewer;
    using VulkanViewer::mouse_press_event;
    using VulkanViewer::mouse_drag_event;
    using VulkanViewer::mouse_release_event;
};
void same_picking(const graphics::FrameImage& a,const graphics::FrameImage& b) {
    require(a.width==b.width && a.height==b.height,"hint changed framebuffer extent");
    for (size_t i=0;i<a.ids.size();++i)
        require(a.depth[i]==b.depth[i] && a.ids[i].object==b.ids[i].object &&
                a.ids[i].primitive==b.ids[i].primitive,"hint changed depth or picking IDs");
}
int main(int argc,char**) {
    try {
        initialize(false,false,false);
        VulkanViewerOptions options; options.width=233;options.height=197;options.visible=false;
        options.device.validation=options.device.synchronization_validation=argc>1;
        HintViewer v("Interaction overlays",options);
        require(v.add_model(resource::directory()+"/data/bunny.ply"),"scene model missing");
        const int w=v.width(),h=v.height();
        std::vector<graphics::Vertex> vertices(3);
        vertices[0].position=vec3(0,0,0);vertices[1].position=vec3(float(w),0,0);vertices[2].position=vec3(0,float(h),0);
        graphics::DrawCommand application;
        application.geometry=std::make_shared<graphics::Geometry>(vertices);
        application.color=vec4(1,0,0,.25f);
        application.model_view_projection=graphics::screen_space_projection(w,h);
        v.set_overlays({application});
        auto focus=glfwSetWindowFocusCallback(v.window(),nullptr);glfwSetWindowFocusCallback(v.window(),focus);
        auto key=glfwSetKeyCallback(v.window(),nullptr);glfwSetKeyCallback(v.window(),key);
        for (unsigned samples:{1u,4u}) {
            if (!(v.device().supported_sample_counts()&samples)) continue;
            v.set_samples(samples);
            const auto baseline=v.capture();
            const auto camera=v.camera()->modelViewProjectionMatrix();
            const int x0=w/4,y0=h/4,x1=3*w/4,y1=3*h/4;
            v.mouse_press_event(x0,y0,VulkanViewer::BUTTON_LEFT,VulkanViewer::MODIF_CTRL);
            v.mouse_drag_event(x1,y1,x1-x0,y1-y0,VulkanViewer::BUTTON_LEFT,VulkanViewer::MODIF_CTRL);
            const auto region=v.capture(); same_picking(baseline,region);
            require(region.rgba!=baseline.rgba,"region overlay missing");
            const size_t inside=4*(size_t(h/2)*w+w/2);
            for (unsigned c=0;c<3;++c) {
                const float expected=.8f*baseline.rgba[inside+c]+(c==2?51:0);
                require(std::abs(region.rgba[inside+c]-expected)<=2,"region fill did not blend blue at alpha 0.2");
            }
            const size_t edge=4*(size_t(h/2)*w+x0);
            require(region.rgba[edge]<5 && region.rgba[edge+1]<5 && region.rgba[edge+2]>250,
                    "region boundary is not solid blue");
            const auto after=v.camera()->modelViewProjectionMatrix();
            for (unsigned i=0;i<16;++i) require(camera[i]==after[i],"region preview moved camera");
            // Application overlay replacement must not erase the host's UI layer.
            v.set_overlays({}); require(v.capture().rgba!=region.rgba,"application overlay did not clear");
            v.set_overlays({application}); require(v.capture().rgba==region.rgba,"application update erased interaction hint");
            require(ImageIO::save("vulkan-interaction-hints.png",region.rgba,w,h,4,false),"hint artifact save failed");
#ifdef __APPLE__
            const int control=GLFW_KEY_LEFT_SUPER;
#else
            const int control=GLFW_KEY_LEFT_CONTROL;
#endif
            key(v.window(),control,0,GLFW_RELEASE,0);
            require(v.capture().rgba==baseline.rgba,"modifier release left stale region preview");
            // Losing focus ends the gesture, including a later stale release.
            focus(v.window(),GLFW_FALSE);
            v.mouse_release_event(x1,y1,VulkanViewer::BUTTON_LEFT,VulkanViewer::MODIF_CTRL);
            require(v.capture().rgba==baseline.rgba,"stale release zoomed after focus cancellation");
            size_t hit=0;while (hit<baseline.ids.size() && baseline.ids[hit].object==UINT32_MAX) ++hit;
            require(hit<baseline.ids.size(),"scene has no pivot hit");
            const int px=int(hit%w),py=int(hit/w);
            const auto picked=v.pick(px,py);
            v.mouse_press_event(px,py,VulkanViewer::BUTTON_LEFT,VulkanViewer::MODIF_SHIFT);
            v.mouse_release_event(px,py,VulkanViewer::BUTTON_LEFT,VulkanViewer::MODIF_SHIFT);
            const auto pivot=v.capture();same_picking(baseline,pivot);
            require(pivot.rgba!=baseline.rgba,"pivot cross missing");
            require(pivot.rgba[4*hit]<5 && pivot.rgba[4*hit+1]<5 && pivot.rgba[4*hit+2]<5,
                    "pivot cross did not match the original black marker");
            const auto hint_pick=v.pick(px,py);
            require(hint_pick.drawable==picked.drawable && hint_pick.position==picked.position,"cross obscured pivot picking");
            v.mouse_press_event(px,py,VulkanViewer::BUTTON_RIGHT,VulkanViewer::MODIF_SHIFT);
            v.mouse_release_event(px,py,VulkanViewer::BUTTON_RIGHT,VulkanViewer::MODIF_SHIFT);
            require(v.capture().rgba==baseline.rgba,"pivot reset left stale cross or erased application overlay");
            // Frame presentation and explicit capture must share the same layers.
            v.mouse_press_event(x0,y0,VulkanViewer::BUTTON_LEFT,VulkanViewer::MODIF_CTRL);
            v.mouse_drag_event(x1,y1,x1-x0,y1-y0,VulkanViewer::BUTTON_LEFT,VulkanViewer::MODIF_CTRL);
            VulkanViewer::poll_events();require(v.draw_frame(),"hint presentation failed");
            same_picking(region,v.capture());
            focus(v.window(),GLFW_FALSE);
        }
        // Expiration also applies to explicit captures when no frames were drawn.
        const auto baseline=v.capture();
        size_t hit=0;while (hit<baseline.ids.size() && baseline.ids[hit].object==UINT32_MAX) ++hit;
        const int px=int(hit%w),py=int(hit/w);
        v.mouse_press_event(px,py,VulkanViewer::BUTTON_LEFT,VulkanViewer::MODIF_SHIFT);
        v.mouse_release_event(px,py,VulkanViewer::BUTTON_LEFT,VulkanViewer::MODIF_SHIFT);
        require(v.capture().rgba!=baseline.rgba,"expiry test cross missing");
        std::this_thread::sleep_for(std::chrono::milliseconds(10020));
        require(v.capture().rgba==baseline.rgba,"pivot cross did not expire without frame callbacks");
        v.mouse_press_event(w/4,h/4,VulkanViewer::BUTTON_LEFT,VulkanViewer::MODIF_CTRL);
        v.mouse_drag_event(w/2,h/2,w/4,h/4,VulkanViewer::BUTTON_LEFT,VulkanViewer::MODIF_CTRL);
        v.resize(257,211);VulkanViewer::poll_events();v.draw_frame();
        const auto resized=v.capture(); focus(v.window(),GLFW_FALSE);
        require(v.capture().rgba==resized.rgba,"resize did not cancel stale pixel-coordinate hint");
        v.clear_scene();
        const auto empty=v.capture();
        for (auto id:empty.ids) require(id.object==UINT32_MAX,"clear retained scene pick IDs");
        for (auto c:empty.rgba) require(c==255,"clear retained viewer or application overlays");
        require(v.device().validation_errors()==0,"interaction overlay validation errors");
        std::cout << "PASS: pivot/region hints, alpha, MSAA, depth/IDs, layer isolation, expiry, resize and focus\n";
    } catch (const std::exception& e) {std::cerr<<"FAIL: "<<e.what()<<'\n';return 1;}
}
