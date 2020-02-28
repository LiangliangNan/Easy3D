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

#include "point_selection.h"
#include <easy3d/core/point_cloud.h>
#include <easy3d/gui/picker_point_cloud.h>
#include <easy3d/viewer/drawable_lines.h>
#include <easy3d/viewer/shader_program.h>
#include <easy3d/viewer/shader_manager.h>
#include <easy3d/viewer/transform.h>
#include <easy3d/viewer/renderer.h>
#include <easy3d/util/logging.h>

#include <3rd_party/glfw/include/GLFW/glfw3.h>    // for the mouse buttons

#define  TEST_LASSO     1

using namespace easy3d;

PointSelection::PointSelection(const std::string &title) : Viewer(title) {
}


std::string PointSelection::usage() const {
    return ("-------------- Surface Reconstruction usage -------------- \n"
            "Press the ALT key, then drag the mouse for point selection \n"
            "---------------------------------------------------------- \n");
}


/// Mouse button press event handler
bool PointSelection::mouse_press_event(int x, int y, int button, int modifiers) {
    if (modifiers == GLFW_MOD_ALT) {
        polygon_.clear();
        polygon_.push_back(vec2(x, y));
        return false;
    } else
        return Viewer::mouse_press_event(x, y, button, modifiers);
}


/// Mouse button release event handler
bool PointSelection::mouse_release_event(int x, int y, int button, int modifiers) {
    if (modifiers == GLFW_MOD_ALT) {
        if (polygon_.size() >= 3) {
            auto model = dynamic_cast<PointCloud *>(current_model());
            if (model) {
                PointCloudPicker picker(camera());
#if TEST_LASSO
                picker.pick_vertices(model, polygon_, false);
#else
                picker.pick_vertices(model, Rect(polygon_[0], polygon_[2]), false);
#endif

                auto select = model->vertex_property<bool>("v:select");
                std::size_t count(0);
                for(auto v : model->vertices()) {
                    if (select[v]) {
                        model->delete_vertex(v);
                        ++count;
                    }
                }
                model->garbage_collection();
                LOG(INFO) << count << " points deleted" << std::endl;

                auto drawable = model->points_drawable("vertices");
                renderer::update_buffer(model, drawable);

                polygon_.clear();
            }
        }
        return false;
    } else
        return Viewer::mouse_release_event(x, y, button, modifiers);
}


/// Mouse drag (i.e., a mouse button was pressed) event handler
bool PointSelection::mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) {
    if (modifiers == GLFW_MOD_ALT) {
#if TEST_LASSO
        polygon_.push_back(vec2(x, y));
#else   // rectangle
        const vec2 first_point = polygon_[0];
        polygon_.clear();
        polygon_.push_back(first_point);
        polygon_.push_back(vec2(first_point.x, y));
        polygon_.push_back(vec2(x, y));
        polygon_.push_back(vec2(x, first_point.y));
#endif
        return false;
    } else
        return Viewer::mouse_drag_event(x, y, dx, dy, button, modifiers);
}


// This function will be called after the main draw procedure.
void PointSelection::post_draw() {
    Viewer::draw_corner_axes();

    if (polygon_.size() >= 3) {
        ShaderProgram *program = ShaderManager::get_program("lines/lines_plain_color");
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes;
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"));
            program = ShaderManager::create_program_from_files("lines/lines_plain_color", attributes);
        }
        if (!program)
            return;

        LinesDrawable drawable("polygon");

        //TODO: better handling 2D->3D points
        std::vector<vec3> points;
        std::size_t n=polygon_.size();
        for (std::size_t s = 0, t = 1; s < n; ++s, ++t, t %= n) {
            points.push_back(polygon_[s]);
            points.push_back(polygon_[t]);
        }
        drawable.update_vertex_buffer(points);

        const mat4 &proj = transform::ortho(0.0f, static_cast<float>(width()), static_cast<float>(height()), 0.0f, 0.0f,
                                            -1.0f);
        glDisable(GL_DEPTH_TEST);   // always on top
        program->bind();
        program->set_uniform("MVP", proj);
        program->set_uniform("per_vertex_color", false);
        program->set_uniform("default_color", vec3(0.0f, 0.0f, 1.0f));
        drawable.gl_draw(false);
        program->release();
        glEnable(GL_DEPTH_TEST);   // restore
    }
}