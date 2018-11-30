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

#include "window.h"
#include "plugin.h"

#include <iostream>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <GLFW/glfw3.h>

#include "viewer.h"
#include "imgui_fonts_droid_sans.h"
#include "ImGuiHelpers.h"


namespace easy3d {


	ImGuiContext* Window::context_ = nullptr;


	Window::Window(Viewer* viewer, const std::string& title)
		: name_(title)
		, movable_(false)
		, alpha_(0.8f)
	{
		viewer_ = viewer;
		viewer_->windows_.push_back(this);
	}


	void Window::init() {
		if (!context_) {
			// Setup ImGui binding
			IMGUI_CHECKVERSION();

			context_ = ImGui::CreateContext();

			const char* glsl_version = "#version 150";
			ImGui_ImplGlfw_InitForOpenGL(viewer_->window_, false);
			ImGui_ImplOpenGL3_Init(glsl_version);
			ImGuiIO& io = ImGui::GetIO();
			//io.IniFilename = nullptr;
			ImGui::StyleColorsDark();
			ImGuiStyle& style = ImGui::GetStyle();
			style.FrameRounding = 5.0f;

			// load font
			reload_font();
		}
	}

	void Window::reload_font(int font_size)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.Fonts->Clear();
		io.Fonts->AddFontFromMemoryCompressedTTF(droid_sans_compressed_data, droid_sans_compressed_size, font_size * hidpi_scaling());
		io.FontGlobalScale = 1.0f / pixel_ratio();
	}


	void Window::cleanup()
	{
		for (auto p : plugins_)
			p->cleanup();
	}


	void Window::post_resize(int width, int height)
	{
		if (context_)
		{
			ImGui::GetIO().DisplaySize.x = float(width);
			ImGui::GetIO().DisplaySize.y = float(height);
		}
	}

	// Mouse IO
	bool Window::mouse_press(int button, int modifier)
	{
		ImGui_ImplGlfw_MouseButtonCallback(viewer_->window_, button, GLFW_PRESS, modifier);
		return ImGui::GetIO().WantCaptureMouse;
	}

	bool Window::mouse_release(int button, int modifier)
	{
		return ImGui::GetIO().WantCaptureMouse;
	}

	bool Window::mouse_move(int mouse_x, int mouse_y)
	{
		return ImGui::GetIO().WantCaptureMouse;
	}

	bool Window::mouse_scroll(double delta_y)
	{
		ImGui_ImplGlfw_ScrollCallback(viewer_->window_, 0.f, delta_y);
		return ImGui::GetIO().WantCaptureMouse;
	}

	// Keyboard IO
	bool Window::char_input(unsigned int key)
	{
		ImGui_ImplGlfw_CharCallback(nullptr, key);
		return ImGui::GetIO().WantCaptureKeyboard;
	}

	bool Window::key_press(int key, int modifiers)
	{
		ImGui_ImplGlfw_KeyCallback(viewer_->window_, key, 0, GLFW_PRESS, modifiers);
		return ImGui::GetIO().WantCaptureKeyboard;
	}

	bool Window::key_release(int key, int modifiers)
	{
		ImGui_ImplGlfw_KeyCallback(viewer_->window_, key, 0, GLFW_RELEASE, modifiers);
		return ImGui::GetIO().WantCaptureKeyboard;
	}


	bool Window::draw()
	{
		float offset = 0.0f;
		float menu_width = 180.f * menu_scaling();
		ImGui::SetNextWindowPos(ImVec2(offset, offset), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(0.0f, offset), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSizeConstraints(ImVec2(menu_width, -1.0f), ImVec2(menu_width, -1.0f));
		ImGui::SetNextWindowBgAlpha(alpha_);

		ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar
			| ImGuiWindowFlags_MenuBar
			| ImGuiWindowFlags_AlwaysAutoResize;
		if (!movable_)
			flags |= ImGuiWindowFlags_NoMove;
		ImGui::Begin(name_.c_str(), NULL, flags);
		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.4f);

		draw_widgets();
		for (auto p : plugins_)
			p->draw();

		ImGui::PopItemWidth();
		ImGui::End();
		return false;
	}


	void Window::draw_widgets()
	{
		static bool show_about = false;
		if (show_about)
		{
			ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowWidth() + 10, 10));
			ImGui::Begin("About easy3d", &show_about, ImGuiWindowFlags_AlwaysAutoResize);
			ImGui::Text(
				"easy3d is an easy, lightweight, and flexible framework for developing\n"
				"cross-platform 3D applications. It requires minimum dependencies, i.e.\n"
				"\t- GLFW (for cross-platform OpenGL context creation) and\n"
				"\t- ImGui (for GUI creation and event handling,\n"
				"\n"
				"easy3d works on all major operating systems with a decent C++11 capable\n"
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

		// Menu
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open", "Ctrl+O")) { viewer_->open(); }
				if (ImGui::MenuItem("Save As...", "Ctrl+S")) { viewer_->save(); }

				ImGui::Separator();
				if (ImGui::BeginMenu("Recent Files...")) {
					ImGui::MenuItem("bunny.ply");
					ImGui::MenuItem("terain.las");
					ImGui::MenuItem("building.obj");
					ImGui::EndMenu();
				}

				ImGui::Separator();
				if (ImGui::MenuItem("Quit", "Alt+F4"))
					glfwSetWindowShouldClose(viewer_->window_, GLFW_TRUE);

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

					ImGui::Checkbox("Window Movable", &movable_);
					ImGui::ColorEdit3("Background Color", (float*)viewer_->background_color(), ImGuiColorEditFlags_NoInputs);
					ImGui::EndMenu();
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
				ImGui::MenuItem("Viewer", NULL, false);
				ImGui::MenuItem("Shortcut", NULL, false);
				ImGui::Separator();
				ImGui::MenuItem("About", NULL, &show_about);
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		// Workspace
		if (ImGui::CollapsingHeader("Workspace", ImGuiTreeNodeFlags_DefaultOpen))
		{
			float w = ImGui::GetContentRegionAvailWidth();
			float p = ImGui::GetStyle().FramePadding.x;
			if (ImGui::Button("Load##Workspace", ImVec2((w - p) / 2.f, 0)))
			{
				//viewer_->load_scene();
			}
			ImGui::SameLine(0, p);
			if (ImGui::Button("Save##Workspace", ImVec2((w - p) / 2.f, 0)))
			{
				//viewer_->save_scene();
			}
		}

		// Mesh
		if (ImGui::CollapsingHeader("Mesh", ImGuiTreeNodeFlags_DefaultOpen))
		{
			float w = ImGui::GetContentRegionAvailWidth();
			float p = ImGui::GetStyle().FramePadding.x;
			if (ImGui::Button("Load##Mesh", ImVec2((w - p) / 2.f, 0)))
			{
				viewer_->open();
			}
			ImGui::SameLine(0, p);
			if (ImGui::Button("Save##Mesh", ImVec2((w - p) / 2.f, 0)))
			{
				viewer_->save();
			}
		}
	}



	float Window::pixel_ratio()
	{
		// Computes pixel ratio for hidpi devices
		int buf_size[2];
		int win_size[2];
		GLFWwindow* window = glfwGetCurrentContext();
		glfwGetFramebufferSize(window, &buf_size[0], &buf_size[1]);
		glfwGetWindowSize(window, &win_size[0], &win_size[1]);
		return (float)buf_size[0] / (float)win_size[0];
	}

	float Window::hidpi_scaling()
	{
		// Computes scaling factor for hidpi devices
		float xscale, yscale;
		GLFWwindow* window = glfwGetCurrentContext();
		glfwGetWindowContentScale(window, &xscale, &yscale);
		return 0.5f * (xscale + yscale);
	}

}