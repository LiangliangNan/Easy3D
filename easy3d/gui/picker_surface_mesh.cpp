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


#include <easy3d/gui/picker_surface_mesh.h>
#include <easy3d/algo/tessellator.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/util/logging.h>
#include <algorithm>
#include <cmath>
#include <limits>
#ifndef EASY3D_HAS_VULKAN
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/framebuffer_object.h>
#include <easy3d/renderer/opengl_error.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/backends/opengl/query_state.h>
#endif

namespace easy3d {
SurfaceMeshPicker::SurfaceMeshPicker(const Camera* cam):Picker(cam),hit_resolution_(15) {}

SurfaceMesh::Face SurfaceMeshPicker::pick_face(SurfaceMesh* model,int x,int y) {
    picked_face_=SurfaceMesh::Face();picked_model_=nullptr;
    if(!model || x<0 || y<0 || x>=camera()->screenWidth() || y>=camera()->screenHeight()) return picked_face_;
#ifndef EASY3D_HAS_VULKAN
    const bool clipped=ClippingPlane::instance()->is_enabled();
    if(use_gpu_if_supported_ && model->renderer() && !clipped) {
        opengl::QueryState state;
        state.prepare(state.viewport[2],state.viewport[3]);
        glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
        auto* program=ShaderManager::get_program("selection/selection_single_primitive");
        if(!program) {
            std::vector<ShaderProgram::Attribute> attributes;
            attributes.emplace_back(ShaderProgram::POSITION,"vtx_position");
            program=ShaderManager::create_program_from_files("selection/selection_single_primitive",attributes);
        }
        if(program) picked_face_=pick_face_gpu(model,x,y,program);
        else use_gpu_if_supported_=false;
    }
    if(!use_gpu_if_supported_ || !model->renderer() || clipped)
#endif
        picked_face_=pick_face_cpu(model,x,y);
    if(picked_face_.is_valid()) picked_model_=model;
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
        const auto p=project(transform*model->position(vertex));
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
        const auto a=project(transform*model->position(model->source(edge)));
        const auto b=project(transform*model->position(model->target(edge)));
        if(!std::isfinite(a.x) || !std::isfinite(a.y) || !std::isfinite(b.x) || !std::isfinite(b.y) ||
           a.z<0 || a.z>=1 || b.z<0 || b.z>=1 || distance2(vec2(a),vec2(b))<1e-12f) continue;
        const double distance=Segment2(vec2(a),vec2(b)).squared_distance(vec2(float(x),float(y)));
        if(distance<closest) {closest=distance;result=edge;}
    }
    return result;
}
SurfaceMesh::Face SurfaceMeshPicker::picked_face() const {return picked_face_;}

bool SurfaceMeshPicker::intersect_face(SurfaceMesh* model,SurfaceMesh::Face face,
                                      const vec3& origin,const vec3& direction,float& t) {
    bool found=false;t=std::numeric_limits<float>::infinity();
    const auto intersect=[&](const dvec3& a,const dvec3& b,const dvec3& c) {
        const dvec3 e1=b-a,e2=c-a,ray(direction),h=cross(ray,e2);
        const double det=dot(e1,h);
        if(std::abs(det)<=std::numeric_limits<double>::epsilon()*e1.length()*h.length()) return;
        const dvec3 offset=dvec3(origin)-a,q=cross(offset,e1);
        const double u=dot(offset,h)/det,v=dot(ray,q)/det,d=dot(e2,q)/det;
        if(u<0 || v<0 || u+v>1 || d<0 || d>=1 || !std::isfinite(d)) return;
        const auto* clipping=ClippingPlane::instance();
        if(clipping->is_enabled()) {
            const mat4 transform=model->manipulator()?model->manipulator()->matrix():mat4::identity();
            const auto world=transform*(origin+float(d)*direction);
            const auto distance=dot(clipping->normal(),world-clipping->center());
            if(distance>0 || (clipping->cross_section() && distance < -clipping->cross_section_width()*clipping->scene_radius())) return;
        }
        if(d<t) {t=float(d);found=true;}
    };
    if(model->valence(face)==3) {
        dvec3 p[3];unsigned i=0;
        for(auto vertex:model->vertices(face)) {
            const auto v=model->position(vertex);
            if(!std::isfinite(v.x) || !std::isfinite(v.y) || !std::isfinite(v.z)) return false;
            p[i++]=dvec3(v);
        }
        intersect(p[0],p[1],p[2]);
        return found;
    }
    for(auto vertex:model->vertices(face)) {
        const auto p=model->position(vertex);
        if(!std::isfinite(p.x) || !std::isfinite(p.y) || !std::isfinite(p.z)) return false;
    }
    Tessellator tess;
    tess.begin_polygon(model->compute_face_normal(face));tess.begin_contour();
    for(auto vertex:model->vertices(face)) tess.add_vertex(model->position(vertex));
    tess.end_contour();tess.end_polygon();
    for(const auto& triangle:tess.elements()) {
        dvec3 p[3];
        for(unsigned j=0;j<3;++j) {const auto* data=tess.vertices()[triangle[j]]->data();p[j]=dvec3(data[0],data[1],data[2]);}
        intersect(p[0],p[1],p[2]);
    }
    return found;
}
SurfaceMesh::Face SurfaceMeshPicker::pick_face_cpu(SurfaceMesh* model,int x,int y) {
    const mat4 transform=model->manipulator()?model->manipulator()->matrix():mat4::identity();
    const mat4 inv=inverse(transform);
    const auto origin=inv*unproject(x,y,0),direction=inv*unproject(x,y,1)-origin;
    if(!std::isfinite(direction.x) || !std::isfinite(direction.y) || !std::isfinite(direction.z)) return SurfaceMesh::Face();
    float closest=std::numeric_limits<float>::infinity();SurfaceMesh::Face result;
    for(auto face:model->faces()) {
        float t;
        if(!intersect_face(model,face,origin,direction,t) || t>=closest) continue;
        closest=t;result=face;
    }
    return result;
}
vec3 SurfaceMeshPicker::picked_point(SurfaceMesh* model,SurfaceMesh::Face face,int x,int y) const {
    if(!model || model!=picked_model_ || face!=picked_face_ || !model->is_valid(face) || model->is_deleted(face)) return vec3(0);
    const mat4 transform=model->manipulator()?model->manipulator()->matrix():mat4::identity();
    const mat4 inv=inverse(transform);
    const auto origin=inv*unproject(x,y,0),direction=inv*unproject(x,y,1)-origin;
    float t;
    return intersect_face(model,face,origin,direction,t)?transform*(origin+t*direction):vec3(0);
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
    const auto mvp=camera()->modelViewProjectionMatrix()*transform;
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
        const vec2 p((clip.x/clip.w+1)*.5f*camera()->screenWidth(),
                     camera()->screenHeight()-1-(clip.y/clip.w+1)*.5f*camera()->screenHeight());
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
    SurfaceMesh::Face SurfaceMeshPicker::pick_face_gpu(SurfaceMesh *model, int x, int y, ShaderProgram* program) {
        auto drawable = model->renderer()->get_triangles_drawable("faces");
        if (!drawable) {
            const auto& drawables = model->renderer()->triangles_drawables();
            if (drawables.empty()) {
                LOG(WARNING) << "surface mesh doesn't have a TrianglesDrawable";
                return SurfaceMesh::Face();
            }
            else {
                drawable = drawables[0].get();
                std::string msg = "default drawable 'faces' not found (available drawables are: ";
                for (std::size_t i = 0; i < drawables.size(); ++i) {
                    msg += drawables[i]->name();
                    if (i != drawables.size() - 1)
                        msg += ", ";
                }
                LOG(WARNING) << msg << "). Picking from drawable '" << drawable->name() + "'";
            }
        }

        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        int width = viewport[2];
        int height = viewport[3];
        if(width<=0 || height<=0) return SurfaceMesh::Face();
        setup_framebuffer(width, height);
        if(!fbo_->is_valid()) return SurfaceMesh::Face();

        //--------------------------------------------------------------------------
        // render the 'scene' to the new FBO.

        // TODO: the performance can be improved. Since the 'scene' is static, we need to render it to the fbo only
        //       once. Then just query. Re-rendering is needed only when the scene is changed/manipulated, or the size
        //       of the viewer changed.

        // Bind the offscreen fbo for drawing
        fbo_->bind();
        easy3d_debug_log_gl_error
        easy3d_debug_log_frame_buffer_error

        float color[4];
        glGetFloatv(GL_COLOR_CLEAR_VALUE, color);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        easy3d_debug_log_gl_error
        easy3d_debug_log_frame_buffer_error

        const mat4 MANIP = model->manipulator() ? model->manipulator()->matrix() : mat4::identity();
        program->bind();
        program->set_uniform("MVP", camera()->modelViewProjectionMatrix())
                ->set_uniform("MANIP", MANIP);
        drawable->gl_draw();
        program->release();

        // --- Maybe this is not necessary ---------
        glFlush();
        glFinish();
        // -----------------------------------------

        int gl_x, gl_y;
        screen_to_opengl(x, y, gl_x, gl_y, width, height);

        unsigned char c[4]={255,255,255,255};
        fbo_->read_color(c, gl_x, gl_y);

        // switch back to the previous fbo
        fbo_->release();
        easy3d_debug_log_gl_error
        easy3d_debug_log_frame_buffer_error

        // restore the clear color
        glClearColor(color[0], color[1], color[2], color[3]);

        //--------------------------------------------------------------------------

        // Convert the color back to an integer ID
        int id = color::encode(c[0], c[1], c[2], c[3]);
        picked_face_ = SurfaceMesh::Face();

        const auto ranges=model->get_face_property<std::pair<int,int>>("f:triangle_range");
        if(id>=0 && ranges) for(auto face:model->faces()) {
            const auto range=ranges[face];
            if(id>=range.first && id<=range.second) {picked_face_=face;break;}
        }
        return picked_face_;
    }

#endif
} // namespace easy3d
