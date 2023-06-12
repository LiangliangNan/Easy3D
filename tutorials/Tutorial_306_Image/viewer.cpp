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

#include <easy3d/renderer/texture.h>
#include <easy3d/renderer/shape.h>
#include <easy3d/util/dialog.h>
#include <easy3d/util/resource.h>


using namespace easy3d;

ImageViewer::ImageViewer(const std::string& title, const std::string& image_file)
    : Viewer(title)
    , scale_(1.0f)
    , texture_(nullptr)
{
    image_file_ = image_file;

    usage_string_ =
            "------------ Image Viewer usage ---------- \n"
            "Press 'Ctrl + O' to open an image          \n"
            "Use wheel to zoom in/out                   \n"
            "Press 'F' to reset the view                \n"
            "------------------------------------------ \n";
}


ImageViewer::~ImageViewer() {
    delete texture_;

    // Not needed: it will be called in the destructor of the base class
    //Viewer::cleanup();
}


void ImageViewer::init() {
    Viewer::init();
    texture_ = Texture::create(image_file_);
    fit_screen();
}


void ImageViewer::compute_image_region(int& x, int& y, int& w, int& h) const {
    w = static_cast<int>(static_cast<float>(texture_->width()) * scale_);
    h = static_cast<int>(static_cast<float>(texture_->height()) * scale_);
    x = static_cast<int>(static_cast<float>(width() - w) * 0.5f);
    y = static_cast<int>(static_cast<float>(height() - h) * 0.5f);
}


bool ImageViewer::key_press_event(int key, int modifiers) {
    if (key == KEY_O && modifiers == MODIF_CTRL) {
        const std::string title = "Please choose an image file";
        const std::string default_path = resource::directory() + "/data/";
        const std::vector<std::string> filters = {
            "Image Files (*.png *.jpg *.bmp *.ppm *.tga)", "*.png *.jpg *.bmp *.ppm *.tga"
        };

        const std::string& file_name = dialog::open(title, default_path, filters);
        if (file_name.empty())
            return false;

        delete texture_;
        texture_ = Texture::create(file_name);
        fit_screen();
        return texture_ != nullptr;
    }
    if (key == KEY_F) {
        fit_screen();
        return true;
    }
    else
        return Viewer::key_press_event(key, modifiers);
}


void ImageViewer::fit_screen() {
    if (texture_ == nullptr)
        return;
    const int image_w = texture_->width();
    const int image_h = texture_->height();
    float image_as = static_cast<float>(image_w) / static_cast<float>(image_h);
    float viewer_as = static_cast<float>(width()) / static_cast<float>(height());
    if (image_as < viewer_as) // thin
        scale_ = static_cast<float>(height()) / static_cast<float>(image_h);
    else
        scale_ = static_cast<float>(width()) / static_cast<float>(image_w);
    update();
}


bool ImageViewer::mouse_scroll_event(int x, int y, int dx, int dy) {
    (void)x;
    (void)y;
    (void)dx;
    if (dy > 0)
        scale_ *= 1.1f;
    else if (dy < 0)
        scale_ /= 1.1f;
    update();

    return false;
}


void ImageViewer::draw() const {
	if (texture_ == nullptr)
		return;

    int x, y, w, h;
    compute_image_region(x, y, w, h);

    const Rect quad(static_cast<float>(x), static_cast<float>(x + w), static_cast<float>(y), static_cast<float>(y + h));
    shape::draw_quad_filled(quad, texture_->id(), width(), height(), -0.9f);
}
