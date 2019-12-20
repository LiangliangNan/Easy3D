/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work,
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan.
*           Easy3D: a lightweight, easy-to-use, and efficient C++
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
*
*	Easy3D is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Easy3D is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "image_viewer.h"

#include <easy3d/viewer/texture.h>
#include <easy3d/viewer/shader_manager.h>
#include <easy3d/viewer/shader_program.h>
#include <easy3d/viewer/primitives.h>
#include <easy3d/util/dialogs.h>
#include <easy3d/util/file_system.h>
#include <easy3d/viewer/setting.h>

#include <3rd_party/glfw/include/GLFW/glfw3.h>	// for the KEYs


using namespace easy3d;


ImageViewer::ImageViewer(const std::string& title, const std::string& image_file)
    : Viewer(title)
    , scale_(1.0f)
{
    image_file_ = image_file;
    set_background_color(vec3(1, 1, 1));

    std::cout << "------------ Image Viewer ----------" << std::endl
              << "Press 'Ctrl + O' to open an image" << std::endl
              << "Press 'Space' to reset the view" << std::endl
        << std::endl;
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
    if (key == GLFW_KEY_O && modifiers == GLFW_MOD_CONTROL) {
        const std::string title = "Please choose an image file";
        const std::string default_path = setting::resource_directory() + "/data/";
        const std::vector<std::string> filters = {
            "Image Files (.png .jpg .bmp .ppm .tga)" , "*.png *.jpg *.bmp *.ppm *.tga"
        };

        const std::vector<std::string>& file_names = FileDialog::open(title, default_path, filters, false);
        if (file_names.size() != 1 && !file_system::is_file(file_names[0]))
            return false;

        if (texture_)
            delete texture_;
        texture_ = Texture::create(file_names[0]);
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

    static const std::string quad_name = "screen_space/quad_color_texture";
    ShaderProgram* program = ShaderManager::get_program(quad_name);
    if (!program) {
        std::vector<ShaderProgram::Attribute> attributes = {
            ShaderProgram::Attribute(ShaderProgram::POSITION, "vertexMC"),
            ShaderProgram::Attribute(ShaderProgram::TEXCOORD, "tcoordMC")
        };
        std::vector<std::string> outputs;
        program = ShaderManager::create_program_from_files(quad_name, attributes, outputs, false);
    }
    if (!program)
        return;

    int x, y, w, h;
    compute_image_region(x, y, w, h);

    program->bind();
    program->bind_texture("textureID", texture_->id(), 0);
    opengl::draw_quad(ShaderProgram::POSITION, ShaderProgram::TEXCOORD, x, y, w, h, width(), height(), -0.9f);
    program->release_texture();
    program->release();
}
