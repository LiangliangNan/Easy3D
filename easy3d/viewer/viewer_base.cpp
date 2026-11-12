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
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <cmath>
#include <algorithm>
#include <limits>
namespace easy3d {
    ViewerBase::~ViewerBase() = default;

    //------------------------------------------------------------------------
    // Legacy event hooks: no-ops unless an interactive host overrides them.
    //------------------------------------------------------------------------
    bool ViewerBase::mouse_press_event(int, int, int, int) { return false; }
    bool ViewerBase::mouse_release_event(int, int, int, int) { return false; }
    bool ViewerBase::mouse_drag_event(int, int, int, int, int, int) { return false; }
    bool ViewerBase::mouse_free_move_event(int, int, int, int, int) { return false; }
    bool ViewerBase::mouse_scroll_event(int, int, int, int) { return false; }
    bool ViewerBase::char_input_event(unsigned int) { return false; }
    bool ViewerBase::key_press_event(int, int) { return false; }
    bool ViewerBase::key_release_event(int, int) { return false; }
    bool ViewerBase::drop_event(const std::vector<std::string> &) { return false; }
    bool ViewerBase::focus_event(bool) { return false; }

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
        return transform::project(world, camera()->model_view_projection_matrix(), viewport, false);
    }
    // Inverse of world_to_window(): window logical coordinates (top-left origin,
    // depth in [0,1]) -> world space. See the coordinate contract above.
    vec3 ViewerBase::window_to_world(const vec3 &window) const {
        int viewport[] = {0, 0, 0, 0};
        viewer_size(viewport[2], viewport[3]);
        return transform::unproject(window, camera()->model_view_projection_matrix(), viewport, false);
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

    void ViewerBase::InputState::reset() {
        button = -1;
        press_modifiers = 0;
        key = -1;
        scroll_x = scroll_y = 0;
    }

    //------------------------------------------------------------------------
    // Event routing: shared by the OpenGL and Vulkan GLFW trampolines.
    //------------------------------------------------------------------------
    void ViewerBase::handle_cursor_pos(int x, int y, int modifiers) {
        const int dx = x - input_.x;
        const int dy = y - input_.y;
        input_.x = x;
        input_.y = y;
        if (input_.button < 0) {
            mouse_free_move_event(x, y, dx, dy, modifiers);
            return;
        }
        // Drags keep the modifiers latched at press on both backends, so pressing a
        // modifier mid-gesture cannot switch the interaction mode.
        mouse_drag_event(x, y, dx, dy, input_.button, input_.press_modifiers);
        // Outside positions still reach the hook, so subclasses can clear hover
        // highlights there; the cursor is then pinned to the edge so an in-flight
        // drag cannot run away.
        const int w = width(), h = height();
        if (window() && (x < 0 || x > w || y < 0 || y > h))
            glfwSetCursorPos(window(), std::max(0.0, std::min(double(w), double(x))),
                             std::max(0.0, std::min(double(h), double(y))));
    }

    void ViewerBase::handle_mouse_button(int x, int y, int button, int action, int modifiers) {
        input_.x = x;
        input_.y = y;
        if (action == GLFW_PRESS) {
            input_.button = button;
            input_.press_x = x;
            input_.press_y = y;
            input_.press_modifiers = modifiers;
            mouse_press_event(x, y, button, modifiers);
        } else if (action == GLFW_RELEASE) {
            const bool held = input_.button == button;
            const bool consumed = mouse_release_event(x, y, button, modifiers);
            if (held) {
                input_.button = -1;
                input_.press_modifiers = 0;
                // A subclass that consumed the release skipped the built-in cleanup.
                if (consumed)
                    end_gesture();
            }
        }
    }

    void ViewerBase::handle_scroll(int x, int y, double dx, double dy) {
        input_.x = x;
        input_.y = y;
        input_.scroll_x += dx;
        input_.scroll_y += dy;
        const int sx = int(input_.scroll_x), sy = int(input_.scroll_y);
        input_.scroll_x -= sx;
        input_.scroll_y -= sy;
        if (!sx && !sy)
            return;
        mouse_scroll_event(x, y, sx, sy);
    }

    void ViewerBase::handle_key(int key, int action, int modifiers) {
        if (action == GLFW_PRESS || action == GLFW_REPEAT) {
            key_press_event(key, modifiers);
        } else {
            input_.key = -1;
            key_release_event(key, modifiers);
        }
    }

    void ViewerBase::handle_character(unsigned int codepoint) {
        char_input_event(codepoint);
    }

    void ViewerBase::handle_drop(const std::vector<std::string> &files) {
        drop_event(files);
    }

    void ViewerBase::handle_focus(bool focused) {
        if (!focused) {
            input_.reset();
            cancel_interaction();
        }
        focus_event(focused);
    }

} // namespace easy3d
