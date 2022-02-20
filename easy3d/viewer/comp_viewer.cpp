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

#include <easy3d/viewer/comp_viewer.h>
#include <easy3d/core/model.h>
#include <easy3d/renderer/opengl.h>
#include <easy3d/renderer/opengl_error.h>
#include <easy3d/renderer/vertex_array_object.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/manipulated_camera_frame.h>

#include <3rd_party/glfw/include/GLFW/glfw3.h>    // Include glfw3.h after our OpenGL definitions

// To have the same shortcut behavior on macOS and other platforms (i.e., Windows, Linux)
#ifdef __APPLE__
#define EASY3D_MOD_CONTROL GLFW_MOD_SUPER
#else
#define EASY3D_MOD_CONTROL GLFW_MOD_CONTROL
#endif


namespace easy3d {


    CompViewer::CompViewer(unsigned int rows, unsigned int cols, const std::string &title)
            : Viewer(title)
            , num_rows_(rows)
            , num_cols_(cols)
            , division_vao_(nullptr)
            , lines_program_(nullptr)
            , division_vertex_buffer_(0)
            , division_visible_(true)
    {
        // the views are created in the constructor to ensure they are accessible immediately
        views_.resize(num_rows_);
        for (auto &row: views_)
            row.resize(num_cols_);

        // initialized to window size
        view_width_ = width_;
        view_height_ = height_;
    }


    void CompViewer::assign(unsigned int row, unsigned int col, const Model *m) {
        if (!m) {
            LOG(ERROR) << "null model cannot be assigned to a view";
            return;
        }

        if (row >= 0 && row < num_rows_ && col >= 0 && col < num_cols_)
            views_[row][col].models.push_back(m);
        else {
            LOG(ERROR) << "view position (" << row << ", " << col
                       << ") is out of range. #rows: " << num_rows_ << ", #cols: " << num_cols_;
        }
    }


    void CompViewer::assign(unsigned int row, unsigned int col, const Drawable *d) {
        if (!d) {
            LOG(ERROR) << "null drawable cannot be assigned to a view";
            return;
        }

        if (row >= 0 && row < num_rows_ && col >= 0 && col < num_cols_) {
            const_cast<Drawable *>(d)->set_visible(true);
            views_[row][col].drawables.push_back(d);
        }
        else {
            LOG(ERROR) << "view position (" << row << ", " << col
                       << ") is out of range. #rows: " << num_rows_ << ", #cols: " << num_cols_;
        }
    }


    void CompViewer::init() {
        Viewer::init();

        // compute the division
        update_division();

        // create the shader program for visualizing the division lines
        const std::string name = "screen_space/screen_space_color";
        lines_program_ = ShaderManager::get_program(name);
        if (!lines_program_) {
            std::vector<ShaderProgram::Attribute> attributes = {
                    ShaderProgram::Attribute(ShaderProgram::POSITION, "ndc_position")
            };
            lines_program_ = ShaderManager::create_program_from_files(name, attributes);
        }
        if (!lines_program_) {
            LOG(ERROR) << "shader doesn't exist: " << name;
            return;
        }
    }


