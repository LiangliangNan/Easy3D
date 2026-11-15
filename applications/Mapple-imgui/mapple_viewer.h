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

#ifndef EASY3D_MAPPLE_VIEWER_H
#define EASY3D_MAPPLE_VIEWER_H

#include <cstddef>
#include <deque>
#include <memory>
#include <mutex>
#include <string>

#include <easy3d/viewer/viewer.h>
#include <easy3d/util/logging.h>

class ImGuiLayer;
namespace easy3d { class Model; }


/**
 * \brief The Mapple shell: a Viewer with a menu bar, toolbar, docked-style panels and a log sink.
 * \details The class owns the ImGui context (through ImGuiLayer), routes window input through
 *      ImGui first, lays out the panels around the central 3D scene, and collects Easy3D log
 *      messages into the Log panel. See docs/mapple.md for the port plan.
 */
class MappleViewer : public easy3d::Viewer, public easy3d::logging::Logger {
public:
    explicit MappleViewer(
            const std::string &title = "Mapple",
            int width = 1440,
            int height = 900,
            int samples = 4,
            bool full_screen = false,
            bool visible = true,
            bool resizable = true,
#ifdef EASY3D_HAS_VULKAN
            const easy3d::VulkanOptions &options = {}
#else
            const easy3d::OpenGLOptions &options = {}
#endif
    );

    ~MappleViewer() override;

    /// Receives every Easy3D log message; shown in the Log panel.
    void send(el::Level level, const std::string &msg) override;

protected:
    void init() override;

    void pre_draw() override;

    void post_resize(int w, int h) override;

    // Input hooks: return true while ImGui owns the input, so the gesture does not
    // also navigate the 3D scene.
    bool mouse_press_event(int x, int y, int button, int modifiers) override;

    bool mouse_release_event(int x, int y, int button, int modifiers) override;

    bool mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) override;

    bool mouse_free_move_event(int x, int y, int dx, int dy, int modifiers) override;

    bool mouse_scroll_event(int x, int y, int dx, int dy) override;

    bool key_press_event(int key, int modifiers) override;

    bool key_release_event(int key, int modifiers) override;

    bool char_input_event(unsigned int codepoint) override;

private:
    struct LogLine {
        el::Level level;
        std::string text;
    };

    // Framebuffer-to-window ratio; may differ from dpi_scaling().
    float pixel_ratio() const;

    void draw_gui();

    // ui/menu.cpp
    void draw_menu_bar();

    // ui/panels.cpp
    void draw_toolbar(float x, float y, float w, float h);

    void draw_panels(float x, float y, float w, float h);

    void draw_rendering_panel(float x, float y, float w, float h);

    void draw_models_panel(float x, float y, float w, float h);

    void draw_log_panel(float x, float y, float w, float h);

    void draw_status_bar(float x, float y, float w, float h);

    void draw_about_window();

    // A 4 px drag handle between the scene and a panel. dragging along the axis
    // adjusts *value by sign * mouse_delta.
    void draw_splitter(const char *id, float x, float y, float w, float h,
                       bool resize_width, float *value, float sign,
                       float min_value, float max_value);

    void delete_model_confirm(easy3d::Model *model);

private:
    std::unique_ptr<ImGuiLayer> imgui_;

    float menu_height_ = 0.0f; // height of the main menu bar this frame

    bool show_rendering_panel_ = true;
    bool show_models_panel_ = true;
    bool show_log_panel_ = true;
    bool show_about_ = false;

    float left_width_ = 250.0f;
    float right_width_ = 300.0f;
    float log_height_ = 170.0f;

    std::mutex log_mutex_;
    std::deque<LogLine> log_;
    std::size_t log_generation_ = 0;     // incremented on every appended line
    std::size_t last_seen_generation_ = 0; // UI-thread view, for auto-scrolling
    bool log_auto_scroll_ = true;
    static constexpr std::size_t max_log_lines_ = 2000;

    easy3d::Model *pending_delete_ = nullptr; // deferred so the Models panel never deletes mid-iteration
    bool mouse_captured_ = false;             // ImGui owned the press; keep the whole gesture away from the scene
};


#endif // EASY3D_MAPPLE_VIEWER_H
