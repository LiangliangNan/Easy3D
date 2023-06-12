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

#include "viewer.h"

#include <iostream>

#include <easy3d/util/file_system.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/text_renderer.h>
#include <easy3d/renderer/camera.h>

#include <3rd_party/glfw/include/GLFW/glfw3.h>  // for glfw functions

#include <3rd_party/imgui/misc/fonts/imgui_fonts_droid_sans.h>
#include <3rd_party/imgui/imgui.h>
#include <3rd_party/imgui/backends/imgui_impl_glfw.h>
#include <3rd_party/imgui/backends/imgui_impl_opengl3.h>


namespace easy3d {

    ImGuiContext* ViewerImGui::context_ = nullptr;

    ViewerImGui::ViewerImGui(
        const std::string& title /* = "Easy3D ImGui Viewer" */,
		int samples /* = 4 */,
		int gl_major /* = 3 */,
		int gl_minor /* = 2 */,
		bool full_screen /* = false */,
		bool resizable /* = true */,
		int depth_bits /* = 24 */,
		int stencil_bits /* = 8 */
	) 
		: Viewer(title, samples, gl_major, gl_minor, full_screen, resizable, depth_bits, stencil_bits)
        , alpha_(0.8f)
        , movable_(true)
	{
#if defined(_WIN32) || defined(MSVC)
        // Liangliang: the internal glfw won't be shared accross dll boundaries (But seems ok on mac. That is weird!)
        glfwInit();
#endif

        camera()->setUpVector(vec3(0, 1, 0));
        camera()->setViewDirection(vec3(0, 0, -1));
        camera_->showEntireScene();
	}


    ViewerImGui::~ViewerImGui() {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();

        ImGui::DestroyContext(context_);

        // Not needed: it will be called in the destructor of the base class
        //Viewer::cleanup();

#if defined(_WIN32) || defined(MSVC)
        // Liangliang: the internal glfw won't be shared accross dll boundaries (But seems ok on mac. That is weird!)
        glfwTerminate();
#endif
    }


    void ViewerImGui::init() {
        Viewer::init();

		if (!context_) {
			// Setup ImGui binding
			IMGUI_CHECKVERSION();

			context_ = ImGui::CreateContext();

			const char* glsl_version = "#version 150";
			ImGui_ImplGlfw_InitForOpenGL(window_, false);
			ImGui_ImplOpenGL3_Init(glsl_version);
			ImGuiIO& io = ImGui::GetIO();
            io.WantCaptureKeyboard = true;
            io.WantTextInput = true;
            io.IniFilename = nullptr;
			ImGui::StyleColorsDark();
			ImGuiStyle& style = ImGui::GetStyle();
			style.FrameRounding = 5.0f;

			// load font
			reload_font();
		}
	}


    float ViewerImGui::pixel_ratio() {
        // Computes pixel ratio for hidpi devices
		int fw = framebuffer_width();
		int vw = width(); 
		return static_cast<float>(fw) / static_cast<float>(vw);
    }


    void ViewerImGui::reload_font(int font_size)
	{
		ImGuiIO& io = ImGui::GetIO();
		io.Fonts->Clear();
        io.Fonts->AddFontFromMemoryCompressedTTF(droid_sans_compressed_data, droid_sans_compressed_size, font_size * dpi_scaling());
		io.FontGlobalScale = 1.0f / pixel_ratio();
        ImGui_ImplOpenGL3_DestroyDeviceObjects();
	}


    void ViewerImGui::post_resize(int w, int h) {
        Viewer::post_resize(w, h);
		if (context_) {
			ImGui::GetIO().DisplaySize.x = float(w);
			ImGui::GetIO().DisplaySize.y = float(h);
		}
	}


    bool ViewerImGui::callback_event_cursor_pos(double x, double y) {
		if (ImGui::GetIO().WantCaptureMouse)
			return true;
		else
			return Viewer::callback_event_cursor_pos(x, y);
	}


    bool ViewerImGui::callback_event_mouse_button(int button, int action, int modifiers) {
		if (ImGui::GetIO().WantCaptureMouse)
			return true;
		else
			return Viewer::callback_event_mouse_button(button, action, modifiers);
	}


    bool ViewerImGui::callback_event_keyboard(int key, int action, int modifiers) {
		if (ImGui::GetIO().WantCaptureKeyboard)
            return true;
		else
			return Viewer::callback_event_keyboard(key, action, modifiers);
	}


    bool ViewerImGui::callback_event_character(unsigned int codepoint) {
		if (ImGui::GetIO().WantCaptureKeyboard)
			return true;
		else
			return Viewer::callback_event_character(codepoint);
	}


    bool ViewerImGui::callback_event_scroll(double dx, double dy) {
		if (ImGui::GetIO().WantCaptureMouse)
			return true;
		else
			return Viewer::callback_event_scroll(dx, dy);
	}


    void ViewerImGui::pre_draw() {
        ImGui_ImplOpenGL3_NewFrame();  
        ImGui_ImplGlfw_NewFrame();    
        ImGui::NewFrame();   

        Viewer::pre_draw(); 
	}


    void ViewerImGui::draw_overlay(bool* visible)
    {
        ImGui::SetNextWindowSize(ImVec2(300 * widget_scaling(), 200 * widget_scaling()), ImGuiCond_FirstUseEver);
        const float distance = 10.0f;
        static int corner = 1;
        ImVec2 window_pos = ImVec2(ImGui::GetIO().DisplaySize.x - distance , distance + 30);
        ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
        if (corner != -1)
            ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        ImGui::SetNextWindowBgAlpha(alpha_); // Transparent background
        if (ImGui::Begin("Easy3D: Information", visible, (corner != -1 ? ImGuiWindowFlags_NoMove : 0) | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize/* | ImGuiWindowFlags_AlwaysAutoResize*/ | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
        {
            ImGui::Text("Info (right-click to change position)");
            ImGui::Separator();
            ImGui::Text("Frame rate: %.1f", ImGui::GetIO().Framerate);
            ImGui::Text("GPU time (ms): %s", gpu_time_);

            if (current_model()) {
                const std::string& name = "Current model: " + file_system::simple_name(current_model()->name());
                ImGui::Text("%s", name.c_str());
                if (dynamic_cast<PointCloud*>(current_model())) {
                    auto cloud = dynamic_cast<PointCloud*>(current_model());
                    ImGui::Text("Type: point cloud");
                    ImGui::Text("#Vertices: %i", cloud->n_vertices());
                }
                else if (dynamic_cast<SurfaceMesh*>(current_model())) {
                    auto mesh = dynamic_cast<SurfaceMesh*>(current_model());
                    ImGui::Text("Type: surface mesh");
                    ImGui::Text("#Faces: %i", mesh->n_faces());
                    ImGui::Text("#Vertices: %i", mesh->n_vertices());
                    ImGui::Text("#Edges: %i", mesh->n_edges());
                 }
            }
            ImGui::Separator();

            int w, h;
            viewer_size(w, h);
            float x = ImGui::GetIO().MousePos.x;
            float y = ImGui::GetIO().MousePos.y;
            if (x >= 0 && x <= w && y >= 0 && y <= h) {
                ImGui::Text("Mouse Position: (%i, %i)", static_cast<int>(x), static_cast<int>(y));
                bool target = false;
                const vec3& p = point_under_pixel(static_cast<int>(x), static_cast<int>(y), target);
                if (target)
                    ImGui::Text("Point Under Mouse: (%.5f, %.5f, %.5f)", p.x, p.y, p.z);
                else
                    ImGui::Text("Point Under Mouse: <invalid>");
            }
            else {
                ImGui::Text("Mouse Position: <invalid>");
                ImGui::Text("Point Under Mouse: <invalid>");
            }
            if (ImGui::BeginPopupContextWindow())
            {
                if (ImGui::MenuItem("Custom",       nullptr, corner == -1)) corner = -1;
                if (ImGui::MenuItem("Top-left",     nullptr, corner == 0)) corner = 0;
                if (ImGui::MenuItem("Top-right",    nullptr, corner == 1)) corner = 1;
                if (ImGui::MenuItem("Bottom-left",  nullptr, corner == 2)) corner = 2;
                if (ImGui::MenuItem("Bottom-right", nullptr, corner == 3)) corner = 3;
                if (visible && ImGui::MenuItem("Close")) *visible = false;
                ImGui::EndPopup();
            }
        }
        ImGui::End();
    }


    void ViewerImGui::post_draw() {
        static bool show_overlay = true;
        if (show_overlay)
            draw_overlay(&show_overlay);

        static bool show_about = false;
		if (show_about) {
            ImGui::SetNextWindowPos(ImVec2(width() * 0.5f, height() * 0.5f), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGui::Begin("About Easy3D ImGui Viewer", &show_about, ImGuiWindowFlags_NoResize);
			ImGui::Text(
                "This viewer shows how to use ImGui for GUI creation and event handling"
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

		static bool show_manual = false;
		if (show_manual) {
			int w, h;
            viewer_size(w, h);
			ImGui::SetNextWindowPos(ImVec2(w * 0.5f, h * 0.5f), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
			ImGui::Begin("Easy3D Manual", &show_manual, ImGuiWindowFlags_NoResize);
            ImGui::Text("%s", usage().c_str());
			ImGui::End();
		}

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 8));
        if (ImGui::BeginMainMenuBar())
		{
            draw_menu_file();

            draw_menu_view();

			if (ImGui::BeginMenu("Help"))
			{
                ImGui::MenuItem("Manual", nullptr, &show_manual);
				ImGui::Separator();
                ImGui::MenuItem("About", nullptr, &show_about);
				ImGui::EndMenu();
			}
			menu_height_ = ImGui::GetWindowHeight();
			ImGui::EndMainMenuBar();
		}
        ImGui::PopStyleVar();

		ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); 

        // TODO: a workaround to draw the Easy3D logo at a different location (due to the menu bar)
        auto texter = texter_;
        texter_ = nullptr;
        Viewer::post_draw();
        texter_ = texter;

        // draw Easy3D logo
        if (texter_) {
            const float font_size = 15.0f;
            const float offset_x = (width() * 0.5f - texter_->string_width("Easy3D", font_size) * 0.5f) * dpi_scaling();
            const float offset_y = 50.0f * dpi_scaling();
            texter_->draw("Easy3D", offset_x, offset_y, font_size, 0);
        }
	}


    void ViewerImGui::draw_menu_file() {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open", "Ctrl+O"))
                open();
            if (ImGui::MenuItem("Save As...", "Ctrl+S"))
                save();

            //ImGui::Separator();
            //if (ImGui::BeginMenu("Recent Files...")) {
            //	std::string file_name;
            //	std::vector<Model*>::const_reverse_iterator it = models_.rbegin();
            //	for (; it != models_.rend(); ++it) {
            //		if (ImGui::MenuItem((*it)->name().c_str())) {
            //			file_name = (*it)->name();
            //		}
            //	}
            //	if (!file_name.empty())
            //		open(file_name);
            //	ImGui::EndMenu();
            //}

            ImGui::Separator();
            if (ImGui::MenuItem("Quit", "Alt+F4"))
                glfwSetWindowShouldClose(window_, GLFW_TRUE);

            ImGui::EndMenu();
        }
    }


    void ViewerImGui::draw_menu_view() {
        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::MenuItem("Snapshot", nullptr))
                snapshot();

            ImGui::Separator();
            if (ImGui::BeginMenu("Options"))
            {
                ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.50f);

                static int style_idx = 1;
                if (ImGui::Combo("Style", &style_idx, "Classic\0Dark\0Light\0")) {
                    switch (style_idx) {
                    case 0: ImGui::StyleColorsClassic(); break;
                    case 1: ImGui::StyleColorsDark(); break;
                    case 2: ImGui::StyleColorsLight(); break;
                    }
                }

                ImGui::Checkbox("Panel Movable", &movable_);
                ImGui::ColorEdit3("Background Color", (float*)background_color_, ImGuiColorEditFlags_NoInputs);
                ImGui::DragFloat("Transparency", &alpha_, 0.005f, 0.0f, 1.0f, "%.1f");
                ImGui::PopItemWidth();
                ImGui::EndMenu();
            }

            ImGui::EndMenu();
        }
    }
}
