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


#include <easy3d/viewer/soft_shadow.h>
#include <easy3d/viewer/framebuffer_object.h>
#include <easy3d/viewer/shader_program.h>
#include <easy3d/viewer/shader_manager.h>
#include <easy3d/viewer/camera.h>
#include <easy3d/viewer/opengl_error.h>
#include <easy3d/viewer/frustum.h>
#include <easy3d/viewer/drawable.h>


namespace easy3d {

    SoftShadow::SoftShadow(Camera* cam)
        : Shadow(cam)
        , softness_(0.8f)
        , sample_pattern_(SP_Poisson_64_128)
    {
    }


    SoftShadow::~SoftShadow()
    {
    }


    void SoftShadow::ensure_fbo() {
        if (!fbo_) {
            fbo_ = new FramebufferObject(shadow_map_size_, shadow_map_size_, 0);

            // I will write the shadow map depth into color attachment 0
            fbo_->add_color_texture(GL_R32F, GL_RED, GL_FLOAT, GL_LINEAR);

            // using sampler2DShadow
            fbo_->add_depth_texture(GL_DEPTH_COMPONENT32F, GL_LINEAR, GL_COMPARE_REF_TO_TEXTURE, GL_LEQUAL); // GL_NEAREST
        }
        fbo_->ensure_size(shadow_map_size_, shadow_map_size_);
    }


    void SoftShadow::shadow_map_pass(const std::vector<TrianglesDrawable*>& surfaces)
    {
        static const std::string name = "shadow/soft_shadow_generate";
        ShaderProgram* program = ShaderManager::get_program(name);
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes = {
                ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position")
            };
            program = ShaderManager::create_program_from_files(name, attributes);
        }
        if (!program)
            return;

        // Pass 1 (shadow map generation)
        fbo_->bind();

        // I will write the shadow map depth into the RED component, so the clear
        // value must be set to 1.0f (in case the default clear value is changed).
        float color[4];
        glGetFloatv(GL_COLOR_CLEAR_VALUE, color);

        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        glClearDepthf(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        program->bind();
        program->set_uniform("MVP", light_projection_matrix_ * light_view_matrix_);	easy3d_debug_gl_error;
        for (auto d : surfaces) {
            if (d->is_visible()) {
                d->draw(false);
            }
        }
        program->release();

        // restore the default color
        glClearColor(color[0], color[1], color[2], color[3]);

        fbo_->release();
    }


    void SoftShadow::render_pass(const std::vector<TrianglesDrawable *> &surfaces) {
        static const std::string name = "shadow/soft_shadow_rendering";
        ShaderProgram* program = ShaderManager::get_program(name);
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes = {
                ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"),
                ShaderProgram::Attribute(ShaderProgram::NORMAL, "vtx_normal"),
                ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color")
            };
            program = ShaderManager::create_program_from_files(name, attributes);
        }
        if (!program)
            return;

        const mat4& MVP = camera_->modelViewProjectionMatrix();
        // camera position is defined in world coordinate system.
        const vec3& wCamPos = camera_->position();
        program->bind();
        program->set_uniform("MVP", MVP);                                  easy3d_debug_gl_error;
        program->set_uniform("SHADOW", shadow_matrix_);                    easy3d_debug_gl_error;
        program->set_uniform("wLightPos", light_pos_);                     easy3d_debug_gl_error;
        program->set_uniform("wCamPos", wCamPos);                          easy3d_debug_gl_error;

        program->set_uniform("samplePattern", sample_pattern_);
        program->set_uniform("darkness", darkness_);
        program->set_uniform("lightViewMatrix", light_view_matrix_);
        program->set_uniform("lightRadiusUV", vec2(softness_, softness_));
        program->set_uniform("lightZNear", light_frustum_->near_distance());

        program->set_uniform("lightZFar", light_frustum_->far_distance());
        // set the shadow map depth texture
        program->bind_texture("shadowMapDepth", fbo_->color_texture(0), 1);

        // set the shadow map PCF texture
        program->bind_texture("shadowMapPCF", fbo_->depth_texture(), 2);

        for (auto d : surfaces) {
            if (d->is_visible()) {
                program->set_uniform("default_color", d->default_color());				easy3d_debug_gl_error;
                program->set_uniform("per_vertex_color", d->per_vertex_color() && d->color_buffer());		easy3d_debug_gl_error;
                d->draw(false);
            }
        }

        program->release_texture();
        program->release();
    }

}
