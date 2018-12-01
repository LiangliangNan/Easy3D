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


#include <easy3d/window/main_window.h>

#include <cmath>
#include <iostream>

#include <3rd_party/imgui/imgui.h>
#include <3rd_party/imgui/impl/imgui_impl_glfw.h>
#include <3rd_party/imgui/impl/imgui_impl_opengl3.h>
#include <3rd_party/GLFW/include/GLFW/glfw3.h>

#include <easy3d/window/plugin.h>
#include <easy3d/window/panel.h>
#include <easy3d/core/camera.h>

#include "imgui_fonts_droid_sans.h"


namespace easy3d {

	ImGuiContext* MainWindow::context_ = nullptr;

	MainWindow::MainWindow(
		const std::string& title /* = "easy3d MainWindow" */,
		int samples /* = 4 */,
		int gl_major /* = 3 */,
		int gl_minor /* = 2 */,
		bool full_screen /* = false */,
		bool resizable /* = true */,
		int depth_bits /* = 24 */,
		int stencil_bits /* = 8 */
	) 
		: Viewer(title, samples, gl_major, gl_minor, full_screen, resizable, depth_bits, stencil_bits)
		, movable_(true)
		, alpha_(0.8f)
	{
	}


	void MainWindow::init() {
        Viewer::init();

		if (!context_) {
			// Setup ImGui binding
			IMGUI_CHECKVERSION();

			context_ = ImGui::CreateContext();

			const char* glsl_version = "#version 150";
			ImGui_ImplGlfw_InitForOpenGL(window_, false);
			ImGui_ImplOpenGL3_Init(glsl_version);
			ImGuiIO& io = ImGui::GetIO();
			io.IniFilename = nullptr;
			ImGui::StyleColorsDark();
			ImGuiStyle& style = ImGui::GetStyle();
			style.FrameRounding = 5.0f;

			// load font
			reload_font();
		}
	}


