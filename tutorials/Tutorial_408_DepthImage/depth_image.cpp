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

#include "depth_image.h"

#include <easy3d/viewer/model.h>
#include <easy3d/viewer/camera.h>
#include <easy3d/viewer/drawable_points.h>
#include <easy3d/viewer/drawable_triangles.h>
#include <easy3d/viewer/framebuffer_object.h>
#include <easy3d/viewer/shader_manager.h>
#include <easy3d/viewer/shader_program.h>
#include <easy3d/viewer/primitives.h>
#include <easy3d/util/dialogs.h>
#include <easy3d/util/file_system.h>

#include <3rd_party/glfw/include/GLFW/glfw3.h>	// for the KEYs


using namespace easy3d;


DepthImage::DepthImage(const std::string& title, const std::string& mesh_file)
    : Viewer(title)
    , depth_rendering_(true)
    , fbo_(nullptr)
{
    if (!open(mesh_file))
        std::cerr << "Error: failed load point cloud." << std::endl;

    set_background_color(vec3(1, 1, 1));

    camera()->setUpVector(vec3(0, 1, 0));
    camera()->setViewDirection(vec3(0, 0, -1));
    camera_->showEntireScene();

    std::cout << "------------ Depth Image ----------" << std::endl
              << "Press 'Space' to switch between depth and normal rendering" << std::endl;
}


bool DepthImage::key_press_event(int key, int modifiers) {
    if (key == GLFW_KEY_SPACE) {
        depth_rendering_ = !depth_rendering_;
        update();
        return true;
    }
    else
        return Viewer::key_press_event(key, modifiers);
}


void DepthImage::cleanup() {
    if (fbo_) {
        delete fbo_;
        fbo_ = nullptr;
    }

    Viewer::cleanup();
}


void DepthImage::draw() const {
    if (depth_rendering_) {
        DepthImage* viewer = const_cast<DepthImage*>(this);
        if (!fbo_) {
            const int samples = 0;
            viewer->fbo_ = new FramebufferObject(width() * dpi_scaling(), height() * dpi_scaling(), samples);
            viewer->fbo_->add_depth_texture(GL_DEPTH_COMPONENT32F, GL_LINEAR, GL_COMPARE_REF_TO_TEXTURE, GL_LEQUAL);
        }
        fbo_->ensure_size(width() * dpi_scaling(), height() * dpi_scaling());

        // generate
        viewer->generate_depth();

        // rendering
        draw_depth();
    }
    else
        Viewer::draw();
}


void DepthImage::generate_depth() {
        static const std::string name = "shadow/shadow_generate";
        ShaderProgram* program = ShaderManager::get_program(name);
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes;
            attributes.push_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
            program = ShaderManager::create_program_from_files(name, attributes);
        }
        if (!program)
            return;

        fbo_->bind();
        fbo_->deactivate_draw_buffers();
        glClear(GL_DEPTH_BUFFER_BIT);
        program->bind();
        program->set_uniform("MVP", camera()->modelViewProjectionMatrix());
        for (auto m : models_) {
            for (auto d : m->points_drawables()) {
                if (d->is_visible()) {
                    glPointSize(d->point_size());
                    d->gl_draw(false);
                }
            }
            for (auto d : m->triangles_drawables()) {
                if (d->is_visible())
                    d->gl_draw(false);
            }
        }
        program->release();
        fbo_->release();
}


void DepthImage::draw_depth() const {
    static const std::string quad_name = "screen_space/quad_gray_texture";
    ShaderProgram* program = ShaderManager::get_program(quad_name);
    if (!program) {
        std::vector<ShaderProgram::Attribute> attributes = {
            ShaderProgram::Attribute(ShaderProgram::POSITION, "vertexMC"),
            ShaderProgram::Attribute(ShaderProgram::TEXCOORD, "tcoordMC")
        };
        program = ShaderManager::create_program_from_files(quad_name, attributes);
    }
    if (!program)
        return;

    program->bind();
    program->bind_texture("textureID", fbo_->depth_texture(), 0);
    opengl::draw_full_screen_quad(ShaderProgram::POSITION, ShaderProgram::TEXCOORD, -0.9f);
    program->release_texture();
    program->release();
}
