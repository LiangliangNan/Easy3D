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
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/framebuffer_object.h>
#include <easy3d/renderer/opengl_error.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/util/logging.h>


namespace easy3d {

    SurfaceMeshPicker::SurfaceMeshPicker(const Camera *cam)
            : Picker(cam)
            , hit_resolution_(15)
    {
        use_gpu_if_supported_ = true;
    }


    SurfaceMesh::Face SurfaceMeshPicker::pick_face(SurfaceMesh *model, int x, int y) {
        if (!model)
            return SurfaceMesh::Face();

        ShaderProgram* program = nullptr;
        if (use_gpu_if_supported_) {
            program = ShaderManager::get_program("selection/selection_single_primitive");
            if (!program) {
                std::vector<ShaderProgram::Attribute> attributes;
                attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
                program = ShaderManager::create_program_from_files("selection/selection_single_primitive", attributes);
            }
            if (!program) {
                use_gpu_if_supported_ = false;
                LOG_N_TIMES(3, ERROR) << "shader program not available, fall back to CPU implementation. " << COUNTER;
            }
        }

        if (use_gpu_if_supported_ && program)
            return pick_face_gpu(model, x, y, program);
        else // CPU with OpenMP (if supported)
            return pick_face_cpu(model, x, y);
    }


    SurfaceMesh::Vertex
    SurfaceMeshPicker::pick_vertex(SurfaceMesh *model, SurfaceMesh::Face picked_face, int x, int y) {
        if (!picked_face.is_valid() || picked_face != picked_face_) {
            LOG(ERROR) << "user provided face is not valid";
            return SurfaceMesh::Vertex();
        }

        const vec3& point = picked_point(model, picked_face, x, y);
        double squared_distance = FLT_MAX;
        SurfaceMesh::Vertex closest_vertex;
        for (auto h : model->halfedges(picked_face)) {
            SurfaceMesh::Vertex v = model->target(h);
            double s = distance2(model->position(v), point);
            if (s < squared_distance) {
                squared_distance = s;
                closest_vertex = v;
            }
        }

        //////////////////////////////////////////////////////////////////////////

        // always check
        if (!closest_vertex.is_valid())
            return closest_vertex;

        const vec3 &p = model->position(closest_vertex);
        const vec2 q(project(p));
        const float sqr_dist = distance2(q, vec2(static_cast<float>(x), static_cast<float>(y)));
        if (sqr_dist < static_cast<float>(hit_resolution_ * hit_resolution_))
            return closest_vertex;
        else
            return SurfaceMesh::Vertex();
    }


    SurfaceMesh::Vertex SurfaceMeshPicker::pick_vertex(SurfaceMesh *model, int x, int y) {
        SurfaceMesh::Face face = pick_face(model, x, y);
        return pick_vertex(model, face, x, y);
    }


    SurfaceMesh::Halfedge
    SurfaceMeshPicker::pick_edge(SurfaceMesh *model, SurfaceMesh::Face picked_face, int x, int y) {
        if (!picked_face.is_valid() || picked_face != picked_face_) {
            LOG(ERROR) << "user provided face is not valid";
            return SurfaceMesh::Halfedge();
        }

        const vec3& point = picked_point(model, picked_face, x, y);

        double squared_distance = FLT_MAX;
        SurfaceMesh::Halfedge closest_edge;

        // for edges that have duplicate vertices
        static double threshold = 1e-10;
        for (auto h : model->halfedges(picked_face)) {
            const vec3 &s = model->position(model->source(h));
            const vec3 &t = model->position(model->target(h));
            if (distance2(s, t) > threshold) {
                Segment3 seg(s, t);
                double d = seg.squared_distance(point);
                if (d < squared_distance/* && seg.projected_inside(point)*/) {
                    squared_distance = d;
                    closest_edge = h;
                }
            }
        }

        //////////////////////////////////////////////////////////////////////////

        // always check
        if (!closest_edge.is_valid())
            return SurfaceMesh::Halfedge();

        const vec3 &s = model->position(model->source(closest_edge));
        const vec3 &t = model->position(model->target(closest_edge));
        const Segment2 seg(vec2(project(s)), vec2(project(t)));
        const float sqr_dist = seg.squared_distance(vec2(static_cast<float>(x), static_cast<float>(y)));
        if (sqr_dist < static_cast<float>(hit_resolution_ * hit_resolution_))
            return closest_edge;
        else
            return SurfaceMesh::Halfedge();
    }


    SurfaceMesh::Halfedge SurfaceMeshPicker::pick_edge(SurfaceMesh *model, int x, int y) {
        SurfaceMesh::Face facet = pick_face(model, x, y);
        return pick_edge(model, facet, x, y);
    }


