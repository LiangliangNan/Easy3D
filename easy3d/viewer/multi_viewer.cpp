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

#include <easy3d/viewer/multi_viewer.h>
#include <easy3d/core/model.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/transform.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_lines_2D.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/dialog.h>
#include <easy3d/util/logging.h>
#include <easy3d/fileio/image_io.h>
#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>
#ifdef EASY3D_HAS_VULKAN
#include <easy3d/renderer/shape.h>
#else
#include <easy3d/renderer/opengl.h>
#include <easy3d/renderer/framebuffer_object.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#endif

namespace easy3d {
namespace {
#ifndef EASY3D_HAS_VULKAN
struct CellScissor {
    GLboolean enabled;GLint box[4];
    CellScissor() : enabled(glIsEnabled(GL_SCISSOR_TEST)) {glGetIntegerv(GL_SCISSOR_BOX,box);glEnable(GL_SCISSOR_TEST);}
    ~CellScissor() {glScissor(box[0],box[1],box[2],box[3]);if(!enabled) glDisable(GL_SCISSOR_TEST);}
};
#endif
// Top-left pixel bounds. Integer boundaries cover odd/HiDPI extents without gaps.
ivec4 cell_bounds(int row,int col,int rows,int cols,int width,int height) {
    const int x=int(int64_t(col)*width/cols), y=int(int64_t(row)*height/rows);
    return ivec4(x,y,int(int64_t(col+1)*width/cols)-x,int(int64_t(row+1)*height/rows)-y);
}
// Grid lines are one logical pixel wide. Both a LinesDrawable2D width and a
// screen-space draw command width are target pixels, so convert through the
// logical window width: this is the DPI scale for a window-sized target and
// output/logical for a higher-resolution snapshot or capture.
float division_line_width(int target_width,int logical_width) {
    return logical_width>0 ? float(target_width)/float(logical_width) : 1.0f;
}
void configure_cell(Camera& cell,const Camera& source,int width,int height) {
    cell.set_screen_width_and_height(width,height);
    cell.set_projection_matrix(source.projection_matrix());
    cell.set_model_view_matrix(source.model_view_matrix());
}

}
MultiViewer::MultiViewer(int rows,int cols,const std::string& title,
                         int width,int height,int samples,
                         bool full_screen,bool visible,bool resizable,
#ifdef EASY3D_HAS_VULKAN
                         const VulkanOptions& options)
#else
                         const OpenGLOptions& options)
