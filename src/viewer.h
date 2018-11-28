/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
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


#include "basic_viewer.h"



// A very good tutorial for imgui:
// https://eliasdaler.github.io/using-imgui-with-sfml-pt1/
// https://eliasdaler.github.io/using-imgui-with-sfml-pt2/


class Window;

class Viewer : public BasicViewer
{
public:
	Viewer(
		const std::string& title = "Easy3D Viewer",
		int samples = 4,
		int gl_major = 3,
		int gl_minor = 2,
		bool full_screen = false,
		bool resizable = true,
		int depth_bits = 24,
		int stencil_bits = 8
	);

	~Viewer();

	const std::vector<Window*>& windows() const { return windows_; }

protected:
	void cleanup();

	void draw_all();

	// imgui plugins
	void init();

    void post_resize(int w, int h);

	bool callback_event_cursor_pos(double x, double y);
	bool callback_event_mouse_button(int button, int action, int modifiers);
	bool callback_event_keyboard(int key, int action, int modifiers);
	bool callback_event_character(unsigned int codepoint);
	bool callback_event_scroll(double dx, double dy);

protected:

	// List of registered windows
	std::vector<Window*>	windows_;
	friend class Window;
};


#endif	// _EASY3D_VIEWER_H_
