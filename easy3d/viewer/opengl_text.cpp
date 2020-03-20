/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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
 */


#include "opengl_text.h"

#include <easy3d/core/types.h>

#include <3rd_party/fontstash/fontstash.h>


namespace easy3d {

    OpenGLText::OpenGLText(int texture_size, bool mipmaps) {
        texture_size_ = geom::next_pow2(texture_size);
        stash_ = sth_create(texture_size_, texture_size_, mipmaps);
    }

    OpenGLText::~OpenGLText() {
        if (stash_ != nullptr)
            sth_delete(stash_);
    }

    bool OpenGLText::add_font(const std::string &font_file) {
        if (stash_ == nullptr) {
            LOG(ERROR) << "addFont() error: font stash_ not initialized, call setup first";
            return false;
        }

        const int id = sth_add_font(stash_, font_file.c_str());
        if (id <= 0) {
            LOG(ERROR) << "could not load font: "<< font_file;
            return false;
        }

        font_ids_.push_back(id);
        LOG(INFO) << "loaded font '" << font_file << "' in texture (" << texture_size_ << " x " << texture_size_ << ")";
        return true;
    }


    float OpenGLText::draw(const std::string &text, float x, float y, float font_size, int fontID) const {
        float dx = 0.0f;
        if (stash_ != nullptr) {
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            sth_begin_draw(stash_);
            sth_draw_text(stash_, font_ids_[fontID], font_size, x, y, text.c_str(), &dx); //this might draw
            sth_end_draw(stash_); // this actually draws
            glDisable(GL_BLEND);
        } else {
            LOG(ERROR) << "couldn't draw() due to the failure in initialization";
        }
        return dx;
    }


    float OpenGLText::font_height(float font_size) const {
        float asc, desc, lineh;
        sth_vmetrics(stash_, font_ids_[0], font_size, &asc, &desc, &lineh);
        return asc - desc;
    }

}