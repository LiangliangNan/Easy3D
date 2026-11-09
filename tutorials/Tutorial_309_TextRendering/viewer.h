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

#ifndef EASY3D_TUTORIAL_TEXT_RENDERING_H
#define EASY3D_TUTORIAL_TEXT_RENDERING_H

#include <easy3d/viewer/viewer.h>

#include <easy3d/renderer/text_layout.h>

/**
 * \brief A Viewer that draws interactive 2D text overlays using a CPU glyph-atlas layout engine.
 *
 * The class owns a TextLayoutEngine (font loading + rasterization + layout) and a set of
 * layout parameters that the user edits with the keyboard. Rebuilding the text is deferred
 * to pre_draw() and gated by a dirty flag, so rapid key presses or resize events cost only
 * one re-layout per frame. key_press_event() edits the parameters, post_resize() marks the
 * layout stale when the window changes, and pre_draw() regenerates the overlay commands.
 */
class TutorialTextRendering : public easy3d::Viewer {
public:
    // Loads all TrueType fonts from the resource directory into the layout engine.
    explicit TutorialTextRendering(const std::string &title);
    ~TutorialTextRendering() override;

protected:
    // Edits the layout parameters (size, spacing, alignment, origin, kerning).
    bool key_press_event(int key, int modifiers) override;
    // Window size changed: the text must be laid out again for the new framebuffer.
    void post_resize(int width, int height) override;
    // Rebuilds the overlay draw commands when the layout is dirty (see viewer.cpp).
    void pre_draw() override;

private:
    // The layout engine; 1024 is the glyph atlas texture size in pixels.
    easy3d::graphics::TextLayoutEngine text_{1024};
    // Layout parameters in window units; dpi_ caches the last framebuffer/window scale.
    float size_ = 28, spacing_ = 0, line_spacing_ = 0, dpi_ = 0;
    easy3d::graphics::TextAlign align_ = easy3d::graphics::TextAlign::center;
    // upper_left_: text origin convention; kerning_: pairwise glyph tightening;
    // dirty_: a re-layout is needed before the next frame.
    bool upper_left_ = true, kerning_ = false, dirty_ = true;
};

#endif // EASY3D_TUTORIAL_TEXT_RENDERING_H