    Plane3 SurfaceMeshPicker::face_plane(SurfaceMesh *model, SurfaceMesh::Face face) const {
        auto h = model->halfedge(face);
        auto v = model->target(h);
        return Plane3(model->position(v), model->compute_face_normal(face));
    }


    SurfaceMesh::Face SurfaceMeshPicker::picked_face() const {
        if (!picked_face_.is_valid()) {
            LOG(ERROR) << "no face has been picked";
        }
        return picked_face_;
    }


    vec3 SurfaceMeshPicker::picked_point(SurfaceMesh *model, SurfaceMesh::Face face, int x, int y) const {
        if (!picked_face_.is_valid() || !face.is_valid() || picked_face_ != face) {
            LOG(ERROR) << "no face has been picked";
            return vec3();
        }

        const Line3 line = picking_line(x, y);
        const Plane3 plane = face_plane(model, face);

        vec3 p;
        if (plane.intersect(line, p))
            return p;
        else {
            LOG(ERROR) << "should have intersection";
            return vec3();
        }
    }


    SurfaceMesh::Face SurfaceMeshPicker::pick_face_cpu(SurfaceMesh *model, int x, int y) {
        int num = static_cast<int>(model->n_faces());
        const vec3 &p_near = unproject(x, y, 0);
        const vec3 &p_far = unproject(x, y, 1);
        const OrientedLine3 oline(p_near, p_far);

        std::vector<char> status(num, 0);
#pragma omp parallel for
        for (int i = 0; i < num; ++i) {
            if (do_intersect(model, SurfaceMesh::Face(i), oline))
                status[i] = 1;
        }

        picked_face_ = SurfaceMesh::Face();
        double squared_distance = FLT_MAX;
        const Line3 &line = picking_line(x, y);
        for (int i = 0; i < num; ++i) {
            if (status[i]) {
                const SurfaceMesh::Face face(i);
                const Plane3 plane = face_plane(model, face);

                vec3 p;
                if (plane.intersect(line, p)) {
                    double s = distance2(p, p_near);
                    if (s < squared_distance) {
                        squared_distance = s;
                        picked_face_ = face;
                    }
                } else {
                    // If reached here, a parallel facet with distance less than hit resolution should be
                    // the candidate. However, the picking line does not intersect the facet.
                    // Logger::err(title()) << "should have intersection";
                }
            }
        }

        return picked_face_;
    }