#endif
    : Viewer(title,width,height,samples,full_screen,visible,resizable,options) {
    snapshot_hud_=false;
    set_layout(rows,cols);
}
MultiViewer::~MultiViewer() = default;
void MultiViewer::set_layout(int rows,int cols) {
    if (rows<=0 || cols<=0)
        throw std::invalid_argument("multi-view layout must contain a positive, representable number of cells");
    num_rows_=rows;num_cols_=cols;active_row_=std::min(active_row_,rows-1);active_col_=std::min(active_col_,cols-1);views_.resize(rows);
    for(auto& row:views_) row.resize(cols);
#ifdef EASY3D_HAS_VULKAN
    division_commands_.clear();
#endif
    cancel_interaction();
    update_division();
}
void MultiViewer::assign(int row,int col,const Model* model) {
    if(row<0 || row>=num_rows_ || col<0 || col>=num_cols_ || !model) {
        LOG(ERROR)<<"invalid multi-view model assignment";return;
    }
    views_[row][col].models.push_back(model);
}
void MultiViewer::assign(int row,int col,const Drawable* drawable) {
    if(row<0 || row>=num_rows_ || col<0 || col>=num_cols_ || !drawable) {
        LOG(ERROR)<<"invalid multi-view drawable assignment";return;
    }
    const_cast<Drawable*>(drawable)->set_visible(true);
    views_[row][col].drawables.push_back(drawable);
}
void MultiViewer::set_division_visible(bool visible) { if(drawable_division_) drawable_division_->set_visible(visible); }
bool MultiViewer::division_visible() const { return drawable_division_ && drawable_division_->is_visible(); }
void MultiViewer::init() { Viewer::init(); update_division(); }
void MultiViewer::post_resize(int,int) { update_division(); }
void MultiViewer::update_division() {
    if(views_.empty()) return;
    if(!drawable_division_) {
        drawable_division_.reset(new LinesDrawable2D("division"));
    }
    const auto active=cell_bounds(active_row_,active_col_,num_rows_,num_cols_,width(),height());
    camera()->set_screen_width_and_height(active.z,active.w);
    const int fw=framebuffer_width(),fh=framebuffer_height();
#ifdef EASY3D_HAS_VULKAN
    division_commands_.clear();
#endif
    for(int r=0;r<num_rows_;++r) for(int c=0;c<num_cols_;++c) {
        auto bounds=cell_bounds(r,c,num_rows_,num_cols_,fw,fh);
#ifndef EASY3D_HAS_VULKAN
        bounds.y=fh-bounds.y-bounds.w;
#endif
        views_[r][c].viewport=bounds;
    }
    std::vector<vec2> points;
    for(int r=1;r<num_rows_;++r) {const float y=float(int64_t(r)*height()/num_rows_);points.push_back(vec2(0,y));points.push_back(vec2(width(),y));}
    for(int c=1;c<num_cols_;++c) {const float x=float(int64_t(c)*width()/num_cols_);points.push_back(vec2(x,0));points.push_back(vec2(x,height()));}
    drawable_division_->update_vertex_buffer(points,width(),height(),true);
}
bool MultiViewer::delete_model(Model* model) {
    auto owned=std::find_if(models_.begin(),models_.end(),[&](const std::shared_ptr<Model>& m) {return m.get()==model;});
    if(owned==models_.end()) return false;
    std::vector<const Drawable*> removed;
    if(model->renderer()) {
        for(const auto& d:model->renderer()->points_drawables()) removed.push_back(d.get());
        for(const auto& d:model->renderer()->lines_drawables()) removed.push_back(d.get());
        for(const auto& d:model->renderer()->triangles_drawables()) removed.push_back(d.get());
    }
    if(!Viewer::delete_model(model)) return false;
    for(auto& row:views_) for(auto& view:row) {
        view.models.erase(std::remove(view.models.begin(),view.models.end(),model),view.models.end());
        view.drawables.erase(std::remove_if(view.drawables.begin(),view.drawables.end(),[&](const Drawable* d) {
            return std::find(removed.begin(),removed.end(),d)!=removed.end();
        }),view.drawables.end());
    }
    return true;
}
bool MultiViewer::delete_drawable(Drawable* drawable) {
    if(!Viewer::delete_drawable(drawable)) return false;
    for(auto& row:views_) for(auto& view:row)
        view.drawables.erase(std::remove(view.drawables.begin(),view.drawables.end(),drawable),view.drawables.end());
    return true;
}
void MultiViewer::clear_scene() {
    for(auto& row:views_) for(auto& view:row) {view.models.clear();view.drawables.clear();}
    Viewer::clear_scene();
}
vec3 MultiViewer::world_to_window(const vec3& world) const {
    const auto vp=cell_bounds(active_row_,active_col_,num_rows_,num_cols_,width(),height());
    Camera cell(*camera());cell.frame()->setPivotPoint(camera()->pivot_point());
    configure_cell(cell,*camera(),vp.z,vp.w);
    const int viewport[] = {vp.x,vp.y,vp.z,vp.w};
    return transform::project(world,cell.model_view_projection_matrix(),viewport,false);
}
vec3 MultiViewer::window_to_world(const vec3& window) const {
    if(!std::isfinite(window.x) || !std::isfinite(window.y) || !std::isfinite(window.z) || width()<=0 || height()<=0)
        throw std::invalid_argument("invalid multi-view unprojection");
    int row=num_rows_-1,col=num_cols_-1;
    for(int r=0;r<num_rows_;++r) if(window.y < int64_t(r+1)*height()/num_rows_) {row=r;break;}
    for(int c=0;c<num_cols_;++c) if(window.x < int64_t(c+1)*width()/num_cols_) {col=c;break;}
    const auto vp=cell_bounds(row,col,num_rows_,num_cols_,width(),height());
    Camera cell(*camera());cell.frame()->setPivotPoint(camera()->pivot_point());
    configure_cell(cell,*camera(),vp.z,vp.w);
    const int viewport[] = {vp.x,vp.y,vp.z,vp.w};
    return transform::unproject(window,cell.model_view_projection_matrix(),viewport,false);
}
bool MultiViewer::snapshot() const {
    std::string name=current_model()?file_system::replace_extension(current_model()->name(),"png"):"untitled.png";
    const auto file=dialog::save("Please choose a file name",name,{"Image Files (*.png *.jpg *.bmp *.tga)","*.png *.jpg *.bmp *.tga","All Files (*.*)","*"},true);
    return !file.empty() && snapshot(file,1,samples(), 0);
}
#ifdef EASY3D_HAS_VULKAN
std::vector<graphics::DrawCommand> MultiViewer::draw_commands(const Camera& cam,int width,int height,
                                                            std::vector<DrawSource>* sources, bool) const {
    std::vector<graphics::DrawCommand> result;std::vector<DrawSource> ignored;
    auto& mapped=sources?*sources:ignored;
    for(int r=0;r<num_rows_;++r) for(int c=0;c<num_cols_;++c) {
        const auto vp=cell_bounds(r,c,num_rows_,num_cols_,width,height);
        if(vp.z<=0 || vp.w<=0) continue;
        // Logical cell bounds drive the cell camera (DPI-independent impostor sizing);
        // the framebuffer cell width drives plain point/line scaling in draw_commands.
        const auto lv=cell_bounds(r,c,num_rows_,num_cols_,Viewer::width(),Viewer::height());
        Camera cell(cam);cell.frame()->setPivotPoint(cam.pivot_point());
        configure_cell(cell,snapshot_active_?*camera():cam,lv.z,lv.w);
        std::vector<DrawSource> local;
        const auto commands=Viewer::draw_commands(cell,vp.z,vp.w,&local,true);
        const auto& view=views_[r][c];
        for(auto command:commands) {
            if(command.object_id!=UINT32_MAX) {
                const auto& source=local.at(command.object_id);
                const bool assigned_model=source.model && source.model->renderer()->is_visible() &&
                    std::find(view.models.begin(),view.models.end(),source.model.get())!=view.models.end();
                if(!assigned_model && std::find(view.drawables.begin(),view.drawables.end(),source.drawable.get())==view.drawables.end()) continue;
                if(mapped.size()>=UINT32_MAX) throw std::length_error("too many multi-view picking sources");
                if(sources) {command.object_id=uint32_t(mapped.size());mapped.push_back(source);}
                else if(source.model) command.object_id=uint32_t(std::find(models_.begin(),models_.end(),source.model)-models_.begin());
                else command.object_id=uint32_t(models_.size()+size_t(std::find(drawables_.begin(),drawables_.end(),source.drawable)-drawables_.begin()));
            }
            command.viewport=vp;result.push_back(std::move(command));
        }
    }
    if(division_visible()) {
        // Cached for one target size; the ribbon is one logical pixel wide, so its
        // target-pixel width follows the target (DPI for a window, output/logical
        // for a snapshot).
        if(division_commands_.empty() || division_width_!=width || division_height_!=height) {
            std::vector<vec2> points;
            for(int r=1;r<num_rows_;++r) {const float y=float(int64_t(r)*height/num_rows_);points.push_back(vec2(0,y));points.push_back(vec2(width,y));}
            for(int c=1;c<num_cols_;++c) {const float x=float(int64_t(c)*width/num_cols_);points.push_back(vec2(x,0));points.push_back(vec2(x,height));}
            division_commands_.clear();
            if(!points.empty()) division_commands_.push_back(graphics::shape::draw_lines_2d(points,vec4(0,0,0,1),width,height,division_line_width(width,Viewer::width())));
            division_width_=width;division_height_=height;
        }
        result.insert(result.end(),division_commands_.begin(),division_commands_.end());
    }
    return result;
}
#else
    void MultiViewer::draw() const {
        CellScissor clip;
        // remember the viewer port, later we will have to restore it after drawing each view
        ivec4 viewport;
        glGetIntegerv(GL_VIEWPORT, viewport);

        // ------------------------------------------------------------

        for (const auto& row : views_) {
            for (const auto& view : row) {
                const auto &vp = view.viewport;
                if(vp[2]<=0 || vp[3]<=0) continue;
                glViewport(vp[0], vp[1], vp[2], vp[3]);
                glScissor(vp[0],vp[1],vp[2],vp[3]);
                for (const auto& owned: models_) {
                    const auto* m=owned.get();
                    if (std::find(view.models.begin(),view.models.end(),m)==view.models.end() || !m->renderer() || !m->renderer()->is_visible())
                        continue;

                    // Let's check if edges and surfaces are both shown. If true, we
                    // make the depth coordinates of the surface smaller, so that displaying
                    // the mesh and the surface together does not cause Z-fighting.
                    std::size_t count = 0;
                    for (auto d: m->renderer()->lines_drawables()) {
                        if (d->is_visible()) {
                            d->draw(camera());
                            ++count;
                        }
                    }

                    for (auto d: m->renderer()->points_drawables()) {
                        if (d->is_visible())
                            d->draw(camera());
                    }

                    if (count > 0) {
                        glEnable(GL_POLYGON_OFFSET_FILL);
                        glPolygonOffset(0.5f, -0.0001f);
                    }
                    for (auto d: m->renderer()->triangles_drawables()) {
                        if (d->is_visible())
                            d->draw(camera());
                    }
                    if (count > 0)
                        glDisable(GL_POLYGON_OFFSET_FILL);
                }

                for (const auto d: view.drawables) {
                    if (d->is_visible())
                        d->draw(camera());
                }

                // draw the transparent clipping plane
                ClippingPlane::instance()->draw(camera());
            }
        }

        // ------------------------------------------------------------

        // restore viewport.
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        glScissor(viewport[0],viewport[1],viewport[2],viewport[3]);

        // draw the division of views
        if (drawable_division_->is_visible()) {
            // The drawable width is in target pixels; keep the grid one logical pixel wide.
            drawable_division_->set_line_width(division_line_width(viewport[2], width()));
            drawable_division_->draw(camera());
        }
        draw_overlays(false);
    }



void MultiViewer::prepare_pick_view(int x,int y,int width,int height) const {
    // x/y and width/height are framebuffer pixels (the primitive-ID pass target).
    if(x<0 || y<0 || x>=width || y>=height) {glDisable(GL_SCISSOR_TEST);glViewport(0,0,width,height);return;}
    int row=num_rows_-1,col=num_cols_-1;
    for(int r=0;r<num_rows_;++r) if(y < int64_t(r+1)*height/num_rows_) {row=r;break;}
    for(int c=0;c<num_cols_;++c) if(x < int64_t(c+1)*width/num_cols_) {col=c;break;}
    const auto& vp=views_[row][col].viewport;
    if(vp.z<=0 || vp.w<=0) {glDisable(GL_SCISSOR_TEST);glViewport(0,0,width,height);return;}
    glEnable(GL_SCISSOR_TEST);
    glScissor(vp.x,vp.y,vp.z,vp.w);
    glViewport(vp.x,vp.y,vp.z,vp.w);
}
void MultiViewer::draw_scene(const Camera& snapshot,const DrawSource* source,bool guides) const {
    CellScissor clip;
    // Partition the caller's render target (the current GL viewport) into physical cell
    // viewports. Offscreen queries render into a framebuffer-sized target, while the
    // overlay-depth callback renders into a snapshot-output-sized target; querying the
    // viewport keeps cells aligned with either. The cell camera stays logical (camera()
    // dims) so world-space impostor radii carry DPI-independent sizing.
    GLint target[4];glGetIntegerv(GL_VIEWPORT,target);
    const int fw=target[2],fh=target[3];
    Camera cell(snapshot);cell.frame()->setPivotPoint(snapshot.pivot_point());
    auto draw=[&](const Drawable* d) {
        if(!d || (source && source->drawable.get()!=d) || !d->is_visible() || dynamic_cast<const LinesDrawable2D*>(d)) return;
        if(auto p=dynamic_cast<const TrianglesDrawable*>(d)) p->TrianglesDrawable::draw(&cell);
        else if(auto p=dynamic_cast<const PointsDrawable*>(d)) p->PointsDrawable::draw(&cell);
        else if(auto p=dynamic_cast<const LinesDrawable*>(d)) p->LinesDrawable::draw(&cell);
    };
    for(int r=0;r<num_rows_;++r) for(int c=0;c<num_cols_;++c) {
        const auto& view=views_[r][c];
        auto vp=cell_bounds(r,c,num_rows_,num_cols_,fw,fh);
        if(vp.z<=0 || vp.w<=0) continue;
        const auto logical=cell_bounds(r,c,num_rows_,num_cols_,width(),height());
        configure_cell(cell,snapshot,logical.z,logical.w);
        vp.y=fh-vp.y-vp.w;
        glViewport(vp.x,vp.y,vp.z,vp.w);glScissor(vp.x,vp.y,vp.z,vp.w);
        for(const auto& model:models_) if(model->renderer() && (!source || source->model==model)) {
            const bool assigned=model->renderer()->is_visible() && std::find(view.models.begin(),view.models.end(),model.get())!=view.models.end();
            auto append=[&](const Drawable* d) {if(assigned || std::find(view.drawables.begin(),view.drawables.end(),d)!=view.drawables.end()) draw(d);};
            for(const auto& d:model->renderer()->triangles_drawables()) append(d.get());
            for(const auto& d:model->renderer()->lines_drawables()) append(d.get());
            for(const auto& d:model->renderer()->points_drawables()) append(d.get());
        }
        for(const auto& d:drawables_) if((!source || !source->model) && std::find(view.drawables.begin(),view.drawables.end(),d.get())!=view.drawables.end()) draw(d.get());
        if(guides) {
            ClippingPlane::instance()->draw(&cell);
            glDisable(GL_DEPTH_TEST);glDepthMask(GL_FALSE);
            const auto flat=[&](const Drawable* d) {if(d && d->is_visible()) if(auto p=dynamic_cast<const LinesDrawable2D*>(d)) p->draw(&cell);};
            for(const auto& model:models_) if(model->renderer()) {
                const bool assigned=model->renderer()->is_visible() && std::find(view.models.begin(),view.models.end(),model.get())!=view.models.end();
                for(const auto& d:model->renderer()->lines_drawables())
                    if(assigned || std::find(view.drawables.begin(),view.drawables.end(),d.get())!=view.drawables.end()) flat(d.get());
            }
            for(const auto& d:drawables_) if(std::find(view.drawables.begin(),view.drawables.end(),d.get())!=view.drawables.end()) flat(d.get());
            glEnable(GL_DEPTH_TEST);glDepthMask(GL_TRUE);
        }
    }
    if(guides && division_visible()) {
        glViewport(0,0,fw,fh);glScissor(0,0,fw,fh);
        glDisable(GL_DEPTH_TEST);glDepthMask(GL_FALSE);
        // Same target-pixel width rule; fw is the current render target, not
        // necessarily the window framebuffer (captures may be larger).
        drawable_division_->set_line_width(division_line_width(fw,width()));
        drawable_division_->draw(&snapshot);
        glEnable(GL_DEPTH_TEST);glDepthMask(GL_TRUE);
    }
}
#endif
bool MultiViewer::mouse_press_event(int x,int y,int button,int modifiers) {
    for(int r=0;r<num_rows_;++r) for(int c=0;c<num_cols_;++c) {
        const auto vp=cell_bounds(r,c,num_rows_,num_cols_,width(),height());
        if(x>=vp.x && y>=vp.y && x<vp.x+vp.z && y<vp.y+vp.w) {
            active_row_=r;active_col_=c;interaction_origin_=ivec2(vp.x,vp.y);
            camera()->set_screen_width_and_height(vp.z,vp.w);
            return Viewer::mouse_press_event(x,y,button,modifiers);
        }
    }
    return false;
}
bool MultiViewer::snapshot(const std::string& file,float scale,int count,int bg,bool expand) const {
    if(!std::isfinite(scale) || scale<=0 || count<=0 || count>64 || (count&(count-1)) || bg<0 || bg>2 ||
       double(framebuffer_width())*scale>std::numeric_limits<int>::max() || double(framebuffer_height())*scale>std::numeric_limits<int>::max()) return false;
    auto* self=const_cast<MultiViewer*>(this);
    if(!initialized_) { self->init(); self->initialized_=true; }
    const int w=int(std::ceil(framebuffer_width()*double(scale))),h=int(std::ceil(framebuffer_height()*double(scale)));
    const mat4 original=camera()->projection_matrix();const bool active=snapshot_active_;
    const double z_near=camera()->z_near(), z_far=camera()->z_far();float x,y;
    if(camera()->type()==Camera::PERSPECTIVE) {y=float(z_near*std::tan(camera()->field_of_view()*.5));x=float(y*camera()->aspect_ratio());}
    else camera()->get_ortho_width_height(x,y);
    const double output_aspect=(double(w)/num_cols_)/(double(h)/num_rows_);
    if((expand && output_aspect>double(x)/y) || (!expand && output_aspect<double(x)/y)) x=float(output_aspect*y);
    else y=float(x/output_aspect);
    self->camera()->set_projection_matrix(camera()->type()==Camera::PERSPECTIVE
        ? transform::frustum(-x,x,-y,y,float(z_near),float(z_far)) : transform::ortho(-x,x,-y,y,float(z_near),float(z_far)));
    snapshot_active_=true;bool result=false;
#ifdef EASY3D_HAS_VULKAN
    try {result=Viewer::snapshot(file,scale,count,bg,expand);}
    catch(...) {self->camera()->set_projection_matrix(original);snapshot_active_=active;throw;}
#else
    GLFWwindow* previous=glfwGetCurrentContext();glfwMakeContextCurrent(window_);
    GLint viewport[4],draw_fbo,read_fbo;GLfloat clear[4];glGetIntegerv(GL_VIEWPORT,viewport);glGetFloatv(GL_COLOR_CLEAR_VALUE,clear);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING,&draw_fbo);glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING,&read_fbo);
    const auto saved=views_;
    try {
        FramebufferObject target(w,h,count<=1?0:count);
        if(!target.add_color_buffer() || !target.add_depth_buffer() || !target.is_valid() || !target.bind()) throw std::runtime_error("cannot create multi-view snapshot target");
        for(int r=0;r<num_rows_;++r) for(int c=0;c<num_cols_;++c) {auto vp=cell_bounds(r,c,num_rows_,num_cols_,w,h);vp.y=h-vp.y-vp.w;self->views_[r][c].viewport=vp;}
        glViewport(0,0,w,h);const vec4 color=bg==0?background_color():bg==1?vec4(1):vec4(0);
        glClearColor(color.x,color.y,color.z,color.w);glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
        self->draw();self->draw_overlays(true);target.release();
        if(bg==2) {
            std::vector<unsigned char> rgba;
            if(target.read_color(0,rgba,GL_RGBA,true)) {
                for(size_t i=0;i<rgba.size();i+=4) if(rgba[i+3] && rgba[i+3]<255)
                    for(unsigned c=0;c<3;++c) rgba[i+c]=static_cast<unsigned char>(std::min(255u,(unsigned(rgba[i+c])*255+rgba[i+3]/2)/rgba[i+3]));
                result=ImageIO::save(file,rgba,w,h,4);
            }
        } else result=target.snapshot_color(0,file);
    } catch(const std::exception& e) {LOG(ERROR)<<"Multi-view snapshot failed: "<<e.what();}
    self->views_=saved;glBindFramebuffer(GL_DRAW_FRAMEBUFFER,draw_fbo);glBindFramebuffer(GL_READ_FRAMEBUFFER,read_fbo);glViewport(viewport[0],viewport[1],viewport[2],viewport[3]);glClearColor(clear[0],clear[1],clear[2],clear[3]);glfwMakeContextCurrent(previous);
#endif
    self->camera()->set_projection_matrix(original);snapshot_active_=active;return result;
}

}
