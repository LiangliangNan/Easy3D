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


#include <easy3d/util/file_system.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/camera.h>

#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>  // for glfw functions

#include <3rd_party/imgui/misc/fonts/imgui_fonts_droid_sans.h>
#include <3rd_party/imgui/imgui.h>
#include <3rd_party/imgui/backends/imgui_impl_glfw.h>
#include <algorithm>
#include <cmath>
#include <stdexcept>


namespace easy3d {

	// \cond


    ViewerImGui::ViewerImGui(
        const std::string& title,
        int width,
        int height,
        int samples,
        bool full_screen,
        bool visible,
        bool resizable,
#ifdef EASY3D_HAS_VULKAN
        const VulkanOptions& options
#else
        const OpenGLOptions& options
#endif
    ) : Viewer(title, width, height, samples, full_screen, visible, resizable, options)
	{
#if defined(_WIN32) && defined(_MSC_VER)
		// Liangliang: the internal glfw won't be shared across dll boundaries (But seems ok on macOS. That is weird!)
        // see https://github.com/LiangliangNan/Easy3D/issues/178
		glfwInit();
#endif
	}


    ViewerImGui::~ViewerImGui() {
        if (!context_) return;
        set_callbacks({});
        ImGui::SetCurrentContext(context_);
        if (ImGui::GetIO().BackendPlatformUserData) ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext(context_);

        // Not needed: it will be called in the destructor of the base class
        //Viewer::cleanup();
    }


    void ViewerImGui::init() {
        Viewer::init();

		if (!context_) {
			// Setup ImGui binding
			IMGUI_CHECKVERSION();

			context_ = ImGui::CreateContext();

            if (!ImGui_ImplGlfw_InitForOther(window(), true))
                throw std::runtime_error("could not initialize ImGui GLFW backend");
            ImGui::GetIO().BackendRendererName = "Easy3D annotations";
            ImGui::GetIO().BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
            ViewerCallbacks inputs;
            const auto mouse = [this]() { ImGui::SetCurrentContext(context_); return ImGui::GetIO().WantCaptureMouse; };
            const auto keyboard = [this]() { ImGui::SetCurrentContext(context_); return ImGui::GetIO().WantCaptureKeyboard || ImGui::IsPopupOpen(nullptr,ImGuiPopupFlags_AnyPopup); };
            inputs.mouse_press_event = [mouse](ViewerBase*,int,int,int,int) { return mouse(); };
            inputs.mouse_release_event = inputs.mouse_press_event;
            inputs.mouse_drag_event = [mouse](ViewerBase*,int,int,int,int,int,int) { return mouse(); };
            inputs.mouse_free_move_event = [mouse](ViewerBase*,int,int,int,int,int) { return mouse(); };
            inputs.mouse_scroll_event = [mouse](ViewerBase*,int,int,int,int) { return mouse(); };
            inputs.key_press_event = [keyboard](ViewerBase*,int,int) { return keyboard(); };
            inputs.key_release_event = inputs.key_press_event;
            inputs.char_input_event = [keyboard](ViewerBase*,unsigned) { return keyboard(); };
            set_callbacks(std::move(inputs));
			ImGuiIO& io = ImGui::GetIO();
            io.WantCaptureKeyboard = true;
            io.WantTextInput = true;
            io.IniFilename = nullptr;
			ImGui::StyleColorsLight();
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
        io.Fonts->AddFontFromMemoryCompressedTTF(droid_sans_compressed_data, droid_sans_compressed_size, static_cast<float>(font_size) * dpi_scaling());
		io.FontGlobalScale = 1.0f / pixel_ratio();
        unsigned char* pixels; int w, h;
        io.Fonts->GetTexDataAsRGBA32(&pixels, &w, &h);
        graphics::Sampler sampler; sampler.filter = graphics::TextureFilter::linear;
        font_image_ = std::make_shared<graphics::Image>(w, h,
            std::vector<unsigned char>(pixels, pixels + size_t(w)*h*4), sampler);
        io.Fonts->SetTexID(ImTextureID(reinterpret_cast<uintptr_t>(font_image_.get())));
	}


    void ViewerImGui::post_resize(int w, int h) {
        Viewer::post_resize(w, h);
		if (context_) {
			ImGui::GetIO().DisplaySize.x = float(w);
			ImGui::GetIO().DisplaySize.y = float(h);
		}
	}


    void ViewerImGui::pre_draw() {
        ImGui::SetCurrentContext(context_);
        ImGui_ImplGlfw_NewFrame();
        if (std::abs(ImGui::GetIO().FontGlobalScale - 1.0f/pixel_ratio()) > 1e-4f) reload_font();
        ImGui::NewFrame();   

        Viewer::pre_draw();
        draw_gui();
        const auto* data = ImGui::GetDrawData();
        const ImVec2 scale = data->FramebufferScale, origin = data->DisplayPos;
        std::vector<graphics::DrawCommand> commands;
        for (int n = 0; n < data->CmdListsCount; ++n) {
            const auto* list = data->CmdLists[n];
            for (const auto& draw : list->CmdBuffer) {
                if (draw.UserCallback) {
                    if (draw.UserCallback != ImDrawCallback_ResetRenderState) draw.UserCallback(list, &draw);
                    continue;
                }
                if (!draw.ElemCount) continue;
                const int x = std::max(0, int(std::floor((draw.ClipRect.x-origin.x)*scale.x)));
                const int y = std::max(0, int(std::floor((draw.ClipRect.y-origin.y)*scale.y)));
                const int right = std::min(framebuffer_width(), int(std::ceil((draw.ClipRect.z-origin.x)*scale.x)));
                const int bottom = std::min(framebuffer_height(), int(std::ceil((draw.ClipRect.w-origin.y)*scale.y)));
                if (right <= x || bottom <= y) continue;
                if (draw.GetTexID() != ImGui::GetIO().Fonts->TexID)
                    throw std::logic_error("this tutorial supports the ImGui font atlas texture only");
                std::vector<graphics::Vertex> vertices; vertices.reserve(draw.ElemCount);
                for (unsigned i = 0; i < draw.ElemCount; ++i) {
                    const auto& source = list->VtxBuffer[list->IdxBuffer[draw.IdxOffset+i]+draw.VtxOffset];
                    graphics::Vertex v;
                    v.position = vec3((source.pos.x-origin.x)*scale.x-x, (source.pos.y-origin.y)*scale.y-y, 0);
                    v.texcoord = vec2(source.uv.x, source.uv.y);
                    v.color = vec4(float((source.col >> IM_COL32_R_SHIFT)&255)/255,
                                   float((source.col >> IM_COL32_G_SHIFT)&255)/255,
                                   float((source.col >> IM_COL32_B_SHIFT)&255)/255,
                                   float((source.col >> IM_COL32_A_SHIFT)&255)/255);
                    vertices.push_back(v);
                }
                graphics::DrawCommand command;
                command.overlay = true; command.texture = font_image_;
                command.viewport = ivec4(x, y, right-x, bottom-y);
                command.model_view_projection = graphics::screen_space_projection(right-x, bottom-y);
                command.geometry = std::make_shared<graphics::Geometry>(std::move(vertices));
                commands.push_back(std::move(command));
            }
        }
        set_overlays(std::move(commands), true);
    }


    void ViewerImGui::draw_gui() {
        ImGui::SetCurrentContext(context_);
        static bool show_about = false;
		if (show_about) {
            ImGui::SetNextWindowPos(ImVec2(static_cast<float>(width()) * 0.5f, static_cast<float>(height()) * 0.5f), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGui::Begin("About Easy3D ImGui Viewer", &show_about, ImGuiWindowFlags_NoResize);
			ImGui::Text("This viewer shows how to use ImGui for GUI creation and event handling");
			ImGui::Separator();
			ImGui::Text(
				"\n"
				"Liangliang Nan\n"
				"liangliang.nan@gmail.com\n"
				"https://3d.bk.tudelft.nl/liangliang/\n"
			);
			ImGui::End();
		}

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 0));
        if (ImGui::BeginMainMenuBar())
		{
            draw_menu_file();
            draw_menu_view();

			if (ImGui::BeginMenu("Help")) {
                ImGui::MenuItem("About", nullptr, &show_about);
				ImGui::EndMenu();
			}
			menu_height_ = ImGui::GetWindowHeight();
			ImGui::EndMainMenuBar();
		}
        ImGui::PopStyleVar();

        ImGui::Render();
    }


    void ViewerImGui::draw_menu_file() {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Open", "Ctrl+O"))
                open();
            if (ImGui::MenuItem("Save As...", "Ctrl+S"))
                save();

            ImGui::Separator();
            if (ImGui::MenuItem("Quit", "Alt+F4"))
                request_close();

            ImGui::EndMenu();
        }
    }


    void ViewerImGui::draw_menu_view() {
        if (ImGui::BeginMenu("View"))
        {
            if (ImGui::MenuItem("Snapshot", nullptr))
                snapshot();

            ImGui::EndMenu();
        }
    }

	// \endcond

}
