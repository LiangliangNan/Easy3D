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

namespace {
const char* const usage_manual = "";
const char* const usage_hint = "Ctrl + o: open an image\n"
            "Mouse wheel: zoom in/out\n"
            "f: reset the view";
}


#include <easy3d/fileio/image_io.h>
#include <easy3d/util/dialog.h>
#include <easy3d/util/resource.h>
#define GLFW_INCLUDE_NONE
#include <3rd_party/glfw/include/GLFW/glfw3.h>
#include <algorithm>
#include <cmath>
#include <stdexcept>

using namespace easy3d;

TutorialImageViewer::TutorialImageViewer(const std::string& title,const std::string& file) : Viewer(title) {
    manual_ = usage_manual;
    hint_ = usage_hint;
    if (!load_image(file)) throw std::runtime_error("could not load image: "+file);
    fit_screen();
}

TutorialImageViewer::~TutorialImageViewer() = default;

bool TutorialImageViewer::key_press_event(int key,int modifiers) {
    if (key == GLFW_KEY_F) { fit_screen(); return true; }
    if (key == GLFW_KEY_O && (modifiers & GLFW_MOD_CONTROL)) {
        const auto file = easy3d::dialog::open("Please choose an image file",easy3d::resource::directory()+"/data/",
            {"Image Files (*.png *.jpg *.bmp *.ppm *.tga)","*.png *.jpg *.bmp *.ppm *.tga"});
        if (!file.empty() && load_image(file)) { fit_screen(); return true; }
        return false;
    }
    return Viewer::key_press_event(key,modifiers);
}

bool TutorialImageViewer::mouse_scroll_event(int,int,int,int dy) {
    const float next = scale_*std::pow(1.1f,float(dy));
    scale_ = std::max(1e-6f,std::min(1e6f,next));
    dirty_ = true; update(); return true;
}

bool TutorialImageViewer::drop_event(const std::vector<std::string>& files) {
    if (files.empty() || !load_image(files.front())) return false;
    fit_screen(); return true;
}

void TutorialImageViewer::post_resize(int,int) {
    dirty_ = true;
}

void TutorialImageViewer::pre_draw() {
    if (!dirty_) return;
    const float w = image_->width()*scale_, h = image_->height()*scale_;
    const float x = (framebuffer_width()-w)*0.5f, y = (framebuffer_height()-h)*0.5f;
    std::vector<easy3d::graphics::Vertex> vertices(4);
    vertices[0].position=easy3d::vec3(x,y,0); vertices[0].texcoord=easy3d::vec2(0,0);
    vertices[1].position=easy3d::vec3(x+w,y,0); vertices[1].texcoord=easy3d::vec2(1,0);
    vertices[2].position=easy3d::vec3(x+w,y+h,0); vertices[2].texcoord=easy3d::vec2(1,1);
    vertices[3].position=easy3d::vec3(x,y+h,0); vertices[3].texcoord=easy3d::vec2(0,1);
    easy3d::graphics::DrawCommand overlay;
    overlay.geometry=std::make_shared<easy3d::graphics::Geometry>(vertices,std::vector<uint32_t>{0,1,2,0,2,3});
    overlay.texture=image_;
    overlay.model_view_projection=easy3d::graphics::screen_space_projection(framebuffer_width(),framebuffer_height());
    set_overlays({overlay}); dirty_=false;
}

void TutorialImageViewer::fit_screen() {
    scale_=std::min(float(framebuffer_width())/image_->width(),float(framebuffer_height())/image_->height());
    dirty_=true; update();
}

bool TutorialImageViewer::load_image(const std::string& file) {
    int w=0,h=0,channels=0;
    std::vector<unsigned char> rgba;
    if (!easy3d::ImageIO::load(file,rgba,w,h,channels,4,false)) return false;
    easy3d::graphics::Sampler sampler; sampler.filter=easy3d::graphics::TextureFilter::linear;
    auto image=std::make_shared<easy3d::graphics::Image>(w,h,std::move(rgba),sampler);
    image_=std::move(image); dirty_=true;
    return true;
}