    void CompViewer::draw() const {
        // remember the viewer port and scissor status, later we will have to restore them
        ivec4 viewport, scissor;
        glGetIntegerv(GL_VIEWPORT, viewport);
        glGetIntegerv(GL_SCISSOR_BOX, scissor);

        // ------------------------------------------------------------

        for (std::size_t i = 0; i < views_.size(); ++i) {
            const auto &row = views_[i];
            for (std::size_t j = 0; j < row.size(); ++j) {
                const auto &view = row[j];
                const auto &viewport = view.viewport;
                glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
                glScissor(viewport[0], viewport[1], viewport[2], viewport[3]);
                for (const auto m: view.models) {
                    if (!m->renderer()->is_visible())
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
            }
        }

        // ------------------------------------------------------------

        // restore scissor and viewport states.
        glScissor(scissor[0], scissor[1], scissor[2], scissor[3]);
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

        // draw the division of views
        if (division_visible_)
            draw_division();
    }


    void CompViewer::draw_division() const {
        if (!lines_program_)
            return;

        const float depth = -1.0f;
        lines_program_->bind();
        lines_program_->set_uniform("screen_color", vec4(0, 0, 0, 1.0f));
        lines_program_->set_uniform("depth", depth);
        division_vao_->bind();
        const unsigned int vertex_count = (num_rows_ - 1) * 2 + (num_cols_ - 1) * 2;
        glDrawArrays(GL_LINES, 0, vertex_count);
        division_vao_->release();
        lines_program_->release();
        easy3d_debug_log_gl_error;
    }


    void CompViewer::post_resize(int w, int h) {
        update_division();
    }


    void CompViewer::cleanup() {
        division_vao_->release_buffer(division_vertex_buffer_);
        delete division_vao_;
        Viewer::cleanup();
    }


    void CompViewer::update_division() {
        if (views_.empty() || views_[0].empty())
            return;

        if (!division_vao_)
            division_vao_ = new VertexArrayObject;

        ivec4 viewport;
        glGetIntegerv(GL_VIEWPORT, viewport);
        const int w = viewport[2];
        const int h = viewport[3];
        view_width_ = int(w / float(num_cols_));
        view_height_ = int(h / float(num_rows_));
        // This is required to ensure a correct aspect ratio (thus the correct projection matrix)
        camera()->setScreenWidthAndHeight(view_width_, view_height_);

        for (std::size_t i = 0; i < num_rows_; ++i) {
            auto &row = views_[i];
            const float y = h - (i + 1) * view_height_;
            for (std::size_t j = 0; j < num_cols_; ++j)
                row[j].viewport = ivec4(j * view_width_, y, view_width_, view_height_);
        }

        // ------------------------------------------------------------

        // Note: we need NDC
        std::vector<vec2> points;
        for (std::size_t i = 1; i < num_rows_; ++i) {
            const float y = 2.0f * i * view_height_ / h - 1.0f;
            points.emplace_back(vec2(-1.0f, y));
            points.emplace_back(vec2(1.0f, y));
        }
        for (std::size_t i = 1; i < num_cols_; ++i) {
            const float x = 2.0f * i * view_width_ / w - 1.0f;
            points.emplace_back(vec2(x, -1.0f));
            points.emplace_back(vec2(x, 1.0f));
        }
        division_vao_->create_array_buffer(division_vertex_buffer_, ShaderProgram::POSITION, points.data(),
                                           points.size() * sizeof(vec2), 2, true);
        easy3d_debug_log_gl_error;
    }


    bool CompViewer::mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) {
        // make the mouse position relative to the current view
        x %= view_width_;
        y %= view_height_;

        // control modifier is reserved for zooming on region
        if (modifiers != EASY3D_MOD_CONTROL) {
            auto axis = ManipulatedFrame::NONE;
            if (pressed_key_ == GLFW_KEY_X) axis = ManipulatedFrame::HORIZONTAL;
            else if (pressed_key_ == GLFW_KEY_Y) axis = ManipulatedFrame::VERTICAL;
            else if (pressed_key_ == GLFW_KEY_O) axis = ManipulatedFrame::ORTHOGONAL;
            switch (button) {
                case GLFW_MOUSE_BUTTON_LEFT:
                    camera_->frame()->action_rotate(x, y, dx, dy, camera_, axis);
                    break;
                case GLFW_MOUSE_BUTTON_RIGHT:
                    camera_->frame()->action_translate(x, y, dx, dy, camera_, axis);
                    break;
                case GLFW_MOUSE_BUTTON_MIDDLE:
                    if (std::abs(dy) >= 1)
                        camera_->frame()->action_zoom(dy > 0 ? 1 : -1, camera_);
                    break;
            }
        }

        return false;
    }

}