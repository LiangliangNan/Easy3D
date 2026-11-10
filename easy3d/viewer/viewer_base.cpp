// SPDX-License-Identifier: GPL-3.0-or-later
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
#include <easy3d/viewer/viewer_base.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/transform.h>
#include <easy3d/util/logging.h>
#include <easy3d/viewer/backends/glfw_runtime.h>
#include <cmath>
#include <algorithm>
#include <limits>
namespace easy3d {
    ViewerBase::~ViewerBase() = default;

    void ViewerBase::poll_events() {
        detail::GlfwRuntime::poll_events();
    }

    // Coordinate contract, identical on the OpenGL and Vulkan backends:
    //  * World space: the scene's 3D coordinates. The camera's model-view-projection
    //    matrix maps world -> clip; these helpers hide that matrix from callers.
    //  * Window (viewer) logical coordinates: top-left origin, x right, y down, in
    //    logical point units spanning [0,width) x [0,height) from viewer_size().
    //    Mouse events and world_to_window()/window_to_world() use these, so client
    //    code is independent of the display's pixel density.
    //  * Framebuffer pixels: physical device pixels spanning [0,fw) x [0,fh) from
    //    framebuffer_size(). On a HiDPI display fw = width * dpi_scaling(). The
    //    world_to_pixel()/pixel_to_world() pair and the internal depth read in
    //    point_under_pixel() work in these units.
    // Depth is always normalized to [0,1] (near=0, far=1) in the public APIs, matching
    // Vulkan; OpenGL's native [-1,1] clip depth is remapped here so both agree.
    vec3 ViewerBase::world_to_window(const vec3 &world) const {
        int viewport[] = {0, 0, 0, 0};
        viewer_size(viewport[2], viewport[3]);
        return transform::project(world, camera()->modelViewProjectionMatrix(), viewport, false);
    }
    // Inverse of world_to_window(): window logical coordinates (top-left origin,
    // depth in [0,1]) -> world space. See the coordinate contract above.
    vec3 ViewerBase::window_to_world(const vec3 &window) const {
        int viewport[] = {0, 0, 0, 0};
        viewer_size(viewport[2], viewport[3]);
        return transform::unproject(window, camera()->modelViewProjectionMatrix(), viewport, false);
    }
    // Logical window coordinates scaled into framebuffer pixels. The depth component
    // is unchanged. See the coordinate contract above.
    vec3 ViewerBase::world_to_pixel(const vec3 &world) const {
        const vec3 window = world_to_window(world);
        int w, h, fw, fh;
        viewer_size(w, h);
        framebuffer_size(fw, fh);
        return vec3(float(transform::scale_coordinate(window.x, w, fw)),
                    float(transform::scale_coordinate(window.y, h, fh)), window.z);
    }
    // Framebuffer pixel center (top-left origin, depth in [0,1]) -> world space.
    // Converts the pixel center to logical window coordinates and unprojects. See
    // the coordinate contract above.
    vec3 ViewerBase::pixel_to_world(int x, int y, float depth) const {
        int w, h, fw, fh;
        viewer_size(w, h);
        framebuffer_size(fw, fh);
        if (w <= 0 || h <= 0 || fw <= 0 || fh <= 0 || x < 0 || y < 0 || x >= fw || y >= fh ||
            !std::isfinite(depth) || depth < 0 || depth > 1)
            throw std::invalid_argument("invalid framebuffer pixel/depth for unprojection");
        return window_to_world(vec3(float(transform::scale_coordinate(x + .5, fw, w)),
                                    float(transform::scale_coordinate(y + .5, fh, h)), depth));
    }
    // Fresh visible-surface query at a logical window pixel. Maps the logical
    // coordinate to its containing framebuffer pixel (DPI-aware), reads the scene
    // depth there, and unprojects that pixel's center back to world space. See the
    // coordinate contract above; HUD/overlays are excluded by read_scene_depth().
    bool ViewerBase::window_pixel(int x, int y, int &px, int &py) const {
        int w, h, fw, fh;
        viewer_size(w, h);
        framebuffer_size(fw, fh);
        if (w <= 0 || h <= 0 || fw <= 0 || fh <= 0 || x < 0 || y < 0 || x >= w || y >= h)
            return false;
        px = transform::pixel_index(x, w, fw);
        py = transform::pixel_index(y, h, fh);
        return true;
    }

