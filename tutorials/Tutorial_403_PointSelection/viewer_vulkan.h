// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef EASY3D_TUTORIAL_POINT_SELECTION_VULKAN_H
#define EASY3D_TUTORIAL_POINT_SELECTION_VULKAN_H
#include <easy3d/viewer/vulkan_viewer.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/algo/tessellator.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <algorithm>
#include <limits>

class TutorialPointSelection : public easy3d::VulkanViewer {
public:
    explicit TutorialPointSelection(const std::string& title="PointSelection",
            const easy3d::VulkanViewerOptions& options={}) : VulkanViewer(title,options) {}
protected:
    bool mouse_press_event(int x,int y,int button,int modifiers) override {
        if ((modifiers & GLFW_MOD_SHIFT) && (button==GLFW_MOUSE_BUTTON_LEFT || button==GLFW_MOUSE_BUTTON_RIGHT)) {
            cancel();
            for (const auto& model:models()) if (model.get()==current_model() && dynamic_cast<easy3d::PointCloud*>(model.get())) {
                target_=model; active_=true; button_=button;
                polygon_.push_back(easy3d::vec2(float(x),float(y))); break;
            }
            return true;
        }
        return VulkanViewer::mouse_press_event(x,y,button,modifiers);
    }
    bool mouse_drag_event(int x,int y,int dx,int dy,int button,int modifiers) override {
        if (active_) { extend(x,y); return true; }
        return VulkanViewer::mouse_drag_event(x,y,dx,dy,button,modifiers);
    }
    bool mouse_release_event(int x,int y,int button,int modifiers) override {
        if (!active_) return VulkanViewer::mouse_release_event(x,y,button,modifiers);
        if (button!=button_) return true;
        extend(x,y);
        const auto model=target_.lock();
        if (model && std::find(models().begin(),models().end(),model)!=models().end()) {
            auto* cloud=dynamic_cast<easy3d::PointCloud*>(model.get());
            auto drawable=cloud->renderer()->get_points_drawable("vertices");
            if (drawable && cloud->renderer()->is_visible() && drawable->is_visible()) {
                easy3d::graphics::PointSelectionRequest request;
                request.width=width(); request.height=height(); request.polygon=polygon_;
                request.points=point_resource(drawable);
                request.words.resize((size_t(request.points->source_count())+31)/32,0);
                auto selected=cloud->vertex_property<bool>("v:select");
                for (auto v:cloud->vertices())
                    if (selected[v]) request.words[v.idx()>>5]|=1u<<(v.idx()&31);
                const auto transform=drawable->manipulated_matrix();
                request.model_view_projection=camera()->modelViewProjectionMatrix()*transform;
                for (const auto& plane:clipping_planes()) request.local_clipping_planes.push_back(transpose(transform)*plane);
                request.deselect=button_==GLFW_MOUSE_BUTTON_RIGHT;
                const auto words=select_points(request);
                auto colors=cloud->vertex_property<easy3d::vec3>("v:color");
                for (auto v:cloud->vertices()) {
                    selected[v]=(words[v.idx()>>5] & (1u<<(v.idx()&31)))!=0;
                    colors[v]=selected[v]?easy3d::vec3(1,0,0):drawable->color().xyz();
                }
                drawable->set_coloring(easy3d::State::COLOR_PROPERTY,easy3d::State::VERTEX,"v:color");
                drawable->update();
            }
        }
        cancel(); return true;
    }
    bool key_press_event(int key,int modifiers) override {
        if (key==GLFW_KEY_ESCAPE && active_) { cancel(); return true; }
        if (modifiers==0 && (key==GLFW_KEY_R || key==GLFW_KEY_L)) {
            cancel(); rectangle_=key==GLFW_KEY_R; return true;
        }
        return VulkanViewer::key_press_event(key,modifiers);
    }
    bool focus_event(bool focused) override { if (!focused) cancel(); return false; }
    void post_resize(int,int) override { cancel(); }
    void pre_draw() override {
        if (!active_ || polygon_.size()<3) { set_overlays({}); return; }
        if (dirty_) {
            easy3d::Tessellator tess;
            tess.begin_polygon(easy3d::vec3(0,0,1)); tess.begin_contour();
            for (const auto& p:polygon_) tess.add_vertex(easy3d::vec3(p,0));
            tess.end_contour(); tess.end_polygon();
            std::vector<easy3d::graphics::Vertex> fill,edge;
            for (const auto& triangle:tess.elements()) for (auto index:triangle) {
                const auto* p=tess.vertices()[index]->data();
                easy3d::graphics::Vertex v; v.position=easy3d::vec3(float(p[0]),float(p[1]),0); fill.push_back(v);
            }
            for (size_t i=0,j=polygon_.size()-1;i<polygon_.size();j=i++) {
                const auto a=polygon_[j],b=polygon_[i],d=b-a;
                if (d.length2()<1e-6f) continue;
                const auto n=easy3d::vec2(-d.y,d.x)*(.75f/d.length());
                const easy3d::vec2 corners[]={a-n,b-n,b+n,a+n};
                for (unsigned k:{0u,1u,2u,0u,2u,3u}) {
                    easy3d::graphics::Vertex v; v.position=easy3d::vec3(corners[k],0); edge.push_back(v);
                }
            }
            fill_=fill.empty()?nullptr:std::make_shared<easy3d::graphics::Geometry>(std::move(fill));
            edge_=edge.empty()?nullptr:std::make_shared<easy3d::graphics::Geometry>(std::move(edge)); dirty_=false;
        }
        std::vector<easy3d::graphics::DrawCommand> overlays;
        for (unsigned i=0;i<2;++i) {
            easy3d::graphics::DrawCommand draw; draw.geometry=i?edge_:fill_;
            if (!draw.geometry) continue;
            draw.model_view_projection=easy3d::graphics::screen_space_projection(width(),height());
            draw.color=easy3d::vec4(1,0,0,i?1:.2f); overlays.push_back(draw);
        }
        set_overlays(std::move(overlays));
    }
private:
    void extend(int x,int y) {
        const easy3d::vec2 p{float(x),float(y)};
        if (rectangle_) {
            const auto first=polygon_.front(); polygon_={first,{first.x,p.y},p,{p.x,first.y}};
        } else if ((p-polygon_.back()).length2()>=1) polygon_.push_back(p);
        dirty_=true; update();
    }
    void cancel() { active_=false; polygon_.clear(); target_.reset(); fill_.reset(); edge_.reset(); dirty_=true; update(); }
    bool active_=false,rectangle_=false,dirty_=true;
    int button_=-1;
    std::vector<easy3d::vec2> polygon_;
    std::weak_ptr<easy3d::Model> target_;
    std::shared_ptr<const easy3d::graphics::Geometry> fill_,edge_;
};
#endif
