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

#include "viewer.h"

#include <easy3d/util/file_system.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/renderer/camera.h>

#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h> // for glfw functions

#include <3rd_party/imgui/misc/fonts/imgui_fonts_droid_sans.h>
#include <3rd_party/imgui/imgui.h>
#include <3rd_party/imgui/backends/imgui_impl_glfw.h>
#include <algorithm>
#include <cmath>
#include <stdexcept>

using namespace easy3d;

// \cond

// Forward all window/backend settings to the base Viewer; the GUI itself is set up later
// in init(), once the window and rendering context exist.
ViewerImGui::ViewerImGui(const std::string &title,
                         int width,
                         int height,
                         int samples,
                         bool full_screen,
                         bool visible,
                         bool resizable,
#ifdef EASY3D_HAS_VULKAN
                         const VulkanOptions &options
#else
                         const OpenGLOptions &options
#endif
    ) : Viewer(title, width, height, samples, full_screen, visible, resizable, options)
{
    // Liangliang: the internal glfw won't be shared across dll boundaries (Linux and macOS resolve GLFW to one instance).
    //      See https://github.com/LiangliangNan/Easy3D/issues/178
    // Details: The vendored GLFW is a private dependency of easy3d_viewer, so an inherited viewer that calls GLFW
    //      itself links its own copy of the static library. On Windows that copy was never initialized and every
    //      public GLFW function returns without touching the window (_GLFW_REQUIRE_INIT_OR_RETURN), so
    //      glfwSetKeyCallback(window, nullptr) hands back a null host callback and the synthetic input driven
    //      from it faults. Initializing the local copy makes these calls work on the window the library created; the
    //      two modules register separate window classes, so this is safe. .
#if defined(_WIN32) && defined(_MSC_VER)
        if (!glfwInit())
            throw std::runtime_error("glfwInit failed for ViewerImGui's GLFW copy");
#endif
}

ViewerImGui::~ViewerImGui() {
    if (!context_)
        return;
    // Tear down ImGui in reverse order of setup: shut down the GLFW backend, then
    // destroy the context. The base Viewer handles the rest.
    ImGui::SetCurrentContext(context_);
    if (ImGui::GetIO().BackendPlatformUserData)
        ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(context_);
    context_ = nullptr; // input hooks may still run while the window is being torn down

    // Not needed: it will be called in the destructor of the base class
    //Viewer::cleanup();
}

// One-time GUI setup, called by the viewer after the window/context are ready.
void ViewerImGui::init() {
    Viewer::init();

    if (!context_) {
        // Setup ImGui binding
        IMGUI_CHECKVERSION();

        // Each viewer owns a separate ImGui context (ImGui is otherwise global-state based).
        context_ = ImGui::CreateContext();

        // Bind ImGui to GLFW for input/time; "InitForOther" leaves rendering to Easy3D.
        if (!ImGui_ImplGlfw_InitForOther(window(), true))
            throw std::runtime_error("could not initialize ImGui GLFW backend");
        ImGui::GetIO().BackendRendererName = "Easy3D annotations";
        ImGui::GetIO().BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
        ImGuiIO &io = ImGui::GetIO();
        io.WantCaptureKeyboard = true;
        io.WantTextInput = true;
        io.IniFilename = nullptr; // do not persist window layout to an .ini file
        ImGui::StyleColorsLight();
        ImGuiStyle &style = ImGui::GetStyle();
        style.FrameRounding = 5.0f;

        // load font
        reload_font();
    }
}

// Routes input through ImGui first: while a menu, popup or text field owns the
// mouse/keyboard, the hook returns true so the viewer skips its built-in controls.
namespace {
    // The viewer window exists before the lazy init() creates the ImGui context, so window
    // events can reach the hooks with no context (or after teardown destroyed it).
    bool imgui_wants_mouse(ImGuiContext *context) {
        if (!context)
            return false;
        ImGui::SetCurrentContext(context);
        return ImGui::GetIO().WantCaptureMouse;
    }

    bool imgui_wants_keyboard(ImGuiContext *context) {
        if (!context)
            return false;
        ImGui::SetCurrentContext(context);
        return ImGui::GetIO().WantCaptureKeyboard ||
               ImGui::IsPopupOpen(nullptr, ImGuiPopupFlags_AnyPopup);
    }
} // namespace

bool ViewerImGui::mouse_press_event(int x, int y, int button, int modifiers) {
    return imgui_wants_mouse(context_) || Viewer::mouse_press_event(x, y, button, modifiers);
}

bool ViewerImGui::mouse_release_event(int x, int y, int button, int modifiers) {
    return imgui_wants_mouse(context_) || Viewer::mouse_release_event(x, y, button, modifiers);
}

bool ViewerImGui::mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) {
    return imgui_wants_mouse(context_) || Viewer::mouse_drag_event(x, y, dx, dy, button, modifiers);
}

bool ViewerImGui::mouse_free_move_event(int x, int y, int dx, int dy, int modifiers) {
    return imgui_wants_mouse(context_) || Viewer::mouse_free_move_event(x, y, dx, dy, modifiers);
}

bool ViewerImGui::mouse_scroll_event(int x, int y, int dx, int dy) {
    return imgui_wants_mouse(context_) || Viewer::mouse_scroll_event(x, y, dx, dy);
}

bool ViewerImGui::key_press_event(int key, int modifiers) {
    return imgui_wants_keyboard(context_) || Viewer::key_press_event(key, modifiers);
}

bool ViewerImGui::key_release_event(int key, int modifiers) {
    return imgui_wants_keyboard(context_) || Viewer::key_release_event(key, modifiers);
}

bool ViewerImGui::char_input_event(unsigned int codepoint) {
    return imgui_wants_keyboard(context_) || Viewer::char_input_event(codepoint);
}

// Framebuffer pixels per window pixel; >1 on high-DPI (e.g. Retina) displays.
float ViewerImGui::pixel_ratio() {
    // Computes pixel ratio for hidpi devices
    int fw = framebuffer_width();
    int vw = width();
    return static_cast<float>(fw) / static_cast<float>(vw);
}

// Rebuilds the font atlas at the current DPI and uploads it as an Easy3D image so the
// overlay renderer can sample it. Called at init and whenever the pixel ratio changes.
void ViewerImGui::reload_font(int font_size) {
    ImGuiIO &io = ImGui::GetIO();
    io.Fonts->Clear();
    io.Fonts->AddFontFromMemoryCompressedTTF(droid_sans_compressed_data, droid_sans_compressed_size,
                                             static_cast<float>(font_size) * dpi_scaling());
    io.FontGlobalScale = 1.0f / pixel_ratio();
    // Rasterize the glyphs into an RGBA bitmap...
    unsigned char *pixels;
    int w, h;
    io.Fonts->GetTexDataAsRGBA32(&pixels, &w, &h);
    graphics::Sampler sampler;
    sampler.filter = graphics::TextureFilter::linear;
    // ...wrap it in an Easy3D graphics::Image (the overlay texture)...
    font_image_ = std::make_shared<graphics::Image>(
        w, h, std::vector<unsigned char>(pixels, pixels + size_t(w) * h * 4), sampler);
    // ...and tell ImGui to use that texture id when drawing text.
    io.Fonts->SetTexID(ImTextureID(reinterpret_cast<uintptr_t>(font_image_.get())));
}

void ViewerImGui::post_resize(int w, int h) {
    Viewer::post_resize(w, h);
    if (context_) {
        // ImGui lays out widgets against this display size, so keep it current.
        ImGui::GetIO().DisplaySize.x = float(w);
        ImGui::GetIO().DisplaySize.y = float(h);
    }
}