	void MainWindow::reload_font(int font_size)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.Fonts->Clear();
		io.Fonts->AddFontFromMemoryCompressedTTF(droid_sans_compressed_data, droid_sans_compressed_size, font_size * hidpi_scaling());
		io.FontGlobalScale = 1.0f / pixel_ratio();
	}


	void MainWindow::post_resize(int w, int h) {
        Viewer::post_resize(w, h);
		if (context_) {
			ImGui::GetIO().DisplaySize.x = float(w);
			ImGui::GetIO().DisplaySize.y = float(h);
		}
	}


	float MainWindow::pixel_ratio()
	{
		// Computes pixel ratio for hidpi devices
		int buf_size[2];
		int win_size[2];
		GLFWwindow* window = glfwGetCurrentContext();
		glfwGetFramebufferSize(window, &buf_size[0], &buf_size[1]);
		glfwGetWindowSize(window, &win_size[0], &win_size[1]);
		return (float)buf_size[0] / (float)win_size[0];
	}


	float MainWindow::hidpi_scaling()
	{
		// Computes scaling factor for hidpi devices
		float xscale, yscale;
		GLFWwindow* window = glfwGetCurrentContext();
		glfwGetWindowContentScale(window, &xscale, &yscale);
		return 0.5f * (xscale + yscale);
	}


	bool MainWindow::callback_event_cursor_pos(double x, double y) {
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


	bool MainWindow::callback_event_mouse_button(int button, int action, int modifiers) {
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


	bool MainWindow::callback_event_keyboard(int key, int action, int modifiers) {
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


	bool MainWindow::callback_event_character(unsigned int codepoint) {
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


	bool MainWindow::callback_event_scroll(double dx, double dy) {
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


	void MainWindow::cleanup() {
		for (auto p : windows_)
			p->cleanup();

		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();

		ImGui::DestroyContext(context_);

        Viewer::cleanup();
	}


	void MainWindow::pre_draw() {
        ImGui_ImplOpenGL3_NewFrame();  
        ImGui_ImplGlfw_NewFrame();    
        ImGui::NewFrame();   

        Viewer::pre_draw(); 
	}


	void MainWindow::post_draw() {
		static bool show_about = false;
		if (show_about) {
			int w, h;
			glfwGetWindowSize(window_, &w, &h);
			ImGui::SetNextWindowPos(ImVec2(w * 0.5f, h * 0.5f), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
			ImGui::Begin("About Easy3D", &show_about, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
			ImGui::Text(
				"Easy3D is an easy, lightweight, and flexible framework for developing\n"
				"cross-platform 3D applications. It requires minimum dependencies, i.e.\n"
				"\t- GLFW (for cross-platform OpenGL context creation) and\n"
				"\t- ImGui (for GUI creation and event handling,\n"
				"\n"
				"Easy3D works on all major operating systems with a decent C++11 capable\n"
				"compiler, e.g., MacOS (Clang), Linux (GCC or Clang), and Windows (Visual\n"
				"Studio >= 2015). All dependencies are included and built using CMake.\n"
				"\n"
			);
			ImGui::Separator();
			ImGui::Text(
				"\n"
				"Liangliang Nan\n"
				"liangliang.nan@gmail.com\n"
				"https://3d.bk.tudelft.nl/liangliang/\n"
			);
			ImGui::End();
		}

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 8));
		if (ImGui::BeginMainMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open", "Ctrl+O")) { open(); }
				if (ImGui::MenuItem("Save As...", "Ctrl+S")) { save(); }

				ImGui::Separator();
				if (ImGui::BeginMenu("Recent Files...")) {
					ImGui::MenuItem("bunny.ply");
					ImGui::MenuItem("terain.las");
					ImGui::MenuItem("building.obj");
					ImGui::EndMenu();
				}

				ImGui::Separator();
				if (ImGui::MenuItem("Quit", "Alt+F4"))
					glfwSetWindowShouldClose(window_, GLFW_TRUE);

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("View"))
			{
				if (ImGui::MenuItem("Snapshot", NULL))
					std::cout << "snapshot" << std::endl;

				ImGui::Separator();
				if (ImGui::MenuItem("Save Camera State", NULL))
					std::cout << "save camera state" << std::endl;
				if (ImGui::MenuItem("Load Camera State", NULL))
					std::cout << "load camera state" << std::endl;

				ImGui::Separator();
				if (ImGui::BeginMenu("Options"))
				{
					const char* items[] = { "Classic", "Dark", "Light" };
					static const char* current_item = items[1];
					if (ImGui::BeginCombo("Window Style", NULL, ImGuiComboFlags_NoPreview))
					{
						for (int i = 0; i < IM_ARRAYSIZE(items); ++i)
						{
							if (ImGui::Selectable(items[i], items[i] == current_item)) {
								switch (i)
								{
								case 0: ImGui::StyleColorsClassic(); break;
								case 1: ImGui::StyleColorsDark(); break;
								case 2: ImGui::StyleColorsLight(); break;
								}
								current_item = items[i];
								ImGui::SetItemDefaultFocus();   // Set the focus
							}
						}
						ImGui::EndCombo();
					}
					
					ImGui::Checkbox("Panel Movable", &movable_);
					ImGui::ColorEdit3("Background Color", (float*)background_color_, ImGuiColorEditFlags_NoInputs);
					ImGui::PushItemWidth(100);
					ImGui::DragFloat("Transparency", &alpha_, 0.005f, 0.0f, 1.0f, "%.1f");
					ImGui::PopItemWidth();
					ImGui::EndMenu();
				}

				if (!windows_.empty()) {
					ImGui::Separator();
					for (std::size_t i = 0; i < windows_.size(); ++i) {
						Panel* win = windows_[i];
						ImGui::MenuItem(win->name_.c_str(), 0, &win->visible_);
					}
				}

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Select"))
			{
				const char* items[] = { "None", "Vertex", "Face", "Edge" };
				static const char* current_item = items[0];
				ImGui::PushItemWidth(120);
				if (ImGui::BeginCombo("##ComboSelect", current_item))
				{
					for (int i = 0; i < IM_ARRAYSIZE(items); ++i)
					{
						if (ImGui::Selectable(items[i], items[i] == current_item)) {
							current_item = items[i];
							ImGui::SetItemDefaultFocus();   // Set the focus
						}
					}
					ImGui::EndCombo();
				}
				ImGui::PopItemWidth();

				ImGui::Separator();
				ImGui::MenuItem("Invert", NULL, false);
				ImGui::MenuItem("Delete", NULL, false);
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Help"))
			{
				ImGui::MenuItem("MainWindow", NULL, false);
				ImGui::MenuItem("Shortcut", NULL, false);
				ImGui::Separator();
				ImGui::MenuItem("About", NULL, &show_about);
				ImGui::EndMenu();
			}
			menu_height_ = ImGui::GetWindowHeight();
			ImGui::EndMainMenuBar();
		}
		ImGui::PopStyleVar();

		for (std::size_t i = 0; i < windows_.size(); ++i) {
			Panel* win = windows_[i];
			if (!win->visible_)
				continue;
			float panel_width = 180.f * MainWindow::widget_scaling();
			float offset = 10.0f;
			ImGui::SetNextWindowPos(ImVec2((panel_width + offset) * i, menu_height_ + offset * i), ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowSize(ImVec2(0.0f, 100), ImGuiCond_FirstUseEver);
			ImGui::SetNextWindowSizeConstraints(ImVec2(panel_width, -1.0f), ImVec2(panel_width, -1.0f));
			ImGui::SetNextWindowBgAlpha(alpha_);
			win->draw();
		}
		
		ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); 

        Viewer::post_draw();
	}

}
