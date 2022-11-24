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

#include <easy3d/core/model.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/framebuffer_object.h>
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/shape.h>
#include <easy3d/renderer/renderer.h>


using namespace easy3d;


DepthImage::DepthImage(const std::string& title)
    : Viewer(title)
    , fbo_(nullptr)
{
    camera()->setUpVector(vec3(0, 1, 0));
    camera()->setViewDirection(vec3(0, 0, -1));
    camera_->showEntireScene();
}


DepthImage::~DepthImage() {
    delete fbo_;

    // Not needed: it will be called in the destructor of the base class
    //Viewer::cleanup();
}


void DepthImage::draw() const {
    draw_depth();
    Viewer::draw();
}


void DepthImage::generate_depth() {
        static const std::string name = "shadow/shadow_generate";
        ShaderProgram* program = ShaderManager::get_program(name);
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes;
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
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
            for (auto d : m->renderer()->points_drawables()) {
                if (d->is_visible()) {
                    glPointSize(d->point_size());
                    d->gl_draw();
                }
            }
            for (auto d : m->renderer()->triangles_drawables()) {
                if (d->is_visible())
                    d->gl_draw();
            }
        }
        program->release();
        fbo_->release();
}


void DepthImage::draw_depth() const {
    auto viewer = const_cast<DepthImage*>(this);
    auto w = static_cast<float>(width()) * dpi_scaling();
    auto h = static_cast<float>(height()) * dpi_scaling();

    if (!fbo_) {
        const int samples = 0;
        viewer->fbo_ = new FramebufferObject(static_cast<int>(w), static_cast<int>(h), samples);
        viewer->fbo_->add_depth_texture(GL_DEPTH_COMPONENT32F, GL_LINEAR, GL_COMPARE_REF_TO_TEXTURE, GL_LEQUAL);
    }
    fbo_->ensure_size(static_cast<int>(w), static_cast<int>(h));

    // generate
    viewer->generate_depth();

    const Rect quad(20 * dpi_scaling(), 20 * dpi_scaling() + w/4, 40 * dpi_scaling(), 40 * dpi_scaling() + h/4);
    shape::draw_depth_texture(quad, fbo_->depth_texture(), static_cast<int>(w), static_cast<int>(h), -0.9f);
    shape::draw_quad_wire(quad, vec4(1.0f, 0.0f, 0.0f, 1.0f), static_cast<int>(w), static_cast<int>(h), -0.99f);
}
