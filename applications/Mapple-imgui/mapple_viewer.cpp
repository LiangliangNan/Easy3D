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

#include "mapple_viewer.h"

#include "imgui_layer.h"

#include <algorithm>
#include <stdexcept>

#include <easy3d/util/file_system.h>

#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>

#include <3rd_party/imgui/imgui.h>

using namespace easy3d;

MappleViewer::MappleViewer(const std::string &title,
                           int width,
                           int height,
                           int samples,
                           bool full_screen,
                           bool visible,
                           bool resizable,
#ifdef EASY3D_HAS_VULKAN
                           const VulkanOptions &options
#else
                           const OpenGLOptions &options
#endif
        )
        : Viewer(title, width, height, samples, full_screen, visible, resizable, options) {
    // The vendored GLFW is a private dependency of easy3d_viewer, so an app that also calls
    // GLFW (here through the ImGui backend) links its own static copy. On Windows that copy
    // must be initialized before it can operate on the viewer's window. See issue #178.
#if defined(_WIN32) && defined(_MSC_VER)
    if (!glfwInit())
        throw std::runtime_error("glfwInit failed for Mapple's GLFW copy");
#endif
}

MappleViewer::~MappleViewer() {
    // Destroy the ImGui context and its font texture while the viewer (and its graphics
    // device) is still alive.
    imgui_.reset();
}

void MappleViewer::send(el::Level level, const std::string &msg) {
    std::lock_guard<std::mutex> lock(log_mutex_);
    log_.push_back({level, msg});
    while (log_.size() > max_log_lines_)
        log_.pop_front();
    ++log_generation_;
}

float MappleViewer::pixel_ratio() const {
    const int w = width();
    return w > 0 ? static_cast<float>(framebuffer_width()) / static_cast<float>(w) : 1.0f;
}

void MappleViewer::init() {
    Viewer::init();

    // The UI replaces the viewer's screen-space HUD: panels cover the framebuffer's
    // corners, and the information is shown in the panels and the status bar.
    show_easy3d_logo_ = false;
    show_frame_rate_ = false;
    show_corner_axes_ = false;

    imgui_.reset(new ImGuiLayer);
    if (!imgui_->init(window(), dpi_scaling(), pixel_ratio()))
        throw std::runtime_error("failed to initialize Dear ImGui");
    imgui_->set_display_size(width(), height());

#ifdef EASY3D_HAS_VULKAN
    LOG(INFO) << "Mapple UI ready (Vulkan)";
#else
    LOG(INFO) << "Mapple UI ready (OpenGL)";
#endif
}

void MappleViewer::pre_draw() {
    imgui_->new_frame(dpi_scaling(), pixel_ratio());
    Viewer::pre_draw();
    draw_gui();
    set_overlays(imgui_->render(framebuffer_width(), framebuffer_height()), true);
}

void MappleViewer::post_resize(int w, int h) {
    Viewer::post_resize(w, h);
    if (imgui_)
        imgui_->set_display_size(w, h);
}

bool MappleViewer::mouse_press_event(int x, int y, int button, int modifiers) {
    mouse_captured_ = imgui_ && imgui_->wants_mouse();
    return mouse_captured_ || Viewer::mouse_press_event(x, y, button, modifiers);
}

bool MappleViewer::mouse_release_event(int x, int y, int button, int modifiers) {
    // The release belongs to whichever side received the press, even if the pointer
    // has moved over a panel since; otherwise the scene would miss its release.
    const bool captured = mouse_captured_;
    mouse_captured_ = false;
    return captured || Viewer::mouse_release_event(x, y, button, modifiers);
}

bool MappleViewer::mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) {
    if (mouse_captured_)
        return true;
    return Viewer::mouse_drag_event(x, y, dx, dy, button, modifiers);
}

bool MappleViewer::mouse_free_move_event(int x, int y, int dx, int dy, int modifiers) {
    return (imgui_ && imgui_->wants_mouse()) || Viewer::mouse_free_move_event(x, y, dx, dy, modifiers);
}

bool MappleViewer::mouse_scroll_event(int x, int y, int dx, int dy) {
    return (imgui_ && imgui_->wants_mouse()) || Viewer::mouse_scroll_event(x, y, dx, dy);
}

bool MappleViewer::key_press_event(int key, int modifiers) {
    return (imgui_ && imgui_->wants_keyboard()) || Viewer::key_press_event(key, modifiers);
}

bool MappleViewer::key_release_event(int key, int modifiers) {
    return (imgui_ && imgui_->wants_keyboard()) || Viewer::key_release_event(key, modifiers);
}

bool MappleViewer::char_input_event(unsigned int codepoint) {
    return (imgui_ && imgui_->wants_keyboard()) || Viewer::char_input_event(codepoint);
}

void MappleViewer::draw_gui() {
    draw_menu_bar();

    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    const float x = viewport->Pos.x;
    const float y = viewport->Pos.y + menu_height_;
    const float w = viewport->Size.x;
    const float h = std::max(0.0f, viewport->Size.y - menu_height_);

    const float toolbar_height = ImGui::GetFrameHeight() + 8.0f;
    const float status_height = ImGui::GetTextLineHeight() + 8.0f;

    draw_toolbar(x, y, w, toolbar_height);
    draw_status_bar(x, y + h - status_height, w, status_height);
    draw_panels(x, y + toolbar_height, w, std::max(0.0f, h - toolbar_height - status_height));

    draw_about_window();
}
