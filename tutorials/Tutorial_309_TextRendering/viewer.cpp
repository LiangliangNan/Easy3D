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

#if EASY3D_TUTORIAL_USES_VULKAN
#include <easy3d/graphics/text_renderer.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/resource.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <cctype>
#include <stdexcept>

using namespace easy3d;

TutorialTextRendering::TutorialTextRendering(const std::string& title,const easy3d::VulkanViewerOptions& options)
        : VulkanViewer(title,options) {
    std::vector<std::string> files;
    easy3d::file_system::get_directory_entries(easy3d::resource::directory()+"/fonts/",files,false);
    std::sort(files.begin(),files.end());
    for (const auto& file:files) {
        auto extension=easy3d::file_system::extension(file);
        std::transform(extension.begin(),extension.end(),extension.begin(),[](unsigned char c) { return char(std::tolower(c)); });
        if (extension=="ttf") text_.add_font(easy3d::resource::directory()+"/fonts/"+file);
    }
    if (text_.font_names().empty()) throw std::runtime_error("Tutorial 309 requires the resource fonts");
}

TutorialTextRendering::~TutorialTextRendering() = default;

bool TutorialTextRendering::key_press_event(int key,int modifiers) {
    using easy3d::graphics::TextAlign;
    switch (key) {
        case GLFW_KEY_MINUS: size_=std::max(8.0f,size_-1); break;
        case GLFW_KEY_EQUAL: size_=std::min(278.0f,size_+1); break;
        case GLFW_KEY_COMMA: spacing_=std::max(0.0f,spacing_-0.5f); break;
        case GLFW_KEY_PERIOD: spacing_=std::min(50.0f,spacing_+0.5f); break;
        case GLFW_KEY_DOWN: line_spacing_=std::max(-1.0f,line_spacing_-0.1f); break;
        case GLFW_KEY_UP: line_spacing_=std::min(2.0f,line_spacing_+0.1f); break;
        case GLFW_KEY_L: align_=TextAlign::left; break;
        case GLFW_KEY_C: align_=TextAlign::center; break;
        case GLFW_KEY_R: align_=TextAlign::right; break;
        case GLFW_KEY_O: upper_left_=!upper_left_; break;
        case GLFW_KEY_SPACE: kerning_=!kerning_; break;
        default: return VulkanViewer::key_press_event(key,modifiers);
    }
    dirty_=true; update(); return true;
}

void TutorialTextRendering::post_resize(int,int) {
    dirty_=true;
}

void TutorialTextRendering::pre_draw() {
    int logical_w,logical_h; glfwGetWindowSize(window(),&logical_w,&logical_h);
    const float dpi=logical_w>0 ? float(width())/logical_w : 1;
    if (dpi!=dpi_) { dirty_=true; dpi_=dpi; }
    if (!dirty_) return;
    // Discard obsolete font sizes on edits; existing draw commands retain their snapshots.
    text_.clear_cache();
    easy3d::graphics::TextOptions o;
    o.size=size_*dpi; o.x=50*dpi; o.y=50*dpi; o.align=align_;
    o.character_spacing=spacing_*dpi; o.line_spacing=line_spacing_;
    o.upper_left=upper_left_; o.kerning=kerning_;
    auto intro=text_.layout(
        "This example shows how to render strings with Easy3D\n"
        "'+'/'-': increase/decrease font size\n"
        "'<'/'>': increase/decrease character spacing\n"
        "'up'/'down': increase/decrease line spacing\n"
        "'l'/'c'/'r': left/center/right align the multi-line text\n"
        "'o': switch the origin between 'upper left' and 'bottom left'\n"
        "'space': enable/disable kerning",width(),height(),o);
    auto commands=std::move(intro.commands);
    o.y+=7*intro.line_height+size_*dpi*0.5f;
    o.align=easy3d::graphics::TextAlign::left;
    const auto& names=text_.font_names();
    float next_x=o.x;
    for (std::size_t i=0;i<names.size();++i) {
        o.font=i; o.x=(i%2) ? next_x : 50*dpi;
        o.color=easy3d::vec4(0.15f+0.5f*float((i*3)%7)/7,0.1f+0.5f*float((i*5)%7)/7,0.25f+0.5f*float(i%7)/7,1);
        const auto label=std::to_string(i)+(i%2 ? " - I Love Easy3D!" : " - Easy3D makes 3D easy!     ");
        auto line=text_.layout(label,width(),height(),o);
        next_x=o.x+line.advance;
        commands.insert(commands.end(),line.commands.begin(),line.commands.end());
        if (i%2) o.y+=size_*dpi*1.5f;
    }
    set_overlays(std::move(commands)); dirty_=false;
}

#else


#include <easy3d/renderer/text_renderer.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/resource.h>
#include <easy3d/core/random.h>


using namespace easy3d;

// \cond

TutorialTextRendering::TutorialTextRendering(const std::string &title)
        : Viewer(title)
        , texter_(nullptr)
        , font_size_delta_(0.0f)
        , line_spacing_(0.0f)
        , alignment_(TextRenderer::ALIGN_CENTER)
        , upper_left_(true)
{
    manual_ =
            "----------------------- Text Rendering usage ------------------------ \n"
            "Press '+'/'-' to increase/decrease font size                          \n"
            "Press '<'/'>' to increase/decrease character spacing                  \n"
            "Press 'up'/'down' to increase/decrease line spacing                   \n"
            "Press 'l'/'c'/'r' to left/center/right align the multi-line text      \n"
            "Press 'o' to switch the origin between 'upper left' and 'bottom left' \n"
            "Press 'space' to enable/disable kerning                               \n"
            "--------------------------------------------------------------------- \n";
}


TutorialTextRendering::~TutorialTextRendering() {
    delete texter_;

    // Not needed: it will be called in the destructor of the base class
    //Viewer::cleanup();
}


void TutorialTextRendering::init() {
    Viewer::init();

    texter_ = new TextRenderer(dpi_scaling());

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


bool TutorialTextRendering::key_press_event(int key, int modifiers) {
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


void TutorialTextRendering::draw() const {
    Viewer::draw();

    if (!texter_ || texter_->num_fonts() == 0)
        return;

    const float font_size = 28.0f + font_size_delta_;
    float x = 50.0f;
    float y = 80.0f;

    const auto num_fonts = texter_->num_fonts();
    const float font_height = texter_->font_height(font_size);

    auto rect = texter_->draw(
            "This example shows how to render strings with Easy3D\n"
            "'+'/'-': increase/decrease font size\n"
            "'<'/'>': increase/decrease character spacing\n"
            "'up'/'down': increase/decrease line spacing\n"
            "'l'/'c'/'r': left/center/right align the multi-line text\n"
            "'o': switch the origin between 'upper left' and 'bottom left'\n"
            "'space': enable/disable kerning",
            x * dpi_scaling(), y * dpi_scaling(), font_size, TextRenderer::Align(alignment_), 0, vec3(0, 0, 0),
            line_spacing_, upper_left_);

    // the new Y position to start; add extra space
    y += rect.height();

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

// \endcond
#endif // native Vulkan / legacy OpenGL
