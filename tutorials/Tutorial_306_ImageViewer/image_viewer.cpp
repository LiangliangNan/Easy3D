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

#include "image_viewer.h"

#include <easy3d/viewer/texture.h>
#include <easy3d/viewer/primitives.h>
#include <easy3d/util/dialogs.h>
#include <easy3d/fileio/resources.h>

#include <3rd_party/glfw/include/GLFW/glfw3.h>	// for the KEYs


using namespace easy3d;


// enforce the same behavior on macOS and other platforms (i.e., Windows, Linux)
#ifdef __APPLE__
#define EASY3D_MOD_CONTROL GLFW_MOD_SUPER
#else
#define EASY3D_MOD_CONTROL GLFW_MOD_CONTROL
#endif


ImageViewer::ImageViewer(const std::string& title, const std::string& image_file)
    : Viewer(title)
    , scale_(1.0f)
{
    image_file_ = image_file;
    set_background_color(vec3(1, 1, 1));
}


std::string ImageViewer::usage() const {
    return ("------------ Image Viewer usage ---------- \n"
            "Press 'Ctrl + O' to open an image\n"
            "Use wheel to zoom in/out\n"
            "Press 'Space' to reset the view\n"
            "------------------------------------------ \n");
}


void ImageViewer::init() {
    Viewer::init();
    texture_ = Texture::create(image_file_);
    fit_screen();
}


void ImageViewer::cleanup() {
    if (texture_)
        delete texture_;
    Viewer::cleanup();
}


void ImageViewer::compute_image_region(int& x, int& y, int& w, int& h) const {
    w = static_cast<int>(texture_->width() * scale_);
    h = static_cast<int>(texture_->height() * scale_);
    x = static_cast<int>((width() - w) * 0.5f);
    y = static_cast<int>((height() - h) * 0.5f);
}


bool ImageViewer::key_press_event(int key, int modifiers) {
    if (key == GLFW_KEY_O && modifiers == EASY3D_MOD_CONTROL) {
        const std::string title = "Please choose an image file";
        const std::string default_path = resource::directory() + "/data/";
        const std::vector<std::string> filters = {
            "Image Files (*.png *.jpg *.bmp *.ppm *.tga)", "*.png *.jpg *.bmp *.ppm *.tga"
        };

        const std::string& file_name = dialog::open(title, default_path, filters);
        if (file_name.empty())
            return false;

        if (texture_)
            delete texture_;
        texture_ = Texture::create(file_name);
        fit_screen();
        return texture_ != nullptr;
    }
    if (key == GLFW_KEY_SPACE) {
        fit_screen();
        return true;
    }
    else
        return Viewer::key_press_event(key, modifiers);
}


void ImageViewer::fit_screen(const Model *model) {
    (void)model;
    if (texture_ == nullptr)
        return;
    const int image_w = texture_->width();
    const int image_h = texture_->height();
    float image_as = image_w / static_cast<float>(image_h);
    float viewer_as = width() / static_cast<float>(height());
    if (image_as < viewer_as) // thin
        scale_ = height() / static_cast<float>(image_h);
    else
        scale_ = width() / static_cast<float>(image_w);
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

    const Rect quad(x, x + w, y, y + h);
    opengl::draw_quad_filled(quad, texture_->id(), width(), height(), -0.9f);
}
