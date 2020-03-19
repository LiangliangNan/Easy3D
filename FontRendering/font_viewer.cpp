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

#include "font_viewer.h"
#include "opengl_text.h"

#include <easy3d/util/dialogs.h>
#include <easy3d/fileio/resources.h>
#include <easy3d/viewer/opengl_error.h>


using namespace easy3d;


FontViewer::FontViewer(const std::string &title)
        : Viewer(title) {
    set_background_color(vec3(1, 1, 1));
}


void FontViewer::init() {
    Viewer::init();

    LOG(WARNING) << "TODO: add a OpenGLText class, allowing changing text color/font/size etc.";

    texter_ = new ofxFontStash;
    
    std::vector<std::string> font_files = {
            resource::directory() + "/fonts/DroidSerif-Regular.ttf",
            resource::directory() + "/fonts/zachary.ttf",
            resource::directory() + "/fonts/BNFontBoy.ttf",
            resource::directory() + "/fonts/Caribbean.ttf",
            resource::directory() + "/fonts/Cousine-Regular.ttf",
            resource::directory() + "/fonts/DroidSerif-Italic.ttf",
            resource::directory() + "/fonts/Earth-Normal.ttf",
            resource::directory() + "/fonts/G-Unit.ttf",
            resource::directory() + "/fonts/ProggyClean.ttf",
            resource::directory() + "/fonts/Vera.ttf",
            resource::directory() + "/fonts/wds052801.ttf"
    };

    for (const auto& file : font_files)
        texter_->add_font(file);
}


void FontViewer::cleanup() {
    Viewer::cleanup();

    delete texter_;
}


void FontViewer::draw() const {
    Viewer::draw();

    const float font_size = 80.0f;
    float x = 100.0f;
    float y = 200.0f;

    const int num_fonts = texter_->num_fonts();

    const float font_height = texter_->getFontHeight(font_size);

    for (int i = 0; i < num_fonts; ++i) {
        float y1 = y + i * (font_height + 50);
        float x1 = texter_->draw("Easy3D makes 3D easy! ", font_size, x, y1, i);
        texter_->draw("I Love Easy3D!", font_size, x1, y1, i);
    }

}
