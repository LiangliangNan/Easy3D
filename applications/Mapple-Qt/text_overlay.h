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

#ifndef MAPPLE_QT_TEXT_OVERLAY_H
#define MAPPLE_QT_TEXT_OVERLAY_H

#include <memory>
#include <string>

#include <easy3d/core/types.h>


namespace easy3d {
    class ShaderProgram;
    namespace graphics {
        class TextLayoutEngine;
    }
}


// Draws single-line UTF-8 text with the backend-independent TextLayoutEngine and a tiny OpenGL pass.
// All coordinates are in framebuffer pixels. A current OpenGL context is required for draw().
class TextOverlay {
public:
    TextOverlay();
    ~TextOverlay();

    // Returns false if the font cannot be loaded.
    bool add_font(const std::string &file);
    std::size_t num_fonts() const;

    // (x, y) is the upper-left corner of the text box, measured from the upper-left corner of the viewport.
    void draw(const std::string &text, float x, float y, float font_size, std::size_t font_id,
              const easy3d::vec3 &color = easy3d::vec3(0, 0, 0)) const;

private:
    std::unique_ptr<easy3d::graphics::TextLayoutEngine> engine_;
    mutable std::unique_ptr<easy3d::ShaderProgram> shader_;
};

#endif
