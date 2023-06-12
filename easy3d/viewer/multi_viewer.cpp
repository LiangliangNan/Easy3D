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


namespace easy3d {


    MultiViewer::MultiViewer(int rows, int cols, const std::string &title)
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
        view_width_ = static_cast<int>(static_cast<float>(width()) / static_cast<float>(num_cols_));
        view_height_ = static_cast<int>(static_cast<float>(height()) / static_cast<float>(num_rows_));
    }


    MultiViewer::~MultiViewer() {
        VertexArrayObject::release_buffer(division_vertex_buffer_);
        delete division_vao_;

        // Not needed: it will be called in the destructor of the base class
        // Viewer::cleanup();
    }


    void MultiViewer::assign(int row, int col, const Model *m) {
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


    void MultiViewer::assign(int row, int col, const Drawable *d) {
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


    void MultiViewer::init() {
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


    void MultiViewer::draw() const {
        // remember the viewer port, later we will have to restore it after drawing each view
        ivec4 viewport;
        glGetIntegerv(GL_VIEWPORT, viewport);

        // ------------------------------------------------------------

        for (const auto& row : views_) {
            for (const auto& view : row) {
                const auto &vp = view.viewport;
                glViewport(vp[0], vp[1], vp[2], vp[3]);
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

        // restore viewport.
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

        // draw the division of views
        if (division_visible_)
            draw_division();
    }


    void MultiViewer::draw_division() const {
        if (!lines_program_)
            return;

        const float depth = -1.0f;
        lines_program_->bind();
        lines_program_->set_uniform("screen_color", vec4(0, 0, 0, 1.0f));
        lines_program_->set_uniform("depth", depth);
        division_vao_->bind();
        const unsigned int vertex_count = (num_rows_ - 1) * 2 + (num_cols_ - 1) * 2;
        glDrawArrays(GL_LINES, 0, static_cast<int>(vertex_count));
        division_vao_->release();
        lines_program_->release();
        easy3d_debug_log_gl_error
    }


    void MultiViewer::post_resize(int w, int h) {
        update_division();
    }


    void MultiViewer::update_division() {
        if (views_.empty() || views_[0].empty())
            return;

        if (!division_vao_)
            division_vao_ = new VertexArrayObject;

        view_width_ = static_cast<int>(static_cast<float>(width()) / static_cast<float>(num_cols_));
        view_height_ = static_cast<int>(static_cast<float>(height()) / static_cast<float>(num_rows_));
        // This is required to ensure a correct aspect ratio (thus the correct projection matrix)
        camera()->setScreenWidthAndHeight(view_width_, view_height_);

        for (int i = 0; i < num_rows_; ++i) {
            auto &row = views_[i];
            const auto y = height() - (i + 1) * view_height_;
            for (int j = 0; j < num_cols_; ++j)
                row[j].viewport = ivec4(
                        static_cast<int>(static_cast<float>(j * view_width_) * dpi_scaling()),
                        static_cast<int>(static_cast<float>(y) * dpi_scaling()),
                        static_cast<int>(static_cast<float>(view_width_) * dpi_scaling()),
                        static_cast<int>(static_cast<float>(view_height_) * dpi_scaling())
                );
        }

        // ------------------------------------------------------------

        // Note: we need NDC
        std::vector<vec2> points;
        for (std::size_t i = 1; i < num_rows_; ++i) {
            const float y = 2.0f * static_cast<float>(i * view_height_) / static_cast<float>(height()) - 1.0f;
            points.emplace_back(vec2(-1.0f, y));
            points.emplace_back(vec2(1.0f, y));
        }
        for (std::size_t i = 1; i < num_cols_; ++i) {
            const float x = 2.0f * static_cast<float>(i * view_width_) / static_cast<float>(width()) - 1.0f;
            points.emplace_back(vec2(x, -1.0f));
            points.emplace_back(vec2(x, 1.0f));
        }
        division_vao_->create_array_buffer(division_vertex_buffer_, ShaderProgram::POSITION, points.data(),
                                           points.size() * sizeof(vec2), 2, true);
        easy3d_debug_log_gl_error
    }


    vec3 MultiViewer::point_under_pixel(int x, int y, bool &found) const {
        // GLFW (same as Qt) uses upper corner for its origin while GL uses the lower corner.
        int glx = x;
        int gly = height() - 1 - y;

        // NOTE: when dealing with OpenGL, we always work in the highdpi screen space
#if defined(__APPLE__)
        glx = static_cast<int>(static_cast<float>(glx) * dpi_scaling());
        gly = static_cast<int>(static_cast<float>(gly) * dpi_scaling());
#endif
        float depth = std::numeric_limits<float>::max();
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glReadPixels(glx, gly, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depth);
        found = depth < 1.0f;
        if (found) {
            // This is different from Viewer::point_under_pixel(...).
            // We use the coordinates w.r.t. the current view.
            x %= view_width_;
            y %= view_height_;
            vec3 point(static_cast<float>(x), static_cast<float>(y), depth);
            point = camera_->unprojectedCoordinatesOf(point);
            return point;
        } else
            return vec3(0);
    }


    bool MultiViewer::mouse_press_event(int x, int y, int button, int modifiers) {
        // This re-implementation does nothing extra but just hides the pivot point.
        bool result = Viewer::mouse_press_event(x, y, button, modifiers);
        show_pivot_point_ = false;
        return result;
    }


    bool MultiViewer::mouse_release_event(int x, int y, int button, int modifiers) {
        // make the mouse position relative to the current view
        x %= view_width_;
        y %= view_height_;
        mouse_pressed_x_ %= view_width_;
        mouse_pressed_y_ %= view_height_;
        return Viewer::mouse_release_event(x, y, button, modifiers);
    }


    bool MultiViewer::mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) {
        // make the mouse position relative to the current view
        x %= view_width_;
        y %= view_height_;
        return Viewer::mouse_drag_event(x, y, dx, dy, button, modifiers);
    }

}