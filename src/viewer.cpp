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


#include "viewer.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

#include "plugin.h"
#include "window.h"
#include "camera.h"

#include <cmath>
#include <iostream>


Viewer::Viewer(
	const std::string& title /* = "Easy3D Viewer" */,
	int samples /* = 4 */,
	int gl_major /* = 3 */,
	int gl_minor /* = 2 */,
	bool full_screen /* = false */,
	bool resizable /* = true */,
	int depth_bits /* = 24 */,
	int stencil_bits /* = 8 */
) 
	: BasicViewer(title, samples, gl_major, gl_minor, full_screen, resizable, depth_bits, stencil_bits)
{
}


void Viewer::init() {
	for (auto p : windows_)
		p->init();
}


void Viewer::post_resize(int w, int h) {
    for (auto p : windows_)
        p->post_resize(w, h);
}


void Viewer::cleanup() {
	for (auto p : windows_)
		p->cleanup();

	if (!windows_.empty()) {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();

		ImGui::DestroyContext(Window::context_);
	}

	BasicViewer::cleanup();
}


bool Viewer::pre_draw() {
	if (!windows_.empty()) {
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	return BasicViewer::pre_draw();
}

bool Viewer::post_draw() {
	for (auto p : windows_) {
		if (p->draw())
			return true;
	}

	if (!windows_.empty()) {
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
	}

	return BasicViewer::post_draw();
}



bool Viewer::callback_event_cursor_pos(double x, double y) {
	int px = static_cast<int>(x);
	int py = static_cast<int>(y);

	try {
		for (auto p : windows_)
			if (p->mouse_move(px, py))
				return true;

		int dx = px - mouse_x_;
		int dy = py - mouse_y_;
		mouse_x_ = px;
		mouse_y_ = py;

		if (drag_active_)
			return mouse_drag_event(px, py, dx, dy, button_, modifiers_);
		else
			return mouse_free_move_event(px, py, dx, dy, modifiers_);
	}
	catch (const std::exception &e) {
		std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
		return false;
	}
}


bool Viewer::callback_event_mouse_button(int button, int action, int modifiers) {
	try {
		if (action == GLFW_PRESS) {
			for (auto p : windows_) {
				if (p->mouse_press(button, modifiers))
					return true;
			}

			drag_active_ = true;
			button_ = button;
			modifiers_ = modifiers;
			return mouse_press_event(mouse_x_, mouse_y_, button, modifiers);
		}
		else {
			for (auto p : windows_) {
				if (p->mouse_release(button, modifiers))
					return true;
			}

			drag_active_ = false;
			return mouse_release_event(mouse_x_, mouse_y_, button, modifiers);
		}
	}
	catch (const std::exception &e) {
		std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
		return false;
	}
}


bool Viewer::callback_event_keyboard(int key, int action, int modifiers) {
	try {
		if (action == GLFW_PRESS) {
			for (auto p : windows_) {
				if (p->key_press(key, modifiers))
					return true;
			}
			return key_press_event(key, modifiers);
		}
		else {
			for (auto p : windows_) {
				if (p->key_release(key, modifiers))
					return true;
			}
			return key_release_event(key, modifiers);
		}
	}
	catch (const std::exception &e) {
		std::cerr << "Caught exception in event handler: " << e.what() << std::endl;
		return false;
	}
}


bool Viewer::callback_event_character(unsigned int codepoint) {
	try {
		for (auto p : windows_) {
			if (p->char_input(codepoint))
				return true;
		}
		return char_input_event(codepoint);
	}
	catch (const std::exception &e) {
		std::cerr << "Caught exception in event handler: " << e.what()
			<< std::endl;
		return false;
	}
}


bool Viewer::callback_event_scroll(double dx, double dy) {
	try {
		for (auto p : windows_) {
			if (p->mouse_scroll(dy))
				return true;
		}

		return mouse_scroll_event(mouse_x_, mouse_y_, static_cast<int>(dx), static_cast<int>(dy));
	}
	catch (const std::exception &e) {
		std::cerr << "Caught exception in event handler: " << e.what()
			<< std::endl;
		return false;
	}
}

