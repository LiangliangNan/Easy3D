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
#include <easy3d/util/resource.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_309_TextRendering/main.cpp
 *
 * \brief Rendering 2D strings (labels, HUDs, annotations) with Easy3D's text layout engine.
 *
 * <b>Overview</b>
 * This tutorial draws text on top of a 3D scene: a multi-line instruction block plus a gallery
 * line for every installed font, each in a different color. All layout and rasterization happen
 * on the CPU in a TextLayoutEngine; the resulting textured quads are handed to the viewer as
 * overlay draw commands. The TutorialTextRendering class below rebuilds the text whenever a
 * layout parameter changes (font size, spacing, alignment, origin, kerning, window size, DPI).
 *
 * <b>The technique</b>
 * Real-time text rendering is usually done with a glyph (font) atlas: each needed character is
 * rasterized once from the TrueType outlines into a small region of a texture, and drawing a
 * string then becomes drawing one textured quad per character, sampling its glyph from the
 * atlas. Layout decides where each quad goes: advancing by each glyph's width, applying extra
 * character/line spacing, aligning shorter lines within the longest one, and optionally kerning
 * (tightening specific pairs like "AV" using the font's kerning table). Because glyph bitmaps
 * are resolution-dependent, sizes are specified in framebuffer pixels: on high-DPI displays the
 * framebuffer is larger than the window, so the tutorial scales all sizes by
 * framebuffer_width()/width() to keep the text crisp and physically the same size. Layouts are
 * immutable snapshots, so previously issued draw commands stay valid even after the cache is
 * cleared and new glyphs are rasterized.
 *
 * <b>Easy3D API</b>
 * - graphics::TextLayoutEngine: CPU TrueType rasterization and layout; add_font() loads .ttf
 *   files, layout() turns a UTF-8 string + TextOptions into TextLayout (draw commands, bounds,
 *   advance, line height), clear_cache() frees cached glyph sizes.
 * - graphics::TextOptions / TextAlign: size, position, color, spacing, alignment, origin
 *   (upper-left vs bottom-left), and kerning.
 * - Viewer::set_overlays(): installs the draw commands as 2D overlays rendered on top of the
 *   scene; pre_draw()/post_resize() are the hooks used to rebuild them lazily.
 *
 * <b>Running it</b>
 * '+'/'-' change the font size, '<'/'>' the character spacing, 'up'/'down' the line spacing,
 * 'l'/'c'/'r' the alignment of the multi-line block, 'o' switches the origin between upper-left
 * and bottom-left, and 'space' toggles kerning. The usual viewer navigation still works.
 *
 * The header file of the viewer class:
 * \include{lineno} Tutorial_309_TextRendering/viewer.h
 * The source file of the viewer class:
 * \include{lineno} Tutorial_309_TextRendering/viewer.cpp
 *
 * \see graphics::TextLayoutEngine, graphics::TextOptions, Viewer
 */

using namespace easy3d;

int main(int argc, char **argv) {
    // initialize Easy3D.
    initialize();

    // A simple background model so the text overlays are seen on top of a real 3D scene.
    const std::string file_name = resource::directory() + "/data/sphere.obj";

    // The constructor loads every .ttf font found in the resource fonts directory.
    TutorialTextRendering viewer(EXAMPLE_TITLE);

    viewer.add_model(file_name);

    // run the viewer
    return viewer.run();
}