    vec3 ViewerBase::point_under_pixel(int x, int y, bool &found) const {
        found = false;
        int px, py;
        if (!window_pixel(x, y, px, py))
            return vec3(0);
        float depth = 1;
        // The live camera carries logical screen dimensions on both backends; the
        // depth read renders into a framebuffer-sized target (see read_scene_depth).
        if (!read_scene_depth(px, py, *camera(), depth) || !std::isfinite(depth) || depth < 0 || depth >= 1)
            return vec3(0);
        const auto world = pixel_to_world(px, py, depth);
        found = true;
        return world;
    }

    void ViewerBase::set_callbacks(ViewerCallbacks callbacks) {
        auto replacement = std::make_shared<ViewerCallbacks>(std::move(callbacks));
        viewer_size(resize_width_, resize_height_);
        framebuffer_size(resize_fb_width_, resize_fb_height_);
        callbacks_ = std::move(replacement);
    }
    void ViewerBase::notify_pre_draw() {
        const auto callbacks = callbacks_;
        if (callbacks && callbacks->pre_draw)
            callbacks->pre_draw(this);
    }
    void ViewerBase::notify_post_draw() {
        const auto callbacks = callbacks_;
        if (callbacks && callbacks->post_draw)
            callbacks->post_draw(this);
    }
    void ViewerBase::notify_resize() {
        int w, h, fw, fh;
        viewer_size(w, h);
        framebuffer_size(fw, fh);
        if (w == resize_width_ && h == resize_height_ && fw == resize_fb_width_ && fh == resize_fb_height_)
            return;
        resize_width_ = w;
        resize_height_ = h;
        resize_fb_width_ = fw;
        resize_fb_height_ = fh;
        const auto callbacks = callbacks_;
        if (callbacks && callbacks->post_resize)
            callbacks->post_resize(this, w, h);
    }
    bool ViewerBase::dispatch_mouse_press(int x, int y, int button, int modifiers) {
        const auto callbacks = callbacks_;
        const bool handled =
            callbacks && callbacks->mouse_press_event && callbacks->mouse_press_event(this, x, y, button, modifiers);
        callback_button_ = button;
        callback_capture_ = handled;
        return handled;
    }
    bool ViewerBase::dispatch_mouse_release(int x, int y, int button, int modifiers) {
        const bool captured = callback_capture_ && callback_button_ == button;
        if (callback_button_ == button) {
            callback_button_ = -1;
            callback_capture_ = false;
        }
        const auto callbacks = callbacks_;
        const bool handled = callbacks && callbacks->mouse_release_event &&
                             callbacks->mouse_release_event(this, x, y, button, modifiers);
        return handled || captured;
    }
    bool ViewerBase::dispatch_mouse_drag(int x, int y, int dx, int dy, int button, int modifiers) {
        const auto callbacks = callbacks_;
        const bool handled = callbacks && callbacks->mouse_drag_event &&
                             callbacks->mouse_drag_event(this, x, y, dx, dy, button, modifiers);
        return handled || (callback_capture_ && callback_button_ == button);
    }
    bool ViewerBase::dispatch_mouse_move(int x, int y, int dx, int dy, int modifiers) {
        const auto callbacks = callbacks_;
        return callbacks && callbacks->mouse_free_move_event &&
               callbacks->mouse_free_move_event(this, x, y, dx, dy, modifiers);
    }
    bool ViewerBase::dispatch_mouse_scroll(int x, int y, int dx, int dy) {
        const auto callbacks = callbacks_;
        return callbacks && callbacks->mouse_scroll_event && callbacks->mouse_scroll_event(this, x, y, dx, dy);
    }
    bool ViewerBase::dispatch_key_press(int key, int modifiers) {
        const auto callbacks = callbacks_;
        return callbacks && callbacks->key_press_event && callbacks->key_press_event(this, key, modifiers);
    }
    bool ViewerBase::dispatch_key_release(int key, int modifiers) {
        const auto callbacks = callbacks_;
        return callbacks && callbacks->key_release_event && callbacks->key_release_event(this, key, modifiers);
    }
    bool ViewerBase::dispatch_character(unsigned int code) {
        const auto callbacks = callbacks_;
        return callbacks && callbacks->char_input_event && callbacks->char_input_event(this, code);
    }
    bool ViewerBase::dispatch_drop(const std::vector<std::string> &files) {
        const auto callbacks = callbacks_;
        return callbacks && callbacks->drop_event && callbacks->drop_event(this, files);
    }
    bool ViewerBase::dispatch_focus(bool focused) {
        if (!focused) {
            callback_button_ = -1;
            callback_capture_ = false;
        }
        const auto callbacks = callbacks_;
        return callbacks && callbacks->focus_event && callbacks->focus_event(this, focused);
    }

} // namespace easy3d
