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
#include <easy3d/util/file.h>

#include <3rd_party/glfw/include/GLFW/glfw3.h>	// for the KEYs


using namespace easy3d;


ImageViewer::ImageViewer(const std::string& title, const std::string& image_file)
    : Viewer(title)
{
    image_file_ = image_file;
    set_background_color(vec3(1, 1, 1));
}


void ImageViewer::init() {
    Viewer::init();
    texture_ = Texture::create(image_file_);
}


void ImageViewer::cleanup() {
    if (texture_)
        delete texture_;
    Viewer::cleanup();
}


void ImageViewer::draw() {
	if (texture_ == nullptr)
		return;

    static const std::string quad_name = "screen_space/textured_quad";
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

    program->bind();
    program->bind_texture("textureID", texture_->id(), 0);
    opengl::draw_full_screen_quad(ShaderProgram::POSITION, ShaderProgram::TEXCOORD, -0.9f);
    program->release_texture();
    program->release();
}
