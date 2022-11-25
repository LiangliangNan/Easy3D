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


#include <easy3d/gui/picker_point_cloud.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/framebuffer_object.h>
#include <easy3d/renderer/opengl_error.h>
#include <easy3d/util/logging.h>


namespace easy3d {


    PointCloudPicker::PointCloudPicker(const Camera *cam)
            : Picker(cam)
            , program_(nullptr)
            , hit_resolution_(15)
    {
        use_gpu_if_supported_ = true;
    }


    PointCloud::Vertex PointCloudPicker::pick_vertex(PointCloud* model, int x, int y) {
        auto drawable = model->renderer()->get_points_drawable("vertices");
        if (!drawable)
            return PointCloud::Vertex();

        if (use_gpu_if_supported_) {
            std::string shader_name = "selection/selection_single_primitive";
            if (drawable->impostor_type() == PointsDrawable::SPHERE)
                shader_name = "selection/selection_pointcloud_single_point_as_sphere_sprite";

            program_ = ShaderManager::get_program(shader_name);
            if (!program_) {
                std::vector<ShaderProgram::Attribute> attributes;
                attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
                program_ = ShaderManager::create_program_from_files(shader_name, attributes);
            }
            if (!program_) {
                use_gpu_if_supported_ = false;
                LOG_N_TIMES(3, ERROR) << "shader program not available, fall back to CPU implementation. " << COUNTER;
            }
        }

        if (use_gpu_if_supported_ && program_) {
            switch (drawable->impostor_type()) {
                case PointsDrawable::PLAIN:
                    return pick_vertex_gpu_plain(model, x, y);
                case PointsDrawable::SPHERE:
                    return pick_vertex_gpu_sphere(model, x, y);
                case PointsDrawable::SURFEL:
                    LOG_N_TIMES(3, WARNING) << "picking points rendered as surfels is not implemented yet";
                    return pick_vertex_gpu_sphere(model, x, y);
            }
        }

        // CPU with OpenMP (if supported)
        return pick_vertex_cpu(model, x, y);
    }


    PointCloud::Vertex PointCloudPicker::pick_vertex_cpu(PointCloud* model, int px, int py) {
        const std::vector<vec3>& points = model->points();
        std::size_t num = points.size();

        std::vector<char> status(num, 0);
        std::vector<float> sqr_dist_to_near(num, FLT_MAX);

        const Line3& line = picking_line(px, py);
        const vec3& p_near = line.point();

        const auto sqr_dist_thresh = static_cast<float>(hit_resolution_ * hit_resolution_);
        // Get combined model-view and projection matrix
        const mat4& MVP = camera()->modelViewProjectionMatrix();
        // transformation introduced by manipulation
        const mat4 MANIP = model->manipulator() ? model->manipulator()->matrix() : mat4::identity();
        const mat4& m = MVP * MANIP;

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
            if (distance2(vec2(x, y), vec2(static_cast<float>(px), static_cast<float>(py))) < sqr_dist_thresh) {
                status[i] = 1;
                sqr_dist_to_near[i] = distance2(p, p_near);
            }
        }

        int idx = -1;
        float min_s_dist = FLT_MAX;
        for (int i = 0; i < num; ++i) {
            if (status[i] == 1 && sqr_dist_to_near[i] < min_s_dist) {
                min_s_dist = sqr_dist_to_near[i];
                idx = i;
            }
        }

        return PointCloud::Vertex(idx);
    }


    PointCloud::Vertex PointCloudPicker::pick_vertex_gpu_plain(PointCloud *model, int x, int y) {
        auto drawable = model->renderer()->get_points_drawable("vertices");
        if (!drawable) {
            LOG_N_TIMES(3, WARNING) << "drawable 'vertices' does not exist. " << COUNTER;
            return PointCloud::Vertex();
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

        const mat4 MANIP = model->manipulator() ? model->manipulator()->matrix() : mat4::identity();
        program_->bind();
        program_->set_uniform("MVP", camera()->modelViewProjectionMatrix())
                ->set_uniform("MANIP", MANIP);
        drawable->gl_draw();
        program_->release();

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
        return PointCloud::Vertex(id);
    }


    PointCloud::Vertex PointCloudPicker::pick_vertex_gpu_sphere(PointCloud *model, int x, int y) {
        auto drawable = model->renderer()->get_points_drawable("vertices");
        if (!drawable) {
            LOG_N_TIMES(3, WARNING) << "drawable 'vertices' does not exist. " << COUNTER;
            return PointCloud::Vertex();
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

        glEnable(GL_PROGRAM_POINT_SIZE); // before OpenGL3.2, use GL_VERTEX_PROGRAM_POINT_SIZE

        // Bind the offscreen fbo for drawing
        fbo_->bind();
        easy3d_debug_log_gl_error
        easy3d_debug_log_frame_buffer_error

        float color[4];
        glGetFloatv(GL_COLOR_CLEAR_VALUE, color);
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const mat4 MANIP = model->manipulator() ? model->manipulator()->matrix() : mat4::identity();
        program_->bind();
        program_->set_uniform("perspective", camera()->type() == Camera::PERSPECTIVE);
        program_->set_uniform("MV", camera_->modelViewMatrix());
        program_->set_uniform("PROJ", camera_->projectionMatrix());
        program_->set_uniform("MANIP", MANIP);
        float ratio = camera_->pixelGLRatio(camera_->pivotPoint());
        program_->set_uniform("sphere_radius", drawable->point_size() * ratio * 0.5f);  // 0.5f from size -> radius
        program_->set_uniform("screen_width", width);
        drawable->gl_draw();
        program_->release();
        easy3d_debug_log_gl_error
        easy3d_debug_log_frame_buffer_error

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

        glEnable(GL_PROGRAM_POINT_SIZE); // before OpenGL3.2, use GL_VERTEX_PROGRAM_POINT_SIZE

        // restore the clear color
        glClearColor(color[0], color[1], color[2], color[3]);

        //--------------------------------------------------------------------------

        // Convert the color back to an integer ID
        int id = color::encode(c[0], c[1], c[2], c[3]);
        return PointCloud::Vertex(id);
    }
    

    void PointCloudPicker::pick_vertices(PointCloud *model, const Rect& rect, bool deselect) {
        if (!model)
            return;
        int win_width = camera()->screenWidth();
        int win_height = camera()->screenHeight();

        float xmin = rect.left() / static_cast<float>(win_width - 1);
        float ymin = 1.0f - rect.top() / static_cast<float>(win_height - 1);
        float xmax = rect.right() / static_cast<float>(win_width - 1);
        float ymax = 1.0f - rect.bottom() / static_cast<float>(win_height - 1);
        if (xmin > xmax) std::swap(xmin, xmax);
        if (ymin > ymax) std::swap(ymin, ymax);

        const auto &points = model->get_vertex_property<vec3>("v:point").vector();
        int num = static_cast<int>(points.size());
        const mat4 MANIP = model->manipulator() ? model->manipulator()->matrix() : mat4::identity();
        const mat4 &m = camera()->modelViewProjectionMatrix() * MANIP;

        auto &select = model->vertex_property<bool>("v:select").vector();

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
                select[i] = !deselect;
        }

        auto count = std::count(select.begin(), select.end(), true);
        LOG(INFO) << "current selection: " << count << " points";
    }


    void PointCloudPicker::pick_vertices(PointCloud *model, const Polygon2 &plg, bool deselect) {
        if (!model)
            return;

        const int win_width = camera()->screenWidth();
        const int win_height = camera()->screenHeight();

        std::vector<vec2> region; // the transformed selection region
        for (std::size_t i = 0; i < plg.size(); ++i) {
            const vec2 &p = plg[i];
            const float x = p.x / static_cast<float>(win_width - 1);
            const float y = 1.0f - p.y / static_cast<float>(win_height - 1);
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
        int num = static_cast<int>(points.size());
        const mat4 MANIP = model->manipulator() ? model->manipulator()->matrix() : mat4::identity();
        const mat4 &m = camera()->modelViewProjectionMatrix() * MANIP;

        auto& select = model->vertex_property<bool>("v:select").vector();

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

            if (x >= xmin && x <= xmax && y >= ymin && y <= ymax) {
                if (geom::point_in_polygon(vec2(x, y), region))
                    select[i] = !deselect;
            }
        }

        auto count = std::count(select.begin(), select.end(), true);
        LOG(INFO) << "current selection: " << count << " points";
    }

}
