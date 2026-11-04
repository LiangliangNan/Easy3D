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

#ifndef EASY3D_TUTORIAL_VIEWER_IMGUI_H
#define EASY3D_TUTORIAL_VIEWER_IMGUI_H



#include <easy3d/viewer/viewer.h>



// A very good tutorial for imgui:
// https://eliasdaler.github.io/using-imgui-with-sfml-pt1/
// https://eliasdaler.github.io/using-imgui-with-sfml-pt2/


struct ImGuiContext;


namespace easy3d {

    class ViewerImGui : public Viewer
	{
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
            const std::string& title = "Easy3D ViewerImGui",
            int width = 800,
            int height = 600,
            int samples = 1,
            bool full_screen = false,
            bool visible = true,
            bool resizable = true,
#ifdef EASY3D_HAS_VULKAN
            const VulkanOptions& options = {}
#else
            const OpenGLOptions& options = {}
#endif
        );

        /**
         * @brief The destructor of the Viewer.
         */
        ~ViewerImGui() override;

	protected:

		// imgui plugins
		void init() override;

		// draw the widgets
		void pre_draw() override;

		//  the widgets
#ifndef EASY3D_HAS_VULKAN
		void post_draw() override;
#endif

		void post_resize(int w, int h) override;

#ifndef EASY3D_HAS_VULKAN
		bool callback_event_cursor_pos(double x, double y) override;
		bool callback_event_mouse_button(int button, int action, int modifiers) override;
		bool callback_event_keyboard(int key, int action, int modifiers) override;
		bool callback_event_character(unsigned int codepoint) override;
		bool callback_event_scroll(double dx, double dy) override;

#endif

        // Override to add widgets, then call ViewerImGui::draw_gui() to finish the frame.
        virtual void draw_gui();
        void draw_menu_file();
        void draw_menu_view();

	protected:
        // Ratio between the framebuffer size and the window size.
        // May be different from the DPI scaling!
		float pixel_ratio();

		float widget_scaling() { return dpi_scaling() / pixel_ratio(); }

		// Rebuild the font atlas for this viewer.
		void  reload_font(int font_size = 16);

	protected:
		// Each viewer owns its UI context.
        ImGuiContext* context_ = nullptr;

		float menu_height_ = 0;
#ifdef EASY3D_HAS_VULKAN
        std::shared_ptr<const graphics::Image> font_image_;
#endif
	};

}

#endif	// EASY3D_TUTORIAL_VIEWER_IMGUI_H
