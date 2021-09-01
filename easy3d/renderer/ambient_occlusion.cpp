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

#include <easy3d/renderer/ambient_occlusion.h>

#include <random>

#include <easy3d/core/model.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/framebuffer_object.h>
#include <easy3d/renderer/opengl_error.h>
#include <easy3d/renderer/shapes.h>
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/transform.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/opengl.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/setting.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/transform.h>


const int  KERNEL_SIZE = 64;
const int  NOISE_RES = 4;

//#define SNAPSHOT_BUFFERS

namespace easy3d {

    static float lerp(float a, float b, float f)
    {
        return a + f * (b - a);
    }


    AmbientOcclusion::AmbientOcclusion(Camera* cam)
        : camera_(cam)
        , radius_(0.5f)
        , bias_(0.005f)
        , geom_fbo_(nullptr)
        , ssao_fbo_(nullptr)
        , noise_texture_(0)
    {
    }


    AmbientOcclusion::~AmbientOcclusion() {
        delete geom_fbo_;
        delete ssao_fbo_;

        if (glIsTexture(noise_texture_)) {
            glDeleteTextures(1, &noise_texture_);
            noise_texture_ = 0;
        }
		
		easy3d_debug_log_gl_error;
    }


    void AmbientOcclusion::init(int w, int h)
    {
        // Liangliang: using multsamples (i.e., 4) causes black screen in my macbook laptop.
        //             the bad call is at glReadPixels() with GL_UNSIGNED_BYTE.
        static int samples = 0;
        if (geom_fbo_ == nullptr) {
            geom_fbo_ = new FramebufferObject(w, h, samples);
            geom_fbo_->add_depth_buffer(GL_DEPTH32F_STENCIL8);
            geom_fbo_->add_color_texture(GL_RGB32F, GL_RGB, GL_FLOAT, GL_LINEAR); // position
            geom_fbo_->add_color_texture(GL_RGB32F, GL_RGB, GL_FLOAT, GL_LINEAR); // normal
        }
        geom_fbo_->ensure_size(w, h);

        if (ssao_fbo_ == nullptr) {
            ssao_fbo_ = new FramebufferObject(w, h, samples);
            ssao_fbo_->add_color_texture(GL_R32F, GL_RED, GL_FLOAT, GL_LINEAR); // for ssao result
            ssao_fbo_->add_color_texture(GL_R32F, GL_RED, GL_FLOAT, GL_LINEAR); // for blur result
        }
        ssao_fbo_->ensure_size(w, h);
    }


    void AmbientOcclusion::generate_noise(int width, int height) {
        ssao_kernel_.clear();

        // generates random floats between 0.0 and 1.0
        std::uniform_real_distribution<float> randomFloats(0.0f, 1.0f);
        std::default_random_engine generator;

        // generate sample kernel
        for (unsigned int i = 0; i < KERNEL_SIZE; ++i) {
            vec3 sample(randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator));
            sample = normalize(sample);
            sample *= randomFloats(generator);
            float scale = float(i) / KERNEL_SIZE;

            // scale samples s.t. they're more aligned to center of kernel
            scale = lerp(0.1f, 1.0f, scale * scale);
            sample *= scale;
            ssao_kernel_.push_back(sample);
        }

        // generate noise texture
        std::vector<vec3> ssaoNoise;
        for (int i = 0; i < width * height; i++) {
            //vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f);//ssao
            vec3 noise(randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator) * 2.0f - 1.0f, randomFloats(generator)); // rotate around z-axis (in tangent space)
            ssaoNoise.push_back(noise);
        }
        glGenTextures(1, &noise_texture_);
        glBindTexture(GL_TEXTURE_2D, noise_texture_);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, width, height, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);
    }


    unsigned int AmbientOcclusion::generate(const std::vector<Model*>& models) {
        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        init(viewport[2], viewport[3]);

        geometry_pass(models);

        ssao_pass();

        blur_pass();

        return ssao_texture();
    }


    unsigned int AmbientOcclusion::ssao_texture() const {
         return ssao_fbo_->color_texture(1);
    }


    void AmbientOcclusion::geometry_pass(const std::vector<Model*>& models) {
        static const std::string name = "ssao/geometry_pass";
        ShaderProgram* program = ShaderManager::get_program(name);
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes = {
                ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"),
                ShaderProgram::Attribute(ShaderProgram::NORMAL, "vtx_normal")
            };
            std::vector<std::string> outputs;
            outputs.push_back("gPosition");
            outputs.push_back("gNormal");
            program = ShaderManager::create_program_from_files(name, attributes, outputs);
        }
        if (!program)
            return;

        geom_fbo_->bind(); easy3d_debug_log_gl_error
        geom_fbo_->activate_draw_buffers(0, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        const mat4& MV = camera_->modelViewMatrix();
        const mat4& PROJ = camera_->projectionMatrix();

        program->bind();
        program->set_uniform("MV", MV);
        program->set_uniform("invMV", transform::normal_matrix(MV));
        program->set_uniform("PROJ", PROJ);

        for (auto model : models) {
            if (model->renderer()->is_visible()) {
                // transformation introduced by manipulation
                const mat4 MANIP = model->manipulator()->matrix();
                // needs be padded when using uniform blocks
                const mat3 NORMAL = transform::normal_matrix(MANIP);

                program->set_uniform("MANIP", MANIP)
                        ->set_uniform("NORMAL", NORMAL);

                for (auto d : model->renderer()->points_drawables()) {
                    if (d->is_visible()) {
                        if (setting::clipping_plane)
                            setting::clipping_plane->set_program(program);
                        d->gl_draw(); easy3d_debug_log_gl_error
                    }
                }
                for (auto d : model->renderer()->triangles_drawables()) {
                    if (d->is_visible()) {
                        if (setting::clipping_plane) {
                            setting::clipping_plane->set_program(program);
                            setting::clipping_plane->set_discard_primitives(program, d->plane_clip_discard_primitive());
                        }
                        program->set_uniform("smooth_shading", d->smooth_shading());
                        d->gl_draw(); easy3d_debug_log_gl_error
                    }
                }
                for (auto d : model->renderer()->lines_drawables()) {
                    if (d->is_visible()) {
                        if (setting::clipping_plane) {
                            setting::clipping_plane->set_program(program);
                            setting::clipping_plane->set_discard_primitives(program, d->plane_clip_discard_primitive());
                        }
                        d->gl_draw(); easy3d_debug_log_gl_error
                    }
                }
            }
        }

        program->release();
        geom_fbo_->release(); easy3d_debug_log_gl_error;

#ifdef SNAPSHOT_BUFFERS
        geom_fbo_->snapshot_color_ppm(0, "ssao_gPosition.ppm");
        geom_fbo_->snapshot_color_ppm(1, "ssao_gNormal.ppm");
#endif

	}


    void AmbientOcclusion::ssao_pass() {
        static const std::string name = "ssao/ssao";
        ShaderProgram* program = ShaderManager::get_program(name);
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes = {
                ShaderProgram::Attribute(ShaderProgram::POSITION, "vertexMC"),
                ShaderProgram::Attribute(ShaderProgram::TEXCOORD, "tcoordMC")
            };
            program = ShaderManager::create_program_from_files(name, attributes);
        }
        if (!program)
            return;

        ssao_fbo_->bind(); easy3d_debug_log_gl_error
        ssao_fbo_->activate_draw_buffer(0); easy3d_debug_log_gl_error
        glClear(GL_COLOR_BUFFER_BIT);

        program->bind(); easy3d_debug_log_gl_error

        // Send kernel + rotation
        if (ssao_kernel_.empty())
            generate_noise(NOISE_RES, NOISE_RES);

        program->set_uniform("samples[0]", ssao_kernel_.data()); easy3d_debug_log_gl_error
        const mat4& PROJ = camera_->projectionMatrix();
        program->set_uniform("PROJ", PROJ);
        program->set_uniform("radius", camera_->sceneRadius() * radius_);
        program->set_uniform("bias", bias_);
        program->bind_texture("gPosition", geom_fbo_->color_texture(0), 0);	// position
        program->bind_texture("gNormal", geom_fbo_->color_texture(1), 1);		// normal
        program->bind_texture("texNoise", noise_texture_, 2); easy3d_debug_log_gl_error

        shapes::draw_full_screen_quad(ShaderProgram::POSITION, ShaderProgram::TEXCOORD, 0.0f);
        easy3d_debug_log_gl_error;

        program->release_texture(); easy3d_debug_log_gl_error
        program->release(); easy3d_debug_log_gl_error
        ssao_fbo_->release();

#ifdef SNAPSHOT_BUFFERS
        ssao_fbo_->snapshot_color_ppm(0, "ssao_ssao.ppm");
#endif
     }


    void AmbientOcclusion::blur_pass() {
        static const std::string blur = "ssao/blur";
        ShaderProgram* program = ShaderManager::get_program(blur);
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes = {
                ShaderProgram::Attribute(ShaderProgram::POSITION, "vertexMC"),
                ShaderProgram::Attribute(ShaderProgram::TEXCOORD, "tcoordMC")
            };
            program = ShaderManager::create_program_from_files(blur, attributes);
        }
        if (!program)
            return;

        ssao_fbo_->bind(); easy3d_debug_log_gl_error
        ssao_fbo_->activate_draw_buffer(1);
        glClear(GL_COLOR_BUFFER_BIT); easy3d_debug_log_gl_error

        program->bind(); easy3d_debug_log_gl_error
        program->bind_texture("ssaoInput", ssao_fbo_->color_texture(0), 0);
        shapes::draw_full_screen_quad(ShaderProgram::POSITION, ShaderProgram::TEXCOORD, 0.0f);
        program->release_texture(); easy3d_debug_log_gl_error
        program->release(); easy3d_debug_log_gl_error
        ssao_fbo_->release();

#ifdef SNAPSHOT_BUFFERS
        ssao_fbo_->snapshot_color_ppm(1, "ssao_blur.ppm");
#endif
    }

} // namespace easy3d
