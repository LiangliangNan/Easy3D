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

#include "viewer.h"
#include <easy3d/core/point_cloud.h>
#include <easy3d/gui/picker_point_cloud.h>
#include <easy3d/renderer/text_renderer.h>
#include <easy3d/renderer/primitives.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/util/logging.h>

#include <3rd_party/glfw/include/GLFW/glfw3.h>    // for the mouse buttons

#define  TEST_LASSO     1

using namespace easy3d;

PointSelection::PointSelection(const std::string &title) : Viewer(title) {
}


std::string PointSelection::usage() const {
    return ("-------------- Point Selection usage -------------- \n"
            "Press the ALT key, then drag the mouse to select point (selected points will be deleted) \n"
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
                model->collect_garbage();
                LOG(INFO) << count << " points deleted";

                auto drawable = model->renderer()->get_points_drawable("vertices");
                drawable->update();
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

    // draw the Easy3D logo and GPU time
    if (texter_ && texter_->num_fonts() >=2) {
        const float font_size = 15.0f;
        const float offset = 20.0f * dpi_scaling();
        texter_->draw("Easy3D", offset, offset, font_size, 0);

        // the rendering time
        texter_->draw(gpu_time_, offset, 50.0f * dpi_scaling(), 16, 1);
    }

    if (polygon_.size() >= 3) {
        // draw the boundary of the rect
        opengl::draw_polygon_wire(polygon_, vec4(1.0f, 0.0f, 0.0f, 1.0f), width(), height(), -1.0f);

        // draw the transparent face
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        opengl::draw_polygon_filled(polygon_, vec4(1.0f, 0.0f, 0.0f, 0.2f), width(), height(), -0.9f);
        glDisable(GL_BLEND);
    }
}
