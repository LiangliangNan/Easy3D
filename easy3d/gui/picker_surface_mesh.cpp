/** \cond LICENSE
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
 \endcond */


#include <easy3d/gui/picker_surface_mesh.h>
#include <easy3d/renderer/transform.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/util/logging.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <utility>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_triangles.h>
#ifdef EASY3D_HAS_VULKAN
#include <easy3d/renderer/render_device.h>
#else
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/framebuffer_object.h>
#include <easy3d/renderer/opengl_error.h>
#include <easy3d/renderer/backends/opengl/query_state.h>
#endif

namespace easy3d {
namespace {
    // The named 'faces' drawable, otherwise the first triangles drawable.
    std::shared_ptr<TrianglesDrawable> face_drawable(SurfaceMesh* model) {
        auto* renderer=model->renderer();
        if(!renderer || !renderer->is_visible()) return {};
        auto* named=renderer->get_triangles_drawable("faces");
        const auto& drawables=renderer->triangles_drawables();
        if(named) for(const auto& drawable:drawables) if(drawable.get()==named) return drawable;
        if(drawables.empty()) {
            LOG(WARNING)<<"surface mesh doesn't have a TrianglesDrawable";
            return {};
        }
        std::string msg="default drawable 'faces' not found (available drawables are: ";
        for(std::size_t i=0;i<drawables.size();++i) {
            msg+=drawables[i]->name();
            if(i!=drawables.size()-1) msg+=", ";
        }
        LOG(WARNING)<<msg<<"). Picking from drawable '"+drawables[0]->name()+"'";
        return drawables[0];
    }

    // The rasterized primitive ID maps back through the face's triangle range.
    SurfaceMesh::Face face_from_primitive(SurfaceMesh* model,std::uint32_t primitive) {
        const auto ranges=model->get_face_property<std::pair<int,int>>("f:triangle_range");
        if(ranges) for(auto face:model->faces()) {
            const auto range=ranges[face];
            if(range.first>=0 && range.second>=range.first &&
               primitive>=std::uint32_t(range.first) && primitive<=std::uint32_t(range.second)) return face;
        }
        return SurfaceMesh::Face();
    }
}

SurfaceMeshPicker::SurfaceMeshPicker(const Camera* cam):Picker(cam),hit_resolution_(15) {}

SurfaceMesh::Face SurfaceMeshPicker::pick_face(SurfaceMesh* model,int x,int y) {
    picked_face_=SurfaceMesh::Face();picked_model_=nullptr;picked_point_=vec3(0);
    picked_x_=picked_y_=-1;
    if(!model || x<0 || y<0 || x>=camera()->screen_width() || y>=camera()->screen_height()) return picked_face_;
    const auto drawable=face_drawable(model);
    if(!drawable || !drawable->is_visible()) return picked_face_;
#ifdef EASY3D_HAS_VULKAN
    if(!drawable->has_cpu_buffers()) return picked_face_;
#endif
    picked_face_=pick_face_impl(model,drawable,x,y);
    if(picked_face_.is_valid()) {
        picked_model_=model;
        picked_x_=x;picked_y_=y;
    }
    return picked_face_;
}

SurfaceMesh::Vertex SurfaceMeshPicker::pick_vertex(SurfaceMesh* model,int x,int y) {
    return pick_vertex(model,pick_face(model,x,y),x,y);
}
SurfaceMesh::Vertex SurfaceMeshPicker::pick_vertex(SurfaceMesh* model,SurfaceMesh::Face face,int x,int y) {
    SurfaceMesh::Vertex result;
    if(!model || model!=picked_model_ || face!=picked_face_ || !model->is_valid(face) || model->is_deleted(face)) return result;
    const mat4 transform=model->manipulator()?model->manipulator()->matrix():mat4::identity();
    double closest=double(hit_resolution_)*hit_resolution_;
    for(auto vertex:model->vertices(face)) {
        const auto p=camera()->projected_coordinates_of(transform*model->position(vertex));
        if(!std::isfinite(p.x) || !std::isfinite(p.y) || p.z<0 || p.z>=1) continue;
        const double distance=distance2(vec2(p),vec2(float(x),float(y)));
        if(distance<closest) {closest=distance;result=vertex;}
    }
    return result;
}
SurfaceMesh::Halfedge SurfaceMeshPicker::pick_edge(SurfaceMesh* model,int x,int y) {
    return pick_edge(model,pick_face(model,x,y),x,y);
}
SurfaceMesh::Halfedge SurfaceMeshPicker::pick_edge(SurfaceMesh* model,SurfaceMesh::Face face,int x,int y) {
    SurfaceMesh::Halfedge result;
    if(!model || model!=picked_model_ || face!=picked_face_ || !model->is_valid(face) || model->is_deleted(face)) return result;
    const mat4 transform=model->manipulator()?model->manipulator()->matrix():mat4::identity();
    double closest=double(hit_resolution_)*hit_resolution_;
    for(auto edge:model->halfedges(face)) {
        const auto a=camera()->projected_coordinates_of(transform*model->position(model->source(edge)));
        const auto b=camera()->projected_coordinates_of(transform*model->position(model->target(edge)));
        if(!std::isfinite(a.x) || !std::isfinite(a.y) || !std::isfinite(b.x) || !std::isfinite(b.y) ||
           a.z<0 || a.z>=1 || b.z<0 || b.z>=1 || distance2(vec2(a),vec2(b))<1e-12f) continue;
        const double distance=Segment2(vec2(a),vec2(b)).squared_distance(vec2(float(x),float(y)));
        if(distance<closest) {closest=distance;result=edge;}
    }
    return result;
}
SurfaceMesh::Face SurfaceMeshPicker::picked_face() const {return picked_face_;}

vec3 SurfaceMeshPicker::picked_point(SurfaceMesh* model,SurfaceMesh::Face face,int x,int y) const {
    if(!model || model!=picked_model_ || face!=picked_face_ || !model->is_valid(face) || model->is_deleted(face) ||
       x!=picked_x_ || y!=picked_y_) return vec3(0);
    return picked_point_;
}
std::vector<SurfaceMesh::Face> SurfaceMeshPicker::pick_faces(SurfaceMesh* model,const Rect& rect) {
    Polygon2 polygon;
    polygon.push_back(vec2(rect.left(),rect.top()));polygon.push_back(vec2(rect.right(),rect.top()));
    polygon.push_back(vec2(rect.right(),rect.bottom()));polygon.push_back(vec2(rect.left(),rect.bottom()));
    return pick_faces(model,polygon);
}
std::vector<SurfaceMesh::Face> SurfaceMeshPicker::pick_faces(SurfaceMesh* model,const Polygon2& polygon) {
    std::vector<SurfaceMesh::Face> result;
    if(!model || polygon.size()<3) return result;
    bool area=false;
    for(const auto& p:polygon) if(!std::isfinite(p.x) || !std::isfinite(p.y)) return result;
    for(size_t i=1;i+1<polygon.size();++i) {
        auto a=polygon[i]-polygon[0],b=polygon[i+1]-polygon[0];area=area || std::abs(double(a.x)*b.y-double(a.y)*b.x)>1e-8;
    }
    if(!area) return result;
    const mat4 transform=model->manipulator()?model->manipulator()->matrix():mat4::identity();
    const auto mvp=camera()->model_view_projection_matrix()*transform;
    const auto& points=model->points();std::vector<unsigned char> selected(points.size(),0);
    const auto* clipping=ClippingPlane::instance();
    // Byte flags avoid vector<bool>'s shared-word writes under OpenMP.
#pragma omp parallel for
    for(int i=0;i<static_cast<int>(points.size());++i) {
        const vec4 clip=mvp*vec4(points[i],1);
        if(!std::isfinite(clip.x) || !std::isfinite(clip.y) || !std::isfinite(clip.z) || !std::isfinite(clip.w) || clip.w<=0 ||
           clip.x < -clip.w || clip.x > clip.w || clip.y < -clip.w || clip.y > clip.w || clip.z < -clip.w || clip.z>=clip.w) continue;
        if(clipping->is_enabled()) {
            const auto world=transform*points[i];const auto d=dot(clipping->normal(),world-clipping->center());
            if(d>0 || (clipping->cross_section() && d < -clipping->cross_section_width()*clipping->scene_radius())) continue;
        }
        const int viewport[] = {0,0,camera()->screen_width(),camera()->screen_height()};
        const vec2 p = vec2(transform::project(points[i],mvp,viewport,false));
        bool inside=polygon.contains(p);
        for(size_t j=0,k=polygon.size()-1;!inside && j<polygon.size();k=j++) {
            const auto edge=polygon[j]-polygon[k];const float length2=edge.length2();
            if(length2>0) {
                const float t=std::max(0.f,std::min(1.f,dot(p-polygon[k],edge)/length2));
                inside=(p-polygon[k]-t*edge).length2()<=1e-8f;
            }
        }
        selected[i]=inside;
    }
    for(auto face:model->faces()) {
        bool inside=true;for(auto vertex:model->vertices(face)) inside=inside && selected[vertex.idx()];
        if(inside) result.push_back(face);
    }
    return result;
}
#ifndef EASY3D_HAS_VULKAN
    SurfaceMesh::Face SurfaceMeshPicker::pick_face_impl(SurfaceMesh* model,
            const std::shared_ptr<TrianglesDrawable>& drawable,int x,int y) {
        if(!drawable || !drawable->is_visible()) return SurfaceMesh::Face();
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT,viewport);
        const int width=viewport[2],height=viewport[3];
        if(width<=0 || height<=0) return SurfaceMesh::Face();
        // Outside-in rendering test: draw only the faces, but write triangle IDs
        // instead of colors. The depth test keeps the nearest triangle per pixel;
        // reading one pixel tells us which face the cursor sees. Clipping is applied
        // by the same shader, so invisible faces can never win the depth test.
        const std::string name="selection/selection_single_primitive";
        auto* program=ShaderManager::get_program(name);
        if(!program)
            program=ShaderManager::create_program_from_files(name,{{ShaderProgram::POSITION,"vtx_position"}});
        if(!program)
            throw std::runtime_error("surface picking requires the selection shader '"+name+"'");
        opengl::QueryState state;
        state.prepare(width,height);
        setup_framebuffer(width,height);
        if(!fbo_->bind()) return SurfaceMesh::Face();
        glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
        glClearColor(1,1,1,1);
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        program->bind();
        const mat4 MANIP=drawable->manipulated_matrix();
        program->set_uniform("MANIP",MANIP)
               ->set_uniform("MVP_MANIP",camera()->model_view_projection_matrix()*MANIP);
        ClippingPlane::instance()->set_program(program);
        ClippingPlane::instance()->set_discard_primitives(program,drawable->plane_clip_discard_primitive());
        drawable->gl_draw();
        program->release();
        int gl_x,gl_y;
        screen_to_opengl(x,y,gl_x,gl_y,width,height);
        unsigned char c[4]={255,255,255,255};
        float depth=1;
        const bool color_ok=fbo_->read_color(c,gl_x,gl_y);
        const bool depth_ok=fbo_->read_depth(depth,gl_x,gl_y);
        fbo_->release();
        if(!color_ok || !depth_ok || depth<0 || depth>=1) return SurfaceMesh::Face();
        // White is the cleared "nothing drawn here" value; ID 0 is a valid triangle.
        const int id=color::encode(c[0],c[1],c[2],c[3]);
        if(id<0) return SurfaceMesh::Face();
        const SurfaceMesh::Face face=face_from_primitive(model,std::uint32_t(id));
        if(face.is_valid()) picked_point_=camera()->unprojected_coordinates_of(vec3(
            float(transform::scale_coordinate(gl_x+.5, width, camera()->screen_width())),
            float(transform::scale_coordinate(height-gl_y-.5, height, camera()->screen_height())), depth));
        return face;
    }
#else
    SurfaceMesh::Face SurfaceMeshPicker::pick_face_impl(SurfaceMesh* model,
            const std::shared_ptr<TrianglesDrawable>& drawable,int x,int y) {
        if(!drawable || !drawable->has_cpu_buffers()) return SurfaceMesh::Face();
        const int width=camera()->screen_width(),height=camera()->screen_height();
        if(width<=0 || height<=0) return SurfaceMesh::Face();
        // The backend RenderDevice repeats the same rasterized ID/depth pass and
        // returns the object ID, primitive ID, and depth of the sampled pixel.
        const graphics::DrawCommand command=draw_command(drawable,0);
        const auto sample=render_device().sample_pixel(std::uint32_t(width),std::uint32_t(height),
                                                       {command},std::uint32_t(x),std::uint32_t(y));
        // Word 0 is this drawable; a background pixel keeps the cleared object ID.
        if(sample.id.object!=0 || sample.depth>=1) return SurfaceMesh::Face();
        const std::uint32_t primitive=sample.id.primitive;
        SurfaceMesh::Face face;
        const auto& faces=drawable->cpu_buffers().source_faces;
        if(primitive<faces.size() && faces[primitive]>=0) {
            const SurfaceMesh::Face f(faces[primitive]);
            if(model->is_valid(f) && !model->is_deleted(f)) face=f;
        }
        // Source provenance is invalidated by buffer edits; the face ranges stay valid
        // for the unchanged draw order.
        if(!face.is_valid()) face=face_from_primitive(model,primitive);
        if(face.is_valid()) picked_point_=camera()->unprojected_coordinates_of(vec3(x+.5f,y+.5f,sample.depth));
        return face;
    }
#endif
} // namespace easy3d
