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

#include <easy3d/renderer/average_color_blending.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/framebuffer_object.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/opengl_error.h>
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/shape.h>
#include <easy3d/renderer/transform.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/util/setting.h>


namespace easy3d {


    AverageColorBlending::AverageColorBlending(Camera* cam)
        : camera_(cam)
        , fbo_(nullptr)
    {
    }


    AverageColorBlending::~AverageColorBlending()
    {
        clear();
    }


    void AverageColorBlending::clear()
    {
        delete fbo_;
        fbo_ = nullptr;
    }


    void AverageColorBlending::init(int w, int h) {
        if (!fbo_) {
            const int samples = 4;
            fbo_ = new FramebufferObject(w, h, samples);
            fbo_->add_color_texture(GL_RGBA32F, GL_RGBA, GL_FLOAT);
            fbo_->add_color_texture(GL_R32F, GL_RED, GL_FLOAT);
        }
        fbo_->ensure_size(w, h);

        // store the background color that will be restored later
        glGetFloatv(GL_COLOR_CLEAR_VALUE, bkg_color_);
    }


    void AverageColorBlending::geometry_pass(const std::vector<TrianglesDrawable*>& surfaces) {
        static const std::string name = "transparency/average_color_init";
        ShaderProgram* program = ShaderManager::get_program(name);
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes = {
                ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"),
                ShaderProgram::Attribute(ShaderProgram::NORMAL, "vtx_normal")
            };
            std::vector<std::string> outputs;
            outputs.emplace_back("fragOutput0");
            outputs.emplace_back("fragOutput1");
            program = ShaderManager::create_program_from_files(name, attributes, outputs);
        }
        if (!program)
            return;

        const mat4& MVP = camera_->modelViewProjectionMatrix();
        // camera position is defined in world coordinate system.
        const vec3& wCamPos = camera_->position();
        // it can also be computed as follows:
        //const vec3& wCamPos = invMV * vec4(0, 0, 0, 1);
        const mat4& MV = camera_->modelViewMatrix();
        const vec4& wLightPos = inverse(MV) * setting::light_position; 

        program->bind();
        program->set_uniform("MVP", MVP);
        program->set_uniform("wLightPos", wLightPos);
        program->set_uniform("wCamPos", wCamPos);

        for (auto d : surfaces) {
            if (d->is_visible()) {
                // transformation introduced by manipulation
                const mat4 MANIP = d->manipulated_matrix();
                // needs be padded when using uniform blocks
                const mat3 NORMAL = transform::normal_matrix(MANIP);
                program->set_uniform("MANIP", MANIP)
                        ->set_uniform( "NORMAL", NORMAL);
                program->set_uniform("smooth_shading", d->smooth_shading());
                program->set_block_uniform("Material", "ambient", d->material().ambient);
                program->set_block_uniform("Material", "specular", d->material().specular);
                program->set_block_uniform("Material", "shininess", &d->material().shininess);
                program->set_uniform("Alpha", d->opacity());
                program->set_uniform("per_vertex_color", d->coloring_method() != State::UNIFORM_COLOR && d->color_buffer());
                program->set_uniform("default_color", d->color());
                program->set_uniform("selected", d->is_selected());
                program->set_uniform("highlight_color", setting::highlight_color);

                ClippingPlane::instance()->set_program(program);
                ClippingPlane::instance()->set_discard_primitives(program, d->plane_clip_discard_primitive());

                d->gl_draw();
            }
        }

        program->release();
    }


    void AverageColorBlending::draw(const std::vector<TrianglesDrawable*>& surfaces)
    {
        if (surfaces.empty())
            return;

        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        const int w = viewport[2];
        const int h = viewport[3];
        init(w, h);

        glDisable(GL_DEPTH_TEST);

        // ---------------------------------------------------------------------
        // 1. Accumulate Colors and Depth Complexity
        // ---------------------------------------------------------------------
        fbo_->bind();
        fbo_->activate_draw_buffers(0, 1);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_ONE, GL_ONE);
        geometry_pass(surfaces);
        fbo_->release();

        // ---------------------------------------------------------------------
        // 2. Approximate Blending
        // ---------------------------------------------------------------------

        static const std::string name = "transparency/average_color_blend";
        ShaderProgram* program = ShaderManager::get_program(name);
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes = {
                ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"),
                ShaderProgram::Attribute(ShaderProgram::TEXCOORD, "vtx_texcoord")
            };
            program = ShaderManager::create_program_from_files(name, attributes);
        }
        if (!program)
            return;

        glDisable(GL_BLEND);	easy3d_debug_log_gl_error
        glEnable(GL_DEPTH_TEST);

        program->bind();
        program->set_uniform("BackgroundColor", bkg_color_);
        program->bind_texture("ColorTex0", fbo_->color_texture(0), 0);
        program->bind_texture("ColorTex1", fbo_->color_texture(1), 1);
        shape::draw_full_screen_quad(ShaderProgram::POSITION, ShaderProgram::TEXCOORD, 0.0f);
        program->release_texture();
        program->release();

        //////////////////////////////////////////////////////////////////////////

        // restore the default background color
        glClearColor(bkg_color_[0], bkg_color_[1], bkg_color_[2], bkg_color_[3]);
    }


} // namespace easy3d
