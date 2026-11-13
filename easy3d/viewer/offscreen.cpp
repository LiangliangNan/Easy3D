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

#include <easy3d/viewer/offscreen.h>
#include <easy3d/renderer/camera.h>

#ifdef EASY3D_HAS_VULKAN
namespace easy3d {
    OffScreen::OffScreen(int w, int h, const VulkanOptions &options) : Viewer(w, h, options) {
        snapshot_hud_ = false;
    }
    OffScreen::~OffScreen() = default;
    void OffScreen::resize(int w, int h) {
        resize_render_target(w, h);
    }
    bool OffScreen::render(const std::string &file, float scale, int samples, int bg, bool expand) const {
        return Viewer::snapshot(file, scale, samples, bg, expand);
    }
    Model *OffScreen::add_model(const std::string &file, bool create, bool fit_scene) {
        return Viewer::add_model(file, create, fit_scene);
    }
    Model *OffScreen::add_model(std::shared_ptr<Model> model, bool create, bool fit_scene) {
        return Viewer::add_model(std::move(model), create, fit_scene);
    }
    Drawable *OffScreen::add_drawable(std::shared_ptr<Drawable> drawable) {
        return Viewer::add_drawable(std::move(drawable));
    }
} // namespace easy3d
#else

#include <easy3d/renderer/opengl.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <cmath>
#include <limits>
#include <stdexcept>
namespace easy3d {
    OffScreen::OffScreen(int w, int h, const OpenGLOptions &gl_options)
        : Viewer("Easy3D OffScreen Renderer",
                 w > 0 ? w : throw std::invalid_argument("offscreen width must be positive"),
                 h > 0 ? h : throw std::invalid_argument("offscreen height must be positive"),
                 4,
                 false,
                 false,
                 true,
                 gl_options) {
        snapshot_hud_ = false;
        manual_.clear();
        width_ = w;
        height_ = h;
        dpi_scaling_ = 1;
        init();
        resize(w, h);
    }
    OffScreen::~OffScreen() {
        activate();
    }
    void OffScreen::activate() const {
        glfwMakeContextCurrent(window_);
        glViewport(0, 0, width_, height_);
    }
    void OffScreen::resize(int w, int h) {
        if (w <= 0 || h <= 0)
            throw std::invalid_argument("offscreen dimensions must be positive");
        activate();
        Viewer::resize(w, h);
        width_ = w;
        height_ = h;
        dpi_scaling_ = 1;
        camera()->set_screen_width_and_height(w, h);
    }
    bool OffScreen::render(const std::string &file, float scale, int samples, int bg, bool expand) const {
        if (!std::isfinite(scale) || scale <= 0 || double(width()) * scale > std::numeric_limits<int>::max() ||
            double(height()) * scale > std::numeric_limits<int>::max() || samples <= 0 || samples > 64 ||
            (samples & (samples - 1)) || bg < 0 || bg > 2)
            return false;
        activate();
        return snapshot(file, scale, samples == 1 ? 0 : samples, bg, expand);
    }
    Model *OffScreen::add_model(const std::string &file, bool create, bool fit_scene) {
        return Viewer::add_model(file, create, fit_scene);
    }
    Model *OffScreen::add_model(std::shared_ptr<Model> model, bool create, bool fit_scene) {
        return Viewer::add_model(std::move(model), create, fit_scene);
    }
    Drawable *OffScreen::add_drawable(std::shared_ptr<Drawable> drawable) {
        activate();
        return Viewer::add_drawable(std::move(drawable));
    }
} // namespace easy3d

#endif // !EASY3D_HAS_VULKAN

namespace easy3d {
    void OffScreen::framebuffer_size(int &w, int &h) const {
        w = width_;
        h = height_;
    }
    void OffScreen::viewer_size(int &w, int &h) const {
        w = width_;
        h = height_;
    }
} // namespace easy3d
