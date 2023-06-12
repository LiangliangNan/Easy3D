/********************************************************************
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
 ********************************************************************/

#include "viewer.h"

#include <easy3d/renderer/text_renderer.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/resource.h>
#include <easy3d/core/random.h>


using namespace easy3d;


TextRendering::TextRendering(const std::string &title)
        : Viewer(title)
        , texter_(nullptr)
        , font_size_delta_(0.0f)
        , line_spacing_(0.0f)
        , alignment_(TextRenderer::ALIGN_CENTER)
        , upper_left_(true)
{
    usage_string_ =
            "----------------------- Text Rendering usage ------------------------ \n"
            "Press '+'/'-' to increase/decrease font size                          \n"
            "Press '<'/'>' to increase/decrease character spacing                  \n"
            "Press 'up'/'down' to increase/decrease line spacing                   \n"
            "Press 'l'/'c'/'r' to left/center/right align the multi-line text      \n"
            "Press 'o' to switch the origin between 'upper left' and 'bottom left' \n"
            "Press 'space' to enable/disable kerning                               \n"
            "--------------------------------------------------------------------- \n";
}


TextRendering::~TextRendering() {
    delete texter_;

    // Not needed: it will be called in the destructor of the base class
    //Viewer::cleanup();
}


void TextRendering::init() {
    Viewer::init();

    texter_ = new TextRenderer(dpi_scaling());
    if (!texter_)
        return;

    std::vector<std::string> files;
    file_system::get_directory_entries(resource::directory() + "/fonts/", files, false);
    for (const auto& file : files) {
        if (file_system::extension(file) == "ttf") {
            texter_->add_font(resource::directory() + "/fonts/" + file);
            colors_.push_back(random_color(true));
        }
    }

#if 0
    const auto& names = texter_->font_names();
    std::cout << "available fonts: " << std::endl;
    for (std::size_t i =0; i< names.size(); ++i)
        std::cout << "\tfont " << i << ": " << names[i] << std::endl;
#endif
}


bool TextRendering::key_press_event(int key, int modifiers) {
    if (key == KEY_MINUS) {
        font_size_delta_ = std::max(font_size_delta_ - 1.0f, -20.0f);
        update();
        return true;
    }
    else if (key == KEY_EQUAL) {
        font_size_delta_ = std::min(font_size_delta_ + 1.0f, 250.0f);
        update();
        return true;
    }

    else if (key == KEY_COMMA) {
        const float spacing = texter_->character_spacing();
        texter_->set_character_spacing(std::max(spacing - 0.5f, 0.0f));
        update();
        return true;
    }
    else if (key == KEY_PERIOD) {
        const float spacing = texter_->character_spacing();
        texter_->set_character_spacing(std::min(spacing + 0.5f, 50.0f));
        update();
        return true;
    }

    else if (key == KEY_DOWN) {
        line_spacing_ = std::max(line_spacing_ - 0.1f, -1.0f);
        update();
        return true;
    }
    else if (key == KEY_UP) {
        line_spacing_ = std::min(line_spacing_ + 0.1f, 2.0f);
        update();
        return true;
    }

    else if (key == KEY_L) {
        alignment_ = TextRenderer::ALIGN_LEFT;
        update();
        return true;
    }

    else if (key == KEY_C) {
        alignment_ = TextRenderer::ALIGN_CENTER;
        update();
        return true;
    }

    else if (key == KEY_R) {
        alignment_ = TextRenderer::ALIGN_RIGHT;
        update();
        return true;
    }

    else if (key == KEY_O) {
        upper_left_ = !upper_left_;
        update();
        return true;
    }

    else if (key == KEY_SPACE) {
        const bool kerning = texter_->kerning();
        texter_->set_kerning(!kerning);
        update();
        return true;
    }

    else
        return Viewer::key_press_event(key, modifiers);
}


void TextRendering::draw() const {
    Viewer::draw();

    if (!texter_ || texter_->num_fonts() == 0)
        return;

    const float font_size = 28.0f + font_size_delta_;
    float x = 50.0f;
    float y = 80.0f;

    const std::size_t num_fonts = texter_->num_fonts();
    const float font_height = texter_->font_height(font_size);

    texter_->draw(
            "This example shows how to render strings with Easy3D"
            "\n'+'/'-': increase/decrease font size"
            "\n'<'/'>': increase/decrease character spacing"
            "\n'up'/'down': increase/decrease line spacing"
            "\n'l'/'c'/'r': left/center/right align the multi-line text"
            "\n'o': switch the origin between 'upper left' and 'bottom left'"
            "\n'space': enable/disable kerning",
            x * dpi_scaling(), y * dpi_scaling(), font_size, TextRenderer::Align(alignment_), 0, vec3(0, 0, 0),
            line_spacing_, upper_left_);

    // the new Y position to start; add extra space
    y += (font_height * 1.5f + line_spacing_) * 5;

    float next_x = 0.0f;
    for (int i = 0; i < num_fonts; ++i) {
        if (i % 2 == 0) {
            next_x = texter_->draw(std::to_string(i) + " - Easy3D makes 3D easy!     ", x * dpi_scaling(),
                                   y * dpi_scaling(), font_size, i, colors_[i], upper_left_);
        } else {
            texter_->draw(std::to_string(i) + " - I Love Easy3D!", next_x * dpi_scaling(), y * dpi_scaling(), font_size,
                          i, colors_[i], upper_left_);
            y += font_height * 1.5f;
        }
    }
}
