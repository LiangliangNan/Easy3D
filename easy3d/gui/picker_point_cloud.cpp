/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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
 */


#include <easy3d/gui/picker_point_cloud.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/framebuffer_object.h>
#include <easy3d/renderer/opengl_error.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/opengl_info.h>
#include <easy3d/util/logging.h>


namespace easy3d {


    PointCloudPicker::PointCloudPicker(const Camera *cam)
            : Picker(cam) {
        use_gpu_if_supported_ = false;
    }


    int PointCloudPicker::pick_vertices(PointCloud *model, const Rect& rect, bool deselect) {
        if (!model)
            return 0;

        if (use_gpu_if_supported_) {
            if (OpenglInfo::gl_version_number() >= 4.3) {
                auto program = ShaderManager::get_program("selection/selection_pointcloud_rect");
                if (!program) {
                    std::vector<ShaderProgram::Attribute> attributes;
                    attributes.push_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
                    program = ShaderManager::create_program_from_files("selection/selection_pointcloud_rect",
                                                                       attributes);
                }
                if (program)
                    return pick_vertices_gpu(model, rect, deselect, program);
                else {
                    LOG_FIRST_N(1, ERROR)
                        << "shader program not available, default to CPU implementation. " << COUNTER;
                }
            }
            else {
                LOG_FIRST_N(1, WARNING)
                    << "GPU implementation requires OpenGL 4.3 or higher (available is "
                    << OpenglInfo::gl_version_number() << "), default to CPU implementation. " << COUNTER;
            }
        }

        // CPU with OpenMP (if supported)
        return pick_vertices_cpu(model, rect, deselect);
    }


    int PointCloudPicker::pick_vertices(PointCloud *model, const Polygon2 &plg, bool deselect) {
        if (!model)
            return 0;

        if (use_gpu_if_supported_) {
            if (OpenglInfo::gl_version_number() >= 4.3) {
                auto program = ShaderManager::get_program("selection/selection_pointcloud_lasso");
                if (!program) {
                    std::vector<ShaderProgram::Attribute> attributes;
                    attributes.push_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
                    program = ShaderManager::create_program_from_files("selection/selection_pointcloud_lasso",
                                                                       attributes);
                }
                if (program)
                    return pick_vertices_gpu(model, plg, deselect, program);
                else {
                    LOG_FIRST_N(1, ERROR)
                        << "shader program not available, default to CPU implementation. " << COUNTER;
                }
            } else {
                LOG_FIRST_N(1, WARNING)
                    << "GPU implementation requires OpenGL 4.3 or higher (available is "
                    << OpenglInfo::gl_version_number() << "), default to CPU implementation. " << COUNTER;
            }
        }

        // CPU with OpenMP (if supported)
        return pick_vertices_cpu(model, plg, deselect);
    }


    int PointCloudPicker::pick_vertices_cpu(PointCloud *model, const Rect& rect, bool deselect) {
        if (!model)
            return 0;

        int win_width = camera()->screenWidth();
        int win_height = camera()->screenHeight();

        float xmin = rect.left() / (win_width - 1.0f);
        float ymin = 1.0f - rect.top() / (win_height - 1.0f);
        float xmax = rect.right() / (win_width - 1);
        float ymax = 1.0f - rect.bottom() / (win_height - 1.0f);
        if (xmin > xmax) std::swap(xmin, xmax);
        if (ymin > ymax) std::swap(ymin, ymax);

        const auto &points = model->get_vertex_property<vec3>("v:point").vector();
        int num = static_cast<int>(points.size());
        const mat4 &m = camera()->modelViewProjectionMatrix() * model->manipulator()->matrix();

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

            if (x >= xmin && x <= xmax && y >= ymin && y <= ymax) {
                select[i] = true;
            }
        }

        return 0;
    }


    int PointCloudPicker::pick_vertices_gpu(PointCloud *model, const Rect& rect, bool deselect, ShaderProgram *program) {
        if (!model)
            return 0;

        auto drawable = model->renderer()->get_points_drawable("vertices");
        if (!drawable) {
            LOG_FIRST_N(1, WARNING) << "drawable 'vertices' does not exist. " << COUNTER;
            return 0;
        }

//        int num_before = drawable->num_selected();

        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        vec4 rectangle(rect.x_min(), rect.x_max(), rect.y_min(), rect.y_max());
        const mat4 &MVP = camera()->modelViewProjectionMatrix() * model->manipulator()->matrix();

        program->bind();
        program->set_uniform("viewport", viewport);
        program->set_uniform("MVP", MVP);
        program->set_uniform("rect", rectangle);
        program->set_uniform("deselect", deselect);
        drawable->gl_draw(true);
        program->release();

        //-----------------------------------------------------------------
        // Liangliang: this is not needed for rendering purpose, but just to keep the CPU data up to date. A better
        //             choice could be retrieving the data only if needed.
        drawable->fetch_selection_buffer();
        easy3d_debug_log_gl_error;
        easy3d_debug_log_frame_buffer_error;
        //-----------------------------------------------------------------

//        int num_changed = std::abs(drawable->num_selected() - num_before);
//        if (num_changed > 0) {
//            // the data in the GPU memory has already been updated by my shader
//            //model->notify_selection_change();
//        }
//        return num_changed;
        return 0;
    }


    int PointCloudPicker::pick_vertices_cpu(PointCloud *model, const Polygon2 &plg, bool deselect) {
        if (!model)
            return 0;

        int win_width = camera()->screenWidth();
        int win_height = camera()->screenHeight();

        std::vector<vec2> region; // the transformed selection region
        for (std::size_t i = 0; i < plg.size(); ++i) {
            const vec2 &p = plg[i];
            float x = p.x / float(win_width - 1);
            float y = 1.0f - p.y / float(win_height - 1);
            region.push_back(vec2(x, y));
        }

        const Box2& box = plg.bbox();
        float xmin = box.min_point().x / (win_width - 1.0f);
        float ymin = 1.0f - box.min_point().y / (win_height - 1.0f);
        float xmax = box.max_point().x / (win_width - 1);
        float ymax = 1.0f - box.max_point().y / (win_height - 1.0f);
        if (xmin > xmax) std::swap(xmin, xmax);
        if (ymin > ymax) std::swap(ymin, ymax);

        const auto &points = model->get_vertex_property<vec3>("v:point").vector();
        int num = static_cast<int>(points.size());
        const mat4 &m = camera()->modelViewProjectionMatrix() * model->manipulator()->matrix();

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

            if (x >= xmin && x <= xmax && y >= ymin && y <= ymax) {
                if (geom::point_in_polygon(vec2(x, y), region))
                    select[i] = true;
            }
        }

//        int num_changed = std::abs(drawable->num_selected() - num_before);
//        if (num_changed > 0)
//            model->notify_selection_change();
//        return num_changed;

        return 0;
    }


    int PointCloudPicker::pick_vertices_gpu(PointCloud *model, const Polygon2 &plg, bool deselect, ShaderProgram *program) {
        if (!model)
            return 0;

        auto drawable = model->renderer()->get_points_drawable("vertices");
        if (!drawable) {
            LOG_FIRST_N(1, WARNING) << "drawable 'vertices' does not exist. " << COUNTER;
            return 0;
        }

//        int num_before = drawable->num_selected();

//        // make sure the vertex buffer holds the right data.
//        if (drawable->num_primitives() != model->n_vertices()) {
//            model->notify_vertex_change();
//            model->active_renderer()->ensure_buffers(drawable);
//        }
//        drawable->update_storage_buffer(plg.data(), plg.size() * sizeof(vec2));

        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);

        program->bind();
        program->set_uniform("viewport", viewport);
        program->set_uniform("MVP", camera()->modelViewProjectionMatrix())
                ->set_uniform("MANIP", model->manipulator()->matrix());
        program->set_uniform("deselect", deselect);
        drawable->gl_draw(true);
        program->release();

        //-----------------------------------------------------------------
        // Liangliang: this is not needed for rendering purpose, but just to keep the CPU data up to date. A better
        //             choice could be retrieving the data only if needed.
        drawable->fetch_selection_buffer();
        easy3d_debug_log_gl_error;
        easy3d_debug_log_frame_buffer_error;
        //-----------------------------------------------------------------

//        int num_changed = std::abs(drawable->num_selected() - num_before);
//        if (num_changed > 0) {
//            // the data in the GPU memory has already been updated by my shader
//            //model->notify_selection_change();
//        }
//        return num_changed;
        return 0;
    }

}
