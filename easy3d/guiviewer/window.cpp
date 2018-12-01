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

#include <3rd_party/imgui/imgui.h>
#include <3rd_party/imgui/impl/imgui_impl_glfw.h>
#include <3rd_party/imgui/impl/imgui_impl_opengl3.h>
#include <3rd_party/GLFW/include/GLFW/glfw3.h>

#include "viewer.h"
#include "ImGuiHelpers.h"


namespace easy3d {


	Window::Window(Viewer* viewer, const std::string& title)
		: name_(title)
		, visible_(true)
	{
		viewer_ = viewer;
		viewer_->windows_.push_back(this);
	}


	void Window::cleanup()
	{
		for (auto p : plugins_)
			p->cleanup();
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
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;
		if (!viewer_->movable_)
			flags |= ImGuiWindowFlags_NoMove;
		ImGui::Begin(name_.c_str(), &visible_, flags);
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

}