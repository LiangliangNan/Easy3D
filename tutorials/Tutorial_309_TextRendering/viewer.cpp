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


#include <easy3d/renderer/text_layout.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/resource.h>
#include <algorithm>
#include <cmath>
#include <cctype>
#include <stdexcept>

using namespace easy3d;

// \cond
TutorialTextRendering::TutorialTextRendering(const std::string &title) : Viewer(title) {
    // manual_ is the help text shown by the viewer's built-in help overlay.
    manual_ =
        "----------------------- Text Rendering usage ------------------------ \n"
        "Press '+'/'-' to increase/decrease font size                          \n"
        "Press '<'/'>' to increase/decrease character spacing                  \n"
        "Press 'up'/'down' to increase/decrease line spacing                   \n"
        "Press 'l'/'c'/'r' to left/center/right align the multi-line text      \n"
        "Press 'o' to switch the origin between 'upper left' and 'bottom left' \n"
        "Press 'space' to enable/disable kerning                               \n"
        "--------------------------------------------------------------------- \n";

    // Discover every font shipped with the resources and register it with the engine.
    // Each font gets an index (its position in font_names()), used later as TextOptions::font.
    std::vector<std::string> files;
    easy3d::file_system::get_directory_entries(easy3d::resource::directory() + "/fonts/", files, false);
    std::sort(files.begin(), files.end());
    for (const auto &file : files) {
        auto extension = easy3d::file_system::extension(file);
        std::transform(extension.begin(), extension.end(), extension.begin(),
                       [](unsigned char c) { return char(std::tolower(c)); });
        if (extension == "ttf")
            text_.add_font(easy3d::resource::directory() + "/fonts/" + file);
    }
    // Without fonts there is nothing to render, so fail loudly instead of showing a blank HUD.
    if (text_.font_names().empty())
        throw std::runtime_error("Tutorial 309 requires the resource fonts");
}

TutorialTextRendering::~TutorialTextRendering() = default;

// Each recognized key edits one layout parameter (clamped to a sensible range), then marks
// the layout dirty and requests a redraw. Unrecognized keys go to the base Viewer.
bool TutorialTextRendering::key_press_event(int key, int modifiers) {
    using easy3d::graphics::TextAlign;
    switch (key) {
    case KEY_MINUS:
        size_ = std::max(8.0f, size_ - 1);
        break;
    case KEY_EQUAL:
        // '+' is shift+'=' on most keyboard layouts, hence KEY_EQUAL.
        size_ = std::min(278.0f, size_ + 1);
        break;
    case KEY_COMMA:
        // '<' is shift+',' — decrease character spacing.
        spacing_ = std::max(0.0f, spacing_ - 0.5f);
        break;
    case KEY_PERIOD:
        // '>' is shift+'.' — increase character spacing.
        spacing_ = std::min(50.0f, spacing_ + 0.5f);
        break;
    case KEY_DOWN:
        line_spacing_ = std::max(-1.0f, line_spacing_ - 0.1f);
        break;
    case KEY_UP:
        line_spacing_ = std::min(2.0f, line_spacing_ + 0.1f);
        break;
    case KEY_L:
        align_ = TextAlign::left;
        break;
    case KEY_C:
        align_ = TextAlign::center;
        break;
    case KEY_R:
        align_ = TextAlign::right;
        break;
    case KEY_O:
        upper_left_ = !upper_left_;
        break;
    case KEY_SPACE:
        kerning_ = !kerning_;
        break;
    default:
        return Viewer::key_press_event(key, modifiers);
    }
    // Parameter changed: re-layout on the next frame (not once per key event).
    dirty_ = true;
    update();
    return true;
}

// A resize changes the framebuffer the text is laid out for, so invalidate the layout.
void TutorialTextRendering::post_resize(int, int) {
    dirty_ = true;
}

// Called before each frame: rebuild the overlay text only when something changed.
void TutorialTextRendering::pre_draw() {
    Viewer::pre_draw();
    // On high-DPI screens the framebuffer has more pixels than the window reports.
    // Text sizes are in framebuffer pixels, so scale by this ratio to keep the text
    // crisp and the same physical size on any display.
    int w = width();
    const float dpi = w > 0 ? float(framebuffer_width()) / w : 1;
    if (dpi != dpi_) {
        dirty_ = true;
        dpi_ = dpi;
    }
    if (!dirty_)
        return;
    // Discard obsolete font sizes on edits; existing draw commands retain their snapshots.
    text_.clear_cache();
    // Translate the user-facing parameters into engine options (all in framebuffer pixels).
    easy3d::graphics::TextOptions o;
    o.size = size_ * dpi;
    o.x = 50 * dpi;
    o.y = 50 * dpi;
    o.align = align_;
    o.character_spacing = spacing_ * dpi;
    o.line_spacing = line_spacing_;
    o.upper_left = upper_left_;
    o.kerning = kerning_;
    // Lay out the multi-line instruction block; layout() rasterizes the needed glyphs
    // into the atlas and returns one textured-quad draw command per character.
    auto intro = text_.layout(
        "This example shows how to render strings with Easy3D\n" "'+'/'-': increase/decrease font size\n" "'<'/'>': increase/decrease character spacing\n" "'up'/'down': increase/decrease line spacing\n" "'l'/'c'/'r': left/center/right align the multi-line text\n" "'o': switch the origin between 'upper left' and 'bottom left'\n" "'space': enable/disable kerning",
        framebuffer_width(), framebuffer_height(), o);
    auto commands = std::move(intro.commands);
    // Move below the instruction block (7 lines plus a gap) for the font gallery.
    o.y += 7 * intro.line_height + size_ * dpi * 0.5f;
    o.align = easy3d::graphics::TextAlign::left;
    // One sample line per loaded font, two per row, each in a pseudo-random color.
    const auto &names = text_.font_names();
    float next_x = o.x;
    for (std::size_t i = 0; i < names.size(); ++i) {
        o.font = i; // select the i-th registered font
        // Even indices start a new row at the left margin; odd indices continue the row.
        o.x = (i % 2) ? next_x : 50 * dpi;
        o.color = easy3d::vec4(0.15f + 0.5f * float((i * 3) % 7) / 7, 0.1f + 0.5f * float((i * 5) % 7) / 7,
                               0.25f + 0.5f * float(i % 7) / 7, 1);
        const auto label = std::to_string(i) + (i % 2 ? " - I Love Easy3D!" : " - Easy3D makes 3D easy!     ");
        auto line = text_.layout(label, framebuffer_width(), framebuffer_height(), o);
        // advance = how far the line extended; the next sample on this row starts there.
        next_x = o.x + line.advance;
        commands.insert(commands.end(), line.commands.begin(), line.commands.end());
        if (i % 2)
            o.y += size_ * dpi * 1.5f; // row finished: step down to the next one
    }
    // Install the commands as 2D overlays; the viewer draws them on top of the 3D scene.
    set_overlays(std::move(commands));
    dirty_ = false;
}
// \endcond