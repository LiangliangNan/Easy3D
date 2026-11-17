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

#ifndef EASY3D_MAPPLE_IMGUI_LAYER_H
#define EASY3D_MAPPLE_IMGUI_LAYER_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <3rd_party/imgui/imgui.h>

#include <easy3d/renderer/render_device.h>

struct GLFWwindow;
struct ImGuiContext;


/**
 * \brief Owns one Dear ImGui context and translates its draw data into Easy3D overlay commands.
 * \details This is the backend-neutral overlay path of Tutorial_202: the ImGui context is bound
 *      to the viewer's GLFW window for input and time, fonts are rasterized for the current DPI,
 *      and every ImGui draw list becomes a graphics::DrawCommand that the viewer composites on
 *      top of the 3D scene. The font atlas and the icons registered through icon() are the
 *      supported textures.
 */
class ImGuiLayer {
public:
    ImGuiLayer();

    ~ImGuiLayer();

    ImGuiLayer(const ImGuiLayer &) = delete;

    ImGuiLayer &operator=(const ImGuiLayer &) = delete;

    /**
     * \brief Creates the context, the GLFW backend and the font atlas.
     * \param window The viewer window; the backend chains its callbacks to the viewer's.
     * \param dpi_scaling Physical pixels per logical unit, used to rasterize the font.
     * \param pixel_ratio Framebuffer-to-window ratio, used to scale the font back to logical units.
     * \return True on success.
     */
    bool init(GLFWwindow *window, float dpi_scaling, float pixel_ratio);

    /// Starts a new ImGui frame, rebuilding the font atlas if the DPI changed.
    void new_frame(float dpi_scaling, float pixel_ratio);

    /// Ends the current ImGui frame and returns its geometry as overlay draw commands.
    std::vector<easy3d::graphics::DrawCommand> render(int framebuffer_width, int framebuffer_height);

    /// Sets the display size ImGui lays out against, in logical units.
    void set_display_size(int width, int height);

    /// True while ImGui owns the mouse (menus, buttons, sliders, ...).
    bool wants_mouse() const;

    /// True while ImGui owns the keyboard (text input or an open popup).
    bool wants_keyboard() const;

    ImGuiContext *context() const { return context_; }

    /**
     * \brief Returns the overlay texture of an icon shipped in `resources/icons`.
     * \details The file is `<resource directory>/icons/<icon_name>.png`, decoded on first
     *      use and cached for the lifetime of the layer. Failures are cached as well, so a
     *      missing icon logs once instead of every frame. The image is drawn at the font
     *      size, so a 32x32 icon stays sharp on a 2x framebuffer and is minified on a 1x one.
     * \param icon_name File base name without directory or extension, e.g. "open".
     * \return The texture ID to pass to ImGui::Image() or ImGui::ImageButton(), or 0 when
     *      the file is missing or cannot be decoded.
     */
    ImTextureID icon(const std::string &icon_name);

private:
    void reload_font();

    ImGuiContext *context_ = nullptr;
    std::shared_ptr<const easy3d::graphics::Image> font_image_;
    // Icon textures by file base name (0 marks a failed load) and by the texture ID that
    // draw commands carry. The shared pointers keep the images alive while commands using
    // them are still queued.
    std::unordered_map<std::string, ImTextureID> icon_ids_;
    std::unordered_map<ImTextureID, std::shared_ptr<const easy3d::graphics::Image>> icon_images_;
    float dpi_scaling_ = 1.0f;
    float pixel_ratio_ = 1.0f;
    int font_size_ = 16;
};


#endif // EASY3D_MAPPLE_IMGUI_LAYER_H
