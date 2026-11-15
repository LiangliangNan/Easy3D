/** \cond LICENSE
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
 \endcond */

#include "imgui_layer.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <stdexcept>
#include <string>

#include <3rd_party/imgui/imgui.h>
#include <3rd_party/imgui/backends/imgui_impl_glfw.h>
#include <3rd_party/imgui/misc/fonts/imgui_fonts_droid_sans.h>


ImGuiLayer::ImGuiLayer() = default;

ImGuiLayer::~ImGuiLayer() {
    if (!context_)
        return;
    // Tear down in reverse order of setup: the GLFW backend, then the context.
    ImGui::SetCurrentContext(context_);
    if (ImGui::GetIO().BackendPlatformUserData)
        ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(context_);
    context_ = nullptr;
}

bool ImGuiLayer::init(GLFWwindow *window, float dpi_scaling, float pixel_ratio) {
    if (context_)
        return true;

    IMGUI_CHECKVERSION();
    context_ = ImGui::CreateContext();
    ImGui::SetCurrentContext(context_);

    ImGuiIO &io = ImGui::GetIO();
    io.IniFilename = nullptr; // layout is computed by the app, not persisted by ImGui
    io.BackendRendererName = "Easy3D overlays";
    io.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;

    // "InitForOther" binds input and timing but leaves rendering to Easy3D. Installing the
    // backend callbacks chains them to the viewer's own window callbacks.
    if (!ImGui_ImplGlfw_InitForOther(window, true))
        return false;

    ImGui::StyleColorsLight();
    ImGuiStyle &style = ImGui::GetStyle();
    style.WindowRounding = 0.0f;
    style.ChildRounding = 0.0f;
    style.PopupRounding = 4.0f;
    style.FrameRounding = 4.0f;
    style.WindowBorderSize = 1.0f;
    style.WindowPadding = ImVec2(8, 6);
    // Keep FramePadding.y >= DisplaySafeAreaPadding.y: the main menu bar clips its text
    // when the padding is smaller than the safe-area inset.
    style.FramePadding = ImVec2(6, 3);
    style.ItemSpacing = ImVec2(6, 4);

    dpi_scaling_ = dpi_scaling;
    pixel_ratio_ = pixel_ratio;
    reload_font();
    return true;
}

void ImGuiLayer::new_frame(float dpi_scaling, float pixel_ratio) {
    ImGui::SetCurrentContext(context_);
    // The GLFW backend sets the display size and framebuffer scale from the window.
    ImGui_ImplGlfw_NewFrame();
    if (std::abs(dpi_scaling - dpi_scaling_) > 1e-4f || std::abs(pixel_ratio - pixel_ratio_) > 1e-4f) {
        dpi_scaling_ = dpi_scaling;
        pixel_ratio_ = pixel_ratio;
        reload_font();
    }
    ImGui::NewFrame();
}

void ImGuiLayer::set_display_size(int width, int height) {
    if (!context_)
        return;
    ImGui::SetCurrentContext(context_);
    ImGui::GetIO().DisplaySize = ImVec2(static_cast<float>(width), static_cast<float>(height));
}

bool ImGuiLayer::wants_mouse() const {
    if (!context_)
        return false;
    ImGui::SetCurrentContext(context_);
    return ImGui::GetIO().WantCaptureMouse;
}

bool ImGuiLayer::wants_keyboard() const {
    if (!context_)
        return false;
    ImGui::SetCurrentContext(context_);
    // An open menu or popup consumes the keyboard even though it is mouse-driven.
    return ImGui::GetIO().WantCaptureKeyboard || ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup);
}

void ImGuiLayer::reload_font() {
    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->Clear();
    io.Fonts->AddFontFromMemoryCompressedTTF(droid_sans_compressed_data, droid_sans_compressed_size,
                                             static_cast<float>(font_size_) * dpi_scaling_);
    // Rasterize at the physical resolution, then scale the logical size back down.
    io.FontGlobalScale = 1.0f / pixel_ratio_;

    unsigned char *pixels = nullptr;
    int width = 0, height = 0;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    easy3d::graphics::Sampler sampler;
    sampler.filter = easy3d::graphics::TextureFilter::linear;
    font_image_ = std::make_shared<easy3d::graphics::Image>(
            width, height, std::vector<unsigned char>(pixels, pixels + size_t(width) * height * 4), sampler);
    io.Fonts->SetTexID(ImTextureID(reinterpret_cast<uintptr_t>(font_image_.get())));
}

std::vector<easy3d::graphics::DrawCommand> ImGuiLayer::render(int framebuffer_width, int framebuffer_height) {
    ImGui::SetCurrentContext(context_);
    ImGui::Render();

    std::vector<easy3d::graphics::DrawCommand> commands;
    const ImDrawData *data = ImGui::GetDrawData();
    if (!data)
        return commands;

    const ImVec2 scale = data->FramebufferScale;
    const ImVec2 origin = data->DisplayPos;
    const ImTextureID font_texture = ImGui::GetIO().Fonts->TexID;

    for (int n = 0; n < data->CmdListsCount; ++n) {
        const ImDrawList *list = data->CmdLists[n];
        for (const ImDrawCmd &draw : list->CmdBuffer) {
            if (draw.UserCallback) {
                if (draw.UserCallback != ImDrawCallback_ResetRenderState)
                    draw.UserCallback(list, &draw);
                continue;
            }
            if (!draw.ElemCount)
                continue;

            // Convert the scissor rectangle to framebuffer pixels; it becomes the overlay
            // viewport so that widgets clip correctly.
            const int x = std::max(0, int(std::floor((draw.ClipRect.x - origin.x) * scale.x)));
            const int y = std::max(0, int(std::floor((draw.ClipRect.y - origin.y) * scale.y)));
            const int right = std::min(framebuffer_width, int(std::ceil((draw.ClipRect.z - origin.x) * scale.x)));
            const int bottom = std::min(framebuffer_height, int(std::ceil((draw.ClipRect.w - origin.y) * scale.y)));
            if (right <= x || bottom <= y)
                continue;
            if (draw.GetTexID() != font_texture)
                throw std::logic_error("the Mapple UI supports the ImGui font atlas texture only");

            // Expand the indexed triangles into a vertex list in viewport-local coordinates.
            std::vector<easy3d::graphics::Vertex> vertices;
            vertices.reserve(draw.ElemCount);
            for (unsigned i = 0; i < draw.ElemCount; ++i) {
                const ImDrawVert &source = list->VtxBuffer[list->IdxBuffer[draw.IdxOffset + i] + draw.VtxOffset];
                easy3d::graphics::Vertex vertex;
                vertex.position = easy3d::vec3((source.pos.x - origin.x) * scale.x - float(x),
                                               (source.pos.y - origin.y) * scale.y - float(y), 0.0f);
                vertex.texcoord = easy3d::vec2(source.uv.x, source.uv.y);
                vertex.color = easy3d::vec4(float((source.col >> IM_COL32_R_SHIFT) & 255) / 255.0f,
                                            float((source.col >> IM_COL32_G_SHIFT) & 255) / 255.0f,
                                            float((source.col >> IM_COL32_B_SHIFT) & 255) / 255.0f,
                                            float((source.col >> IM_COL32_A_SHIFT) & 255) / 255.0f);
                vertices.push_back(vertex);
            }

            easy3d::graphics::DrawCommand command;
            command.overlay = true;
            command.texture = font_image_;
            command.viewport = easy3d::ivec4(x, y, right - x, bottom - y);
            command.model_view_projection =
                    easy3d::graphics::screen_space_projection(std::uint32_t(right - x), std::uint32_t(bottom - y));
            command.geometry = std::make_shared<easy3d::graphics::Geometry>(std::move(vertices));
            commands.push_back(std::move(command));
        }
    }
    return commands;
}