    SurfaceMesh::Face SurfaceMeshPicker::pick_face_gpu(SurfaceMesh *model, int x, int y, ShaderProgram* program) {
        auto drawable = model->renderer()->get_triangles_drawable("faces");
        if (!drawable) {
            const auto& drawables = model->renderer()->triangles_drawables();
            if (drawables.empty()) {
                LOG(WARNING) << "surface mesh doesn't have a TrianglesDrawable";
                return SurfaceMesh::Face();
            }
            else {
                drawable = drawables[0];
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
        setup_framebuffer(width, height);

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

        unsigned char c[4];
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

#if 0 // If the mesh is a triangle mesh.
        picked_face_ = SurfaceMesh::Face(id);
        return picked_face_;
#else
        if (id >= 0) {
            // We draw the polygonal faces as triangles and the picked id is the index of the picked triangle. So we
            // need to figure out from which face this triangle comes from.
            auto triangle_range = model->get_face_property<std::pair<int, int> >("f:triangle_range");
            if (!triangle_range) {
                LOG(ERROR) << "face property 'f:triangle_range' not defined. Selection aborted";
                return SurfaceMesh::Face();
            }

            // Triangle meshes are more common... So treat is as a triangle mesh and check if the id is within the range.
            if (static_cast<unsigned int>(id) < model->n_faces()) {
                auto face = SurfaceMesh::Face(id);
                const auto &range = triangle_range[face];
                if (id >= range.first && id <= range.second) {
                    picked_face_ = face;
                    return picked_face_;
                }
            }

            // Now treat the model as a general polygonal mesh.
            for (unsigned int face_id = 0; face_id < model->n_faces(); ++face_id) {
                auto face = SurfaceMesh::Face(static_cast<int>(face_id));
                const auto &range = triangle_range[face];
                if (id >= range.first && id <= range.second) {
                    picked_face_ = face;
                    return picked_face_;
                }
            }
        }
#endif

        return SurfaceMesh::Face();
    }


    std::vector<SurfaceMesh::Face> SurfaceMeshPicker::pick_faces(SurfaceMesh *model, const Rect& rect) {
        std::vector<SurfaceMesh::Face> faces;
        if (!model)
            return faces;

        const int win_width = camera()->screenWidth();
        const int win_height = camera()->screenHeight();

        float xmin = rect.left() / static_cast<float>(win_width - 1);
        float ymin = 1.0f - rect.top() / static_cast<float>(win_height - 1);
        float xmax = rect.right() / static_cast<float>(win_width - 1);
        float ymax = 1.0f - rect.bottom() / static_cast<float>(win_height - 1);
        if (xmin > xmax) std::swap(xmin, xmax);
        if (ymin > ymax) std::swap(ymin, ymax);

        const auto &points = model->get_vertex_property<vec3>("v:point").vector();
        const int num = static_cast<int>(points.size());
        const mat4 MANIP = model->manipulator() ? model->manipulator()->matrix() : mat4::identity();
        const mat4 &m = camera()->modelViewProjectionMatrix() * MANIP;

        std::vector<bool> status(num, false);

#pragma omp parallel for
        for (int i = 0; i < num; ++i) {
            const vec3 &p = points[i];
            float x = m[0] * p.x + m[4] * p.y + m[8] * p.z + m[12];
            float y = m[1] * p.x + m[5] * p.y + m[9] * p.z + m[13];
            //float z = m[2] * p.x + m[6] * p.y + m[10] * p.z + m[14]; // I don't need z
            float w = m[3] * p.x + m[7] * p.y + m[11] * p.z + m[15];
            x /= w;
            y /= w;
            x = 0.5f * x + 0.5f;
            y = 0.5f * y + 0.5f;

            if (x >= xmin && x <= xmax && y >= ymin && y <= ymax)
                status[i] = true;
        }

        // a face is selected if all its vertices are selected
        for (auto f : model->faces()) {
            bool selected = true;
            for (auto v : model->vertices(f)) {
                if (!status[v.idx()]) {
                    selected = false;
                    break;
                }
            }

            if (selected)
                faces.push_back(f);
        }

        return faces;
    }


    std::vector<SurfaceMesh::Face> SurfaceMeshPicker::pick_faces(SurfaceMesh *model, const Polygon2 &plg) {
        std::vector<SurfaceMesh::Face> faces;
        if (!model)
            return faces;

        const int win_width = camera()->screenWidth();
        const int win_height = camera()->screenHeight();

        std::vector<vec2> region; // the transformed selection region
        for (std::size_t i = 0; i < plg.size(); ++i) {
            const vec2 &p = plg[i];
            const float x = p.x / float(win_width - 1);
            const float y = 1.0f - p.y / float(win_height - 1);
            region.emplace_back(vec2(x, y));
        }

        const Box2& box = plg.bbox();
        float xmin = box.min_point().x / static_cast<float>(win_width - 1);
        float ymin = 1.0f - box.min_point().y / static_cast<float>(win_height - 1);
        float xmax = box.max_point().x / static_cast<float>(win_width - 1);
        float ymax = 1.0f - box.max_point().y / static_cast<float>(win_height - 1);
        if (xmin > xmax) std::swap(xmin, xmax);
        if (ymin > ymax) std::swap(ymin, ymax);

        const auto &points = model->get_vertex_property<vec3>("v:point").vector();
        const int num = static_cast<int>(points.size());
        const mat4 MANIP = model->manipulator() ? model->manipulator()->matrix() : mat4::identity();
        const mat4 &m = camera()->modelViewProjectionMatrix() * MANIP;

        std::vector<bool> select_vertices(num, false);

#pragma omp parallel for
        for (int i = 0; i < num; ++i) {
            const vec3& p = points[i];
            float x = m[0] * p.x + m[4] * p.y + m[8] * p.z + m[12];
            float y = m[1] * p.x + m[5] * p.y + m[9] * p.z + m[13];
            //float z = m[2] * p.x + m[6] * p.y + m[10] * p.z + m[14]; // I don't need z
            float w = m[3] * p.x + m[7] * p.y + m[11] * p.z + m[15];
            x /= w;
            y /= w;
            x = 0.5f * x + 0.5f;
            y = 0.5f * y + 0.5f;

            if (x >= xmin && x <= xmax && y >= ymin && y <= ymax) {
                if (geom::point_in_polygon(vec2(x, y), region))
                    select_vertices[i] = true;
            }
        }

        // a face is selected if all its vertices are selected
        for (auto f : model->faces()) {
            bool selected = true;
            for (auto v : model->vertices(f)) {
                if (!select_vertices[v.idx()]) {
                    selected = false;
                    break;
                }
            }

            if (selected)
                faces.push_back(f);
        }

        return faces;
    }


}
