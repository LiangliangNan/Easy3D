/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of EasyGUI: software for processing and rendering
*   meshes and point clouds.
*
*	EasyGUI is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	EasyGUI is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _EASY3D_VIEWER_H_
#define _EASY3D_VIEWER_H_


#include <easy3d/core/basic_viewer.h>



// A very good tutorial for imgui:
// https://eliasdaler.github.io/using-imgui-with-sfml-pt1/
// https://eliasdaler.github.io/using-imgui-with-sfml-pt2/



struct ImGuiContext;


namespace easy3d {

	class Panel;

	class MainWindow : public BasicViewer
	{
	public:
		MainWindow(
			const std::string& title = "easy3d MainWindow",
			int samples = 4,
			int gl_major = 3,
			int gl_minor = 2,
			bool full_screen = false,
			bool resizable = true,
			int depth_bits = 24,
			int stencil_bits = 8
		);

	protected:

		// imgui plugins
		void init() override;

		// draw the widgets
		void pre_draw() override;

		//  the widgets
		void post_draw() override;

		void cleanup() override;

		void post_resize(int w, int h) override;

		bool callback_event_cursor_pos(double x, double y) override;
		bool callback_event_mouse_button(int button, int action, int modifiers) override;
		bool callback_event_keyboard(int key, int action, int modifiers) override;
		bool callback_event_character(unsigned int codepoint) override;
		bool callback_event_scroll(double dx, double dy) override;

	protected:
		// Ratio between the framebuffer size and the window size.
		// May be different from the hipdi scaling!
		static float pixel_ratio();

		// Hidpi scaling to be used for text rendering.
		static float hidpi_scaling();

		static float widget_scaling() { return hidpi_scaling() / pixel_ratio(); }

		// We don't need a per-window font. So this function is static
		void  reload_font(int font_size = 16);

	protected:
		// Single global context by default, but can be overridden by the user
		static ImGuiContext *	context_;

		// List of registered windows
		std::vector<Panel*>	windows_;

		// Global variables for all the windows
		float	alpha_;
		bool	movable_;
		bool    visible_;
		
		float   menu_height_;

		friend class Panel;
	};

}

#endif	// _EASY3D_VIEWER_H_
