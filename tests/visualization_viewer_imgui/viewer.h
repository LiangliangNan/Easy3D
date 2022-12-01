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

#ifndef EASY3D_TESTS_IMGUI_VIEWER_H
#define EASY3D_TESTS_IMGUI_VIEWER_H


#include <easy3d/viewer/viewer.h>



// A very good tutorial for imgui:
// https://eliasdaler.github.io/using-imgui-with-sfml-pt1/
// https://eliasdaler.github.io/using-imgui-with-sfml-pt2/


struct ImGuiContext;


namespace easy3d {

    class ViewerImGui : public Viewer
	{
	public:
        explicit ViewerImGui(
            const std::string& title = "Easy3D ImGui Viewer",
			int samples = 4,
			int gl_major = 3,
			int gl_minor = 2,
			bool full_screen = false,
			bool resizable = true,
			int depth_bits = 24,
			int stencil_bits = 8
		);

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

        // To provide real-time feedback to the user, e.g., current state of the
        // model and viewer, etc. This is implemented as a simple static window
        // with no decoration + a context-menu to choose its position.
        void  draw_overlay(bool* visible);

	protected:
		// Single global context by default, but can be overridden by the user
		static ImGuiContext *	context_;

        // Global variables for all the windows
        float	alpha_;
        bool	movable_;
		
		float   menu_height_;
	};

}

#endif	// EASY3D_TESTS_IMGUI_VIEWER_H
