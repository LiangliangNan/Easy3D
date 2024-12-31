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
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_lines_2D.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/framebuffer_object.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/dialog.h>


namespace easy3d {


    MultiViewer::MultiViewer(int rows, int cols, const std::string &title)
            : Viewer(title)
    {

        set_layout(rows, cols);
    }


    MultiViewer::~MultiViewer() {
        drawable_division_.reset();
        // Not needed: it will be called in the destructor of the base class
        // Viewer::cleanup();
    }


    void MultiViewer::set_layout(int rows, int cols) {
        num_rows_ = rows;
        num_cols_ = cols;

        // the views are created in the constructor to ensure they are accessible immediately
        views_.resize(num_rows_);
        for (auto &row: views_)
            row.resize(num_cols_);

        update_division();
    }


    bool MultiViewer::snapshot() const {
        const std::string& title = "Please choose a file name";
        std::string default_file_name("untitled.png");
        if (current_model())
            default_file_name = file_system::replace_extension(current_model()->name(), "png");
        const std::vector<std::string>& filters = {
                "Image Files (*.png *.jpg *.bmp *.tga)", "*.png *.jpg *.bmp *.tga",
                "All Files (*.*)", "*"
        };

        const bool warn_overwrite = true;
        const std::string& file_name = dialog::save(title, default_file_name, filters, warn_overwrite);
        if (file_name.empty())
            return false;

        const std::string& ext = file_system::extension(file_name, true);
        if (ext != "png" && ext != "jpg" && ext != "bmp" && ext != "tga") {
            LOG(ERROR) << "snapshot format must be png, jpg, bmp, or tga";
            return false;
        }

        // the following code could be made int a dedicated function
        {
            int w, h;
            framebuffer_size(w, h);

            // Note: it also possible to use a scaled framebuffer size (to render a larger image).
            //       This requires setting scaled viewport size (by calling glViewport() for each 
            //       view in MultiViewer::draw().
            FramebufferObject fbo(w, h, samples_);
            fbo.add_color_buffer();
            fbo.add_depth_buffer();

            fbo.bind();

            if (true) // white background
                glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            else
                glClearColor(background_color_[0], background_color_[1], background_color_[2], background_color_[3]);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

            const_cast<MultiViewer*>(this)->draw();
            
            fbo.release();

            // color render buffer
            return fbo.snapshot_color(0, file_name);
        }
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


    void MultiViewer::set_division_visible(bool b) {
        if (drawable_division_)
            drawable_division_->set_visible(b);
    }


    bool MultiViewer::division_visible() const {
        if (drawable_division_)
            return drawable_division_->is_visible();
        return false;
    }


    void MultiViewer::init() {
        Viewer::init();

        // compute the division
        update_division();
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

                // draw the transparent clipping plane
                ClippingPlane::instance()->draw(camera());
            }
        }

        // ------------------------------------------------------------

        // restore viewport.
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);

        // draw the division of views
        if (drawable_division_->is_visible())
            drawable_division_->draw(camera());
    }


    void MultiViewer::post_resize(int w, int h) {
        update_division();
    }


    void MultiViewer::update_division() {
        if (views_.empty() || views_[0].empty())
            return;

        if (!drawable_division_)
            drawable_division_ = std::unique_ptr<LinesDrawable2D>(new LinesDrawable2D("division"));

        view_width_ = static_cast<int>(static_cast<float>(width()) / static_cast<float>(num_cols_));
        view_height_ = static_cast<int>(static_cast<float>(height()) / static_cast<float>(num_rows_));
        // This is required to ensure a correct aspect ratio (thus the correct projection matrix)
        camera()->setScreenWidthAndHeight(view_width_, view_height_);

        for (int i = 0; i < num_rows_; ++i) {
            auto &row = views_[i];
            const auto y = height() - (i + 1) * view_height_;
            for (int j = 0; j < num_cols_; ++j) {
                const auto x = j * view_width_;
                row[j].viewport = ivec4(
                        static_cast<int>(static_cast<float>(x) * dpi_scaling()),
                        static_cast<int>(static_cast<float>(y) * dpi_scaling()),
                        static_cast<int>(static_cast<float>(view_width_) * dpi_scaling()),
                        static_cast<int>(static_cast<float>(view_height_) * dpi_scaling())
                );
            }
        }

        // ------------------------------------------------------------

        // Update the vertex buffer for the division drawable.
        // Each pair of consecutive points (screen coordinates) represent a line.
        std::vector<vec2> points;
        for (std::size_t i = 1; i < num_rows_; ++i) {
            const float y = i * view_height_;
            points.emplace_back(vec2(0, y));
            points.emplace_back(vec2(width(), y));
        }
        for (std::size_t i = 1; i < num_cols_; ++i) {
            const float x = i * view_width_;
            points.emplace_back(vec2(x, 0));
            points.emplace_back(vec2(x, height()));
        }
        drawable_division_->update_vertex_buffer(points, width(), height(), true);
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