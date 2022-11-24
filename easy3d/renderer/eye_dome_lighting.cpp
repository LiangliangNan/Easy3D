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

#include <easy3d/renderer/eye_dome_lighting.h>
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/framebuffer_object.h>
#include <easy3d/renderer/opengl_error.h>
#include <easy3d/renderer/shape.h>
#include <easy3d/renderer/camera.h>


namespace easy3d {


    EyeDomeLighting::EyeDomeLighting(Camera* cam)
        : camera_(cam)
        , projection_fbo_(nullptr)
        , high_fbo_(nullptr)
        , low_fbo_(nullptr)
        , shade_program_(nullptr)
        , compose_program_(nullptr)
        , blur_program_(nullptr)
        , low_res_factor_(2)
        , z_near_(0.1f)
        , z_far_(1.0f)
        , width_(0)
        , height_(0)
    {
        // init neighbors in image space
        for (int c = 0; c < 8; c++) {
            auto x = static_cast<float>(std::cos(2 * M_PI * c / 8.0));
            auto y = static_cast<float>(std::sin(2 * M_PI * c / 8.0));
            neighbours_[c][0] = x / std::sqrt(x*x + y*y);
            neighbours_[c][1] = y / std::sqrt(x*x + y*y);
            neighbours_[c][2] = 0.;
            neighbours_[c][3] = 0.;
        }
    }


    EyeDomeLighting::~EyeDomeLighting()
    {
        clear();
    }


    void EyeDomeLighting::init_frame_buffers(int w, int h)
    {
        //  PROJECTION FBO and TEXTURES
        if (!projection_fbo_) {
            projection_fbo_ = new FramebufferObject(w, h, 0);
            projection_fbo_->add_color_texture();	// color render target for projection pass
            projection_fbo_->add_depth_texture();	// depth render target for projection pass
        }
        projection_fbo_->ensure_size(w, h);

        //  EDL-RES1 FBO and TEXTURE
        if (!high_fbo_) {
            high_fbo_ = new FramebufferObject(w, h, 0);
            high_fbo_->add_color_texture(GL_RGBA32F, GL_RGBA, GL_FLOAT);	// color render target for EDL full res pass
        }
        high_fbo_->ensure_size(w, h);

        //  EDL-RES2 FBO and TEXTURE
        if (!low_fbo_) {
            low_fbo_ = new FramebufferObject(
                    static_cast<int>(static_cast<float>(w) / low_res_factor_),
                    static_cast<int>(static_cast<float>(h) / low_res_factor_),
                    0
            );
            low_fbo_->add_color_texture(GL_RGBA32F, GL_RGBA, GL_FLOAT);	// color render target for EDL low res pass
            low_fbo_->add_color_texture(GL_RGBA32F, GL_RGBA, GL_FLOAT);	// color render target for EDL low res bilateral filter pass
        }
        low_fbo_->ensure_size(
                static_cast<int>(static_cast<float>(w) / low_res_factor_),
                static_cast<int>(static_cast<float>(h) / low_res_factor_)
        );
    }


    void EyeDomeLighting::init_shader_programs()
    {
        //  EDL SHADE
        static const std::string shade = "edl/edl_shade";
        shade_program_ = ShaderManager::get_program(shade);
        if (!shade_program_) {
            std::vector<ShaderProgram::Attribute> attributes = {
                ShaderProgram::Attribute(ShaderProgram::POSITION, "vertexMC"),
                ShaderProgram::Attribute(ShaderProgram::TEXCOORD, "tcoordMC")
            };
            shade_program_ = ShaderManager::create_program_from_files(shade, attributes);
        }

        //  EDL COMPOSE
        static const std::string compose = "edl/edl_compose";
        compose_program_ = ShaderManager::get_program(compose);
        if (!compose_program_) {
            std::vector<ShaderProgram::Attribute> attributes = {
                ShaderProgram::Attribute(ShaderProgram::POSITION, "vertexMC"),
                ShaderProgram::Attribute(ShaderProgram::TEXCOORD, "tcoordMC")
            };
            compose_program_ = ShaderManager::create_program_from_files(compose, attributes);
        }

        //  BILATERAL FILTER
        static const std::string blur = "edl/edl_bilateral_filter";
        blur_program_ = ShaderManager::get_program(blur);
        if (!blur_program_) {
            std::vector<ShaderProgram::Attribute> attributes = {
                ShaderProgram::Attribute(ShaderProgram::POSITION, "vertexMC"),
                ShaderProgram::Attribute(ShaderProgram::TEXCOORD, "tcoordMC")
            };
            blur_program_ = ShaderManager::create_program_from_files(blur, attributes);
        }
    }


    void EyeDomeLighting::shade_high(float scene_radius)
    {
        if (!shade_program_)
            return;

        //  VARIABLES
        float d = 1.0f;
        float F_scale = 5.0f;
        float SX = 1.0f / float(width_);
        float SY = 1.0f / float(height_);
        float L[3] = { 0.0f, 0.0f, -1.0f };

        high_fbo_->bind();
        high_fbo_->activate_draw_buffer(0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shade_program_->bind();

        // shader parameters
        shade_program_->bind_texture("s2_depth", projection_fbo_->depth_texture(), 0);
        shade_program_->set_uniform("d", d);
        shade_program_->set_uniform("F_scale", F_scale);
        shade_program_->set_uniform("SX", SX);
        shade_program_->set_uniform("SY", SY);
        shade_program_->set_uniform("L", L);
        shade_program_->set_uniform("N[0]", neighbours_[0]);
        shade_program_->set_uniform("Znear", z_near_);
        shade_program_->set_uniform("Zfar", z_far_);

        // set the scene size to the diagonal of scene bounding box.
        float diag = scene_radius * 2.0f;
        shade_program_->set_uniform("SceneSize", diag);

        shape::draw_full_screen_quad(ShaderProgram::POSITION, ShaderProgram::TEXCOORD, 0.0f);

        shade_program_->release_texture();
        shade_program_->release();

        high_fbo_->release();
    }


    void EyeDomeLighting::shade_low()
    {
        if (!shade_program_)
            return;

        //  VARIABLES
        float d = 2.0f;
        float F_scale = 5.0f;
        float SX = 1.0f / (static_cast<float>(width_) / low_res_factor_);
        float SY = 1.0f / (static_cast<float>(height_) / low_res_factor_);
        float L[3] = { 0.0f, 0.0f, -1.0f };

        low_fbo_->bind();
        low_fbo_->activate_draw_buffer(0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shade_program_->bind();

        // shader parameters
        shade_program_->bind_texture("s2_depth", projection_fbo_->depth_texture(), 0);
        shade_program_->set_uniform("d", d);
        shade_program_->set_uniform("F_scale", F_scale);
        shade_program_->set_uniform("SX", SX);
        shade_program_->set_uniform("SY", SY);
        shade_program_->set_uniform("L", L);
        shade_program_->set_uniform("N[0]", neighbours_[0]); // USELESS, ALREADY DEFINED IN FULL RES
        shade_program_->set_uniform("Znear", z_near_);
        shade_program_->set_uniform("Zfar", z_far_);

        shape::draw_quad(
                ShaderProgram::POSITION, ShaderProgram::TEXCOORD, 0, 0,
                static_cast<int>(static_cast<float>(width_) / low_res_factor_),
                static_cast<int>(static_cast<float>(height_) / low_res_factor_),
                width_, height_, 0.0f
        );

        shade_program_->release_texture();
        shade_program_->release();

        low_fbo_->release();
    }


    void EyeDomeLighting::blur_low()
    {
        if (!blur_program_)
            return;

        // shader parameters
        float SX = 1.0f / (static_cast<float>(width_) / low_res_factor_);
        float SY = 1.0f / (static_cast<float>(height_) / low_res_factor_);
        int EDL_Bilateral_N = 5;
        float EDL_Bilateral_Sigma = 2.5f;

        low_fbo_->bind();
        low_fbo_->activate_draw_buffer(1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        blur_program_->bind();

        // shader parameters
        blur_program_->bind_texture("s2_I", low_fbo_->color_texture(0), 1);
        blur_program_->bind_texture("s2_D", projection_fbo_->depth_texture(), 0);
        blur_program_->set_uniform("SX", SX);
        blur_program_->set_uniform("SY", SY);
        blur_program_->set_uniform("N", EDL_Bilateral_N);
        blur_program_->set_uniform("sigma", EDL_Bilateral_Sigma);

        shape::draw_quad(ShaderProgram::POSITION, ShaderProgram::TEXCOORD, 0, 0,
                         static_cast<int>(static_cast<float>(width_) / low_res_factor_),
                         static_cast<int>(static_cast<float>(height_) / low_res_factor_),
                         width_, height_, 0.0f
        );

        blur_program_->release_texture();
        blur_program_->release();

        low_fbo_->release();
    }


    void EyeDomeLighting::compose()
    {
        if (!compose_program_)
            return;

        compose_program_->bind();

        //  EDL shaded texture - full res
        compose_program_->bind_texture("s2_S1", high_fbo_->color_texture(), 2);
        compose_program_->bind_texture("s2_S2", low_fbo_->color_texture(1), 1);

        //  initial color texture
        compose_program_->bind_texture("s2_C", projection_fbo_->color_texture(), 0);

        //-----------------------------------------------------------------------------
        // IMPORTANT since we enable depth writing hereafter
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        // IMPORTANT : so that depth information is propagated
        glDisable(GL_SCISSOR_TEST);
        //-----------------------------------------------------------------------------

        shape::draw_full_screen_quad(ShaderProgram::POSITION, ShaderProgram::TEXCOORD, 0.0f);

        compose_program_->release_texture();
        compose_program_->release();
    }


    void EyeDomeLighting::begin()
    {
        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        width_ = viewport[2];
        height_ = viewport[3];
        if (camera_->type() == Camera::PERSPECTIVE) {
            z_near_ = camera_->zNear();
            z_far_ = camera_->zFar();
        }
        else {	// Liangliang: I tried many times and found this works perfectly in orthogonal mode
            float radius = camera_->sceneRadius();
            z_near_ = 2.0f * radius;
            z_far_ = 10.0f * radius;
        }

        //  Shaders
        init_shader_programs();
        if (!shade_program_ || !compose_program_ || !blur_program_)
            return;

        //  FBOs
        init_frame_buffers(width_, height_);

        projection_fbo_->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }


    void EyeDomeLighting::end()
    {
        projection_fbo_->release();

        // full resolution pass
        shade_high(camera_->sceneRadius());

        // low resolution + blur pass
        shade_low();
        blur_low();

        // compositing pass (in original framebuffer)
        compose();
    }


    void EyeDomeLighting::clear()
    {
        delete projection_fbo_;	projection_fbo_ = nullptr; 	easy3d_debug_log_gl_error
        delete high_fbo_;		high_fbo_ = nullptr;        easy3d_debug_log_gl_error
        delete low_fbo_;		low_fbo_ = nullptr; 		easy3d_debug_log_gl_error
    }

}
