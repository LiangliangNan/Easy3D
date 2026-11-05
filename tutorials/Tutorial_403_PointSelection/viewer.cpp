/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
 * Easy3D is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3
 * as published by the Free Software Foundation.
 *
 * Easy3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ********************************************************************/

#include "viewer.h"


#include <easy3d/core/point_cloud.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/algo/tessellator.h>
#include <algorithm>
#include <limits>

using namespace easy3d;

TutorialPointSelection::TutorialPointSelection(const std::string& title) : Viewer(title)
{
    manual_ = "-------------- Point selection usage --------------\n"
              "Shift + left/right drag: select/deselect points\n"
              "L: lasso mode; R: rectangle mode; Escape: cancel\n"
              "---------------------------------------------------\n";
    hint_ = "Pressing Shift, drag the mouse to select (left) or deselect (right) points\n"
    "\tL: lasso mode (default)\n"
    "\tR: rectangle mode";
}

bool TutorialPointSelection::mouse_press_event(int x,int y,int button,int modifiers) {
    if ((modifiers & MODIF_SHIFT) && (button==BUTTON_LEFT || button==BUTTON_RIGHT)) {
        cancel();
        for (const auto& model:models()) if (model.get()==current_model() && dynamic_cast<easy3d::PointCloud*>(model.get())) {
            target_=model; active_=true; button_=button;
            polygon_.push_back(easy3d::vec2(float(x)*framebuffer_width()/camera()->screenWidth(),
                                              float(y)*framebuffer_height()/camera()->screenHeight())); break;
        }
        return true;
    }
    return Viewer::mouse_press_event(x,y,button,modifiers);
}

bool TutorialPointSelection::mouse_drag_event(int x,int y,int dx,int dy,int button,int modifiers) {
    if (active_) { extend(x,y); return true; }
    return Viewer::mouse_drag_event(x,y,dx,dy,button,modifiers);
}

bool TutorialPointSelection::mouse_release_event(int x,int y,int button,int modifiers) {
    if (!active_) return Viewer::mouse_release_event(x,y,button,modifiers);
    if (button!=button_) return true;
    extend(x,y);
    const auto model=target_.lock();
    if (model && std::find(models().begin(),models().end(),model)!=models().end()) {
        auto* cloud=dynamic_cast<easy3d::PointCloud*>(model.get());
        auto drawable=cloud->renderer()->get_points_drawable("vertices");
        if (drawable && cloud->renderer()->is_visible() && drawable->is_visible()) {
            easy3d::graphics::PointSelectionRequest request;
            request.width=framebuffer_width(); request.height=framebuffer_height(); request.polygon=polygon_;
            request.points=point_resource(drawable);
            request.words.resize((size_t(request.points->source_count())+31)/32,0);
            auto selected=cloud->vertex_property<bool>("v:select");
            for (auto v:cloud->vertices())
                if (selected[v]) request.words[v.idx()>>5]|=1u<<(v.idx()&31);
            const auto transform=drawable->manipulated_matrix();
            request.model_view_projection=camera()->modelViewProjectionMatrix()*transform;
            for (const auto& plane:clipping_planes()) request.local_clipping_planes.push_back(transpose(transform)*plane);
            request.deselect=button_==BUTTON_RIGHT;
            const auto words=select_points(request);
            auto colors=cloud->vertex_property<easy3d::vec3>("v:color");
            for (auto v:cloud->vertices()) {
                selected[v]=(words[v.idx()>>5] & (1u<<(v.idx()&31)))!=0;
                colors[v]=selected[v]?easy3d::vec3(1,0,0):drawable->color().xyz();
            }
            drawable->set_coloring(easy3d::State::COLOR_PROPERTY,easy3d::State::VERTEX,"v:color");
            drawable->update_color_buffer(colors.vector());
        }
    }
    cancel(); return true;
}

bool TutorialPointSelection::key_press_event(int key,int modifiers) {
    if (key==KEY_ESCAPE && active_) { cancel(); return true; }
    if (modifiers==0 && (key==KEY_R || key==KEY_L)) {
        cancel(); rectangle_=key==KEY_R; return true;
    }
    return Viewer::key_press_event(key,modifiers);
}

bool TutorialPointSelection::focus_event(bool focused) {
    if (!focused) cancel(); return false;
}

void TutorialPointSelection::post_resize(int,int) {
    cancel();
}

void TutorialPointSelection::pre_draw() {
    Viewer::pre_draw();
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
        draw.model_view_projection=easy3d::graphics::screen_space_projection(framebuffer_width(),framebuffer_height());
        draw.color=easy3d::vec4(1,0,0,i?1:.2f); overlays.push_back(draw);
    }
    set_overlays(std::move(overlays));
}

void TutorialPointSelection::extend(int x,int y) {
    const easy3d::vec2 p{float(x)*framebuffer_width()/camera()->screenWidth(),
                         float(y)*framebuffer_height()/camera()->screenHeight()};
    if (rectangle_) {
        const auto first=polygon_.front(); polygon_={first,{first.x,p.y},p,{p.x,first.y}};
    } else if ((p-polygon_.back()).length2()>=1) polygon_.push_back(p);
    dirty_=true; update();
}

void TutorialPointSelection::cancel() {
    active_=false; polygon_.clear(); target_.reset(); fill_.reset(); edge_.reset(); dirty_=true; update();
}
