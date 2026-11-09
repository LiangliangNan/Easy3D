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

#ifndef EASY3D_TUTORIAL_VIEWER_IMGUI_H
#define EASY3D_TUTORIAL_VIEWER_IMGUI_H

#include <easy3d/viewer/viewer.h>

// A very good tutorial for imgui:
// https://eliasdaler.github.io/using-imgui-with-sfml-pt1/
// https://eliasdaler.github.io/using-imgui-with-sfml-pt2/

struct ImGuiContext;

namespace easy3d {

    /**
     * \brief A Viewer subclass that adds a Dear ImGui menu bar and widgets on top of the scene.
     *
     * This is the standard pattern for putting an immediate-mode GUI into Easy3D: derive from
     * Viewer, own an ImGuiContext, and hook the frame at init() (create the context/backend),
     * pre_draw() (start the ImGui frame, emit widgets, convert them to overlays), and
     * post_resize() (keep the display size in sync). Input callbacks forward mouse/keyboard
     * events to ImGui first so that GUI clicks do not also navigate the 3D scene. Subclass
     * draw_gui() to add your own windows and controls.
     */
    class ViewerImGui : public Viewer {
    public:
        /**
         * @brief Constructor
         * @param title The window title of the viewer, which can be changed by calling set_title() after construction.
         * @param width The initial window width.
         * @param height The initial window height.
         * @param samples The number of samples for multisample antialiasing.
         * @param full_screen Whether to use a fullscreen window.
         * @param visible Whether the window is initially visible.
         * @param resizable Whether the viewer can be resized by the user.
         * @param options Backend device/context settings.
         */
        explicit ViewerImGui(
            const std::string &title = "Easy3D ViewerImGui",
            int width = 800,
            int height = 600,
            int samples = 1,
            bool full_screen = false,
            bool visible = true,
            bool resizable = true,
#ifdef EASY3D_HAS_VULKAN
            const VulkanOptions &options = {}
#else
            const OpenGLOptions &options = {}
#endif
        );

        /**
         * @brief The destructor of the Viewer.
         */
        ~ViewerImGui() override;

    protected:
        // Creates the ImGui context, GLFW backend, input callbacks, and font atlas.
        void init() override;

        // Starts an ImGui frame, draws the widgets, and converts them to viewer overlays.
        void pre_draw() override;

        // Keeps ImGui's display size in sync when the window is resized.
        void post_resize(int w, int h) override;

        // Override to add widgets, then call ViewerImGui::draw_gui() to finish the frame.
        virtual void draw_gui();
        // The individual top-level menus emitted by draw_gui().
        void draw_menu_file();
        void draw_menu_view();

    protected:
        // Ratio between the framebuffer size and the window size.
        // May be different from the DPI scaling!
        float pixel_ratio();

        // Scale factor applied to widgets so they stay crisp on high-DPI displays.
        float widget_scaling() {
            return dpi_scaling() / pixel_ratio();
        }

        // Rebuild the font atlas for this viewer.
        void reload_font(int font_size = 16);

    protected:
        // Each viewer owns its UI context.
        ImGuiContext *context_ = nullptr;

        float menu_height_ = 0; //!< Height of the menu bar, in pixels (used to lay out the scene).
        std::shared_ptr<const graphics::Image> font_image_; //!< GPU texture holding the ImGui font atlas.
    };

} // namespace easy3d

#endif // EASY3D_TUTORIAL_VIEWER_IMGUI_H
