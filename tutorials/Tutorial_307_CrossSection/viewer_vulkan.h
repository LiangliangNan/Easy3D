// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_TUTORIAL_CROSS_SECTION_VULKAN_H
#define EASY3D_TUTORIAL_CROSS_SECTION_VULKAN_H
#include <easy3d/viewer/vulkan_viewer.h>
#include <easy3d/renderer/manipulated_frame.h>
#include <easy3d/renderer/constraint.h>
#include <easy3d/util/setting.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>

namespace easy3d {
class CrossSection : public VulkanViewer {
public:
    explicit CrossSection(const std::string& title,const VulkanViewerOptions& options = {}) : VulkanViewer(title,options) {
        constraint_.setTranslationConstraint(AxisPlaneConstraint::AXIS,vec3(0,0,1));
        plane_.setConstraint(&constraint_);
    }
protected:
    bool key_press_event(int key,int modifiers) override {
        if (modifiers==GLFW_MOD_ALT) {
            if (key==GLFW_KEY_C) enabled_=!enabled_;
            else if (key==GLFW_KEY_V && enabled_) visible_=!visible_;
            else if (key==GLFW_KEY_X && enabled_) { section_=!section_; dirty_=true; }
            else return VulkanViewer::key_press_event(key,modifiers);
            update(); return true;
        }
        return VulkanViewer::key_press_event(key,modifiers);
    }
    bool mouse_press_event(int x,int y,int button,int modifiers) override {
        if (modifiers==0 && (button==GLFW_MOUSE_BUTTON_LEFT || button==GLFW_MOUSE_BUTTON_RIGHT)) {
            plane_.action_start(); return true;
        }
        return VulkanViewer::mouse_press_event(x,y,button,modifiers);
    }
    bool mouse_release_event(int x,int y,int button,int modifiers) override {
        plane_.action_end(); return VulkanViewer::mouse_release_event(x,y,button,modifiers);
    }
    bool mouse_drag_event(int x,int y,int dx,int dy,int button,int modifiers) override {
        if (modifiers==0 && button==GLFW_MOUSE_BUTTON_LEFT)
            plane_.action_rotate(x,y,dx,dy,camera(),ManipulatedFrame::NONE);
        else if (modifiers==0 && button==GLFW_MOUSE_BUTTON_RIGHT)
            plane_.action_translate(x,y,dx,dy,camera(),ManipulatedFrame::NONE);
        else return VulkanViewer::mouse_drag_event(x,y,dx,dy,button,modifiers);
        dirty_=true; update(); return true;
    }
    void pre_draw() override {
        if (!models().empty() && (last_model_.lock()!=models().back() || last_count_!=models().size())) {
            plane_.setPosition(camera()->sceneCenter()); radius_=camera()->sceneRadius();
            last_model_=models().back(); last_count_=models().size(); dirty_=true;
        }
        const mat4 pose=plane_.matrix();
        const auto n=mat3(pose)*vec3(0,0,1); const auto c=plane_.position();
        const float width=radius_*.01f;
        // Match the legacy tutorial: keep the negative-normal halfspace, or a
        // slab ending at that plane. The guide tests scene depth while leaving
        // scene depth and picking untouched.
        if (enabled_) {
            std::vector<vec4> planes{vec4(-n,dot(n,c))};
            if (section_) planes.push_back(vec4(n,-dot(n,c)+width));
            set_clipping_planes(planes);
        } else set_clipping_planes({});
        if (!enabled_ || !visible_) { set_overlays({}); return; }
        if (dirty_) {
            std::vector<graphics::Vertex> vertices;
            auto band=[&](float x0,float y0,float x1,float y1,float z) {
                const vec3 corners[]={vec3(x0,y0,z),vec3(x1,y0,z),vec3(x1,y1,z),vec3(x0,y1,z)};
                for (unsigned i:{0u,1u,2u,0u,2u,3u}) {
                    graphics::Vertex v; v.position=pose*corners[i]; vertices.push_back(v);
                }
            };
            const float r=radius_,t=r*.006f;
            auto outline=[&](float z) {
                band(-r,-r,r,-r+t,z); band(-r,r-t,r,r,z);
                band(-r,-r,-r+t,r,z); band(r-t,-r,r,r,z);
                band(-r*.06f,-t,r*.06f,t,z); band(-t,-r*.06f,t,r*.06f,z);
            };
            outline(0); if (section_) outline(-width);
            guide_=std::make_shared<graphics::Geometry>(std::move(vertices));
            std::vector<graphics::Vertex> face;
            const vec3 corners[]={vec3(-r,-r,0),vec3(r,-r,0),vec3(r,r,0),vec3(-r,r,0)};
            for (unsigned i:{0u,1u,2u,0u,2u,3u}) {
                graphics::Vertex v; v.position=pose*corners[i]; face.push_back(v);
            }
            fill_=std::make_shared<graphics::Geometry>(std::move(face)); dirty_=false;
        }
        graphics::DrawCommand fill; fill.geometry=fill_; fill.model_view_projection=camera()->modelViewProjectionMatrix();
        fill.color=setting::clipping_plane_color; fill.overlay_depth_test=true;
        auto guide=fill; guide.geometry=guide_; guide.color.w=1;
        set_overlays({fill,guide});
    }
private:
    LocalConstraint constraint_;
    ManipulatedFrame plane_;
    bool enabled_=true,visible_=true,section_=false,dirty_=true;
    float radius_=1;
    std::weak_ptr<Model> last_model_;
    size_t last_count_=0;
    std::shared_ptr<const graphics::Geometry> guide_,fill_;
};
}
#endif
