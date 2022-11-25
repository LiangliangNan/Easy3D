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
         * @param samples The number of samples for multisample antialiasing
         * @param gl_major/gl_minor The OpenGL version to request. The created context will be
         *        compatible with the requested version (if the context creation succeeded).
         * @param full_screen Do you want the viewer to be fullscreen?
         * @param resizable Whether the viewer will be resizable by the user.
         * @param depth_bits The desired bit depths of the depth component of the default framebuffer.
         * @param stencil_bits The desired bit depths of the stencil component of the default framebuffer.
         * @param width The width of the viewer, which can be changed by calling resize() after construction.
         * @param height The height of the viewer, which can be changed by calling resize() after construction.
         */
        explicit ViewerImGui(
            const std::string& title = "Easy3D ViewerImGui",
			int samples = 4,
            int gl_major = 3,   // must >= 3
            int gl_minor = 2,   // must >= 2
			bool full_screen = false,
			bool resizable = true,
			int depth_bits = 24,
			int stencil_bits = 8,
            int width = 800,
            int height = 600
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
		void post_draw() override;

		void post_resize(int w, int h) override;

		bool callback_event_cursor_pos(double x, double y) override;
		bool callback_event_mouse_button(int button, int action, int modifiers) override;
		bool callback_event_keyboard(int key, int action, int modifiers) override;
		bool callback_event_character(unsigned int codepoint) override;
		bool callback_event_scroll(double dx, double dy) override;

        void draw_menu_file();
        void draw_menu_view();

	protected:
        // Ratio between the framebuffer size and the window size.
        // May be different from the DPI scaling!
		float pixel_ratio();

		float widget_scaling() { return dpi_scaling() / pixel_ratio(); }

		// We don't need a per-window font. So this function is static
		void  reload_font(int font_size = 16);

	protected:
		// Single global context by default, but can be overridden by the user
		static ImGuiContext *	context_;

		float menu_height_;
	};

}

#endif	// EASY3D_TUTORIAL_VIEWER_IMGUI_H
