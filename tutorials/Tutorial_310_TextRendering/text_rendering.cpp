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

#include "text_rendering.h"

#include <easy3d/viewer/opengl_text.h>
#include <easy3d/util/file_system.h>
#include <easy3d/fileio/resources.h>


using namespace easy3d;


TextRendering::TextRendering(const std::string &title)
        : Viewer(title) {
    set_background_color(vec3(1, 1, 1));
}


void TextRendering::init() {
    Viewer::init();

    texter_ = new OpenGLText;
    
    std::vector<std::string> files;
    file_system::get_directory_entries(resource::directory() + "/fonts/", files, false);
    for (const auto& file : files) {
        if (file_system::extension(file) == "ttf")
            texter_->add_font(resource::directory() + "/fonts/" + file);
    }
}


void TextRendering::cleanup() {
    Viewer::cleanup();

    delete texter_;
}


void TextRendering::draw() const {
    Viewer::draw();

    const float font_size = 80.0f;
    float x = 50.0f;
    float y = 50.0f;

    const int num_fonts = texter_->num_fonts();
    const float font_height = texter_->font_height(font_size);

    float next = 0.0f;
    for (int i = 0; i < num_fonts; ++i) {
        if (i % 2 == 0) {
            y += (font_height + 40);
            next = texter_->draw("Easy3D makes 3D easy! ", x, y, font_size, i);
        }
        else
            texter_->draw("I Love Easy3D!", next, y, font_size, i);
    }

}