// The heart of the integration: build the ImGui frame, then translate its draw lists into
// Easy3D overlay draw commands that the viewer composites on top of the 3D scene.
void ViewerImGui::pre_draw() {
    ImGui::SetCurrentContext(context_);
    ImGui_ImplGlfw_NewFrame();
    // Re-bake the font if the DPI/pixel ratio changed since the last frame.
    if (std::abs(ImGui::GetIO().FontGlobalScale - 1.0f / pixel_ratio()) > 1e-4f)
        reload_font();
    ImGui::NewFrame();

    Viewer::pre_draw();
    draw_gui(); // user widgets are emitted here (between NewFrame and Render)
    // After Render(), ImGui produces triangle lists with a scissor rect per command.
    const auto *data = ImGui::GetDrawData();
    const ImVec2 scale = data->FramebufferScale, origin = data->DisplayPos;
    std::vector<graphics::DrawCommand> commands;
    for (int n = 0; n < data->CmdListsCount; ++n) {
        const auto *list = data->CmdLists[n];
        for (const auto &draw : list->CmdBuffer) {
            if (draw.UserCallback) {
                if (draw.UserCallback != ImDrawCallback_ResetRenderState)
                    draw.UserCallback(list, &draw);
                continue;
            }
            if (!draw.ElemCount)
                continue;
            // Convert ImGui's scissor rectangle to framebuffer pixels; this becomes the
            // viewport for the overlay command so widgets clip correctly.
            const int x = std::max(0, int(std::floor((draw.ClipRect.x - origin.x) * scale.x)));
            const int y = std::max(0, int(std::floor((draw.ClipRect.y - origin.y) * scale.y)));
            const int right = std::min(framebuffer_width(), int(std::ceil((draw.ClipRect.z - origin.x) * scale.x)));
            const int bottom =
                std::min(framebuffer_height(), int(std::ceil((draw.ClipRect.w - origin.y) * scale.y)));
            if (right <= x || bottom <= y)
                continue;
            if (draw.GetTexID() != ImGui::GetIO().Fonts->TexID)
                throw std::logic_error("this tutorial supports the ImGui font atlas texture only");
            // Expand the indexed triangles into a plain vertex list, converting ImGui's
            // packed RGBA color and pixel-space position into Easy3D vertex attributes.
            std::vector<graphics::Vertex> vertices;
            vertices.reserve(draw.ElemCount);
            for (unsigned i = 0; i < draw.ElemCount; ++i) {
                const auto &source = list->VtxBuffer[list->IdxBuffer[draw.IdxOffset + i] + draw.VtxOffset];
                graphics::Vertex v;
                v.position =
                    vec3((source.pos.x - origin.x) * scale.x - x, (source.pos.y - origin.y) * scale.y - y, 0);
                v.texcoord = vec2(source.uv.x, source.uv.y);
                v.color = vec4(float((source.col >> IM_COL32_R_SHIFT) & 255) / 255,
                               float((source.col >> IM_COL32_G_SHIFT) & 255) / 255,
                               float((source.col >> IM_COL32_B_SHIFT) & 255) / 255,
                               float((source.col >> IM_COL32_A_SHIFT) & 255) / 255);
                vertices.push_back(v);
            }
            graphics::DrawCommand command;
            command.overlay = true; // drawn on top of the 3D scene, in screen space
            command.texture = font_image_;
            command.viewport = ivec4(x, y, right - x, bottom - y);
            command.model_view_projection = graphics::screen_space_projection(right - x, bottom - y);
            command.geometry = std::make_shared<graphics::Geometry>(std::move(vertices));
            commands.push_back(std::move(command));
        }
    }
    // Hand the translated GUI geometry to the viewer for compositing this frame.
    set_overlays(std::move(commands), true);
}

// The actual widgets. In immediate mode this runs every frame: each MenuItem both draws
// itself and returns true when clicked, so the action is invoked right here.
void ViewerImGui::draw_gui() {
    ImGui::SetCurrentContext(context_);
    static bool show_about = false;
    if (show_about) {
        ImGui::SetNextWindowPos(ImVec2(static_cast<float>(width()) * 0.5f, static_cast<float>(height()) * 0.5f),
                                ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
        ImGui::Begin("About Easy3D ImGui Viewer", &show_about, ImGuiWindowFlags_NoResize);
        ImGui::Text("This viewer shows how to use ImGui for GUI creation and event handling");
        ImGui::Separator();
        ImGui::Text("\n" "Liangliang Nan\n" "liangliang.nan@gmail.com\n" "https://3d.bk.tudelft.nl/liangliang/\n");
        ImGui::End();
    }

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 0));
    // The main menu bar spans the top of the window; its height is recorded for layout.
    if (ImGui::BeginMainMenuBar()) {
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

// The "File" menu: each MenuItem invokes the matching base-Viewer action when clicked.
void ViewerImGui::draw_menu_file() {
    if (ImGui::BeginMenu("File")) {
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

// The "View" menu: exposes the viewer's snapshot action.
void ViewerImGui::draw_menu_view() {
    if (ImGui::BeginMenu("View")) {
        if (ImGui::MenuItem("Snapshot", nullptr))
            snapshot();

        ImGui::EndMenu();
    }
}

// \endcond