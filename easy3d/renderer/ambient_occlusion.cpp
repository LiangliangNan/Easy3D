/** \cond LICENSE
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
 \endcond */

#include <easy3d/renderer/ambient_occlusion.h>

#include <algorithm>

#include <easy3d/core/model.h>
#include <easy3d/renderer/camera.h>

#ifdef EASY3D_HAS_VULKAN

#include <map>


namespace easy3d {

    namespace {
        struct FrameParams {
            float radius;
            float bias;
            mat4 projection;
        };

        std::map<const Camera*, FrameParams>& frames() {
            static std::map<const Camera*, FrameParams> value;
            return value;
        }
    }

    AmbientOcclusion::AmbientOcclusion(Camera* cam)
        : camera_(cam)
        , radius_(0.5f)
        , bias_(0.005f)
    {
    }

    AmbientOcclusion::~AmbientOcclusion() {
        if (camera_)
            frames().erase(camera_);
    }

    void AmbientOcclusion::begin(const std::vector< std::shared_ptr<Model> >&) {
        if (!camera_)
            return;
        FrameParams p{};
        p.radius = std::max(camera_->scene_radius() * radius_, 1e-4f);
        p.bias = bias_;
        p.projection = camera_->projection_matrix();
        frames()[camera_] = p;
    }

    void AmbientOcclusion::end() {
        if (camera_)
            frames().erase(camera_);
    }

    bool AmbientOcclusion::frame_params(const Camera* camera, float& radius, float& bias, mat4& projection) {
        auto it = frames().find(camera);
        if (it == frames().end())
            return false;
        radius = it->second.radius;
        bias = it->second.bias;
        projection = it->second.projection;
        return true;
    }

}

#else

#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/framebuffer_object.h>
#include <easy3d/renderer/opengl_error.h>
#include <easy3d/renderer/shape.h>
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/opengl.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/renderer.h>


//#define SNAPSHOT_BUFFER

namespace easy3d {


    AmbientOcclusion::AmbientOcclusion(Camera* cam)
        : camera_(cam)
        , radius_(0.5f)
        , bias_(0.005f)
        , projection_fbo_(nullptr)
        , ssao_fbo_(nullptr)
    {
    }


    AmbientOcclusion::~AmbientOcclusion() {
        delete projection_fbo_;
        delete ssao_fbo_;

		easy3d_debug_log_gl_error
    }


    void AmbientOcclusion::init(int w, int h)
    {
        width_ = w;
        height_ = h;

        if (projection_fbo_ == nullptr) {
            projection_fbo_ = new FramebufferObject(w, h, 0);
            projection_fbo_->add_color_texture();
            // Positions and normals are reconstructed from this depth buffer, so
            // no additional geometry pass is needed.
            projection_fbo_->add_depth_texture(GL_DEPTH_COMPONENT32F);
        }
        projection_fbo_->ensure_size(w, h);

        // Liangliang: using multsamples (i.e., 4) causes black screen in my macbook laptop.
        //             the bad call is at glReadPixels() with GL_UNSIGNED_BYTE.
        if (ssao_fbo_ == nullptr) {
            ssao_fbo_ = new FramebufferObject(w, h, 0);
            ssao_fbo_->add_color_texture(GL_R32F, GL_RED, GL_FLOAT, GL_LINEAR); // for ssao result
            ssao_fbo_->add_color_texture(GL_R32F, GL_RED, GL_FLOAT, GL_LINEAR); // for blur result
        }
        ssao_fbo_->ensure_size(w, h);
    }


    void AmbientOcclusion::begin(const std::vector< std::shared_ptr<Model> >&) {
        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        init(viewport[2], viewport[3]);

        projection_fbo_->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }


    void AmbientOcclusion::end() {
        if (!projection_fbo_)
            return;

        projection_fbo_->release();

        ssao_pass();
        blur_pass();
        compose();
    }


    unsigned int AmbientOcclusion::generate(const std::vector< std::shared_ptr<Model> >& models) {
        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        init(viewport[2], viewport[3]);

        // Only the depth of this buffer is used, so the models are drawn with
        // their regular shaders.
        projection_fbo_->bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        for (auto model : models) {
            if (!model->renderer()->is_visible())
                continue;
            for (auto d : model->renderer()->triangles_drawables())
                if (d->is_visible()) d->draw(camera_);
            for (auto d : model->renderer()->lines_drawables())
                if (d->is_visible()) d->draw(camera_);
            for (auto d : model->renderer()->points_drawables())
                if (d->is_visible()) d->draw(camera_);
        }
        projection_fbo_->release();

        ssao_pass();

        blur_pass();

        return ssao_texture();
    }


    unsigned int AmbientOcclusion::ssao_texture() const {
         return ssao_fbo_->color_texture(1);
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

        program->set_uniform("invPROJ", inverse(camera_->projection_matrix()));
        program->set_uniform("PROJ", camera_->projection_matrix());
        program->set_uniform("radius", std::max(camera_->scene_radius() * radius_, 1e-4f));
        program->set_uniform("bias", bias_);
        program->bind_texture("scene_depth", projection_fbo_->depth_texture(), 0);

        shape::draw_full_screen_quad(ShaderProgram::POSITION, ShaderProgram::TEXCOORD, 0.0f);
        easy3d_debug_log_gl_error

        program->release_texture(); easy3d_debug_log_gl_error
        program->release(); easy3d_debug_log_gl_error
        ssao_fbo_->release();

#ifdef SNAPSHOT_BUFFER
        ssao_fbo_->snapshot_color(0, "/tmp/ssao_raw.png");
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
        program->set_uniform("invPROJ", inverse(camera_->projection_matrix()));
        program->set_uniform("radius", std::max(camera_->scene_radius() * radius_, 1e-4f));
        program->bind_texture("scene_depth", projection_fbo_->depth_texture(), 0);
        program->bind_texture("ssaoInput", ssao_fbo_->color_texture(0), 1);
        shape::draw_full_screen_quad(ShaderProgram::POSITION, ShaderProgram::TEXCOORD, 0.0f);
        program->release_texture(); easy3d_debug_log_gl_error
        program->release(); easy3d_debug_log_gl_error
        ssao_fbo_->release();

#ifdef SNAPSHOT_BUFFER
        ssao_fbo_->snapshot_color(1, "/tmp/ssao_blur.png");
#endif
    }


    void AmbientOcclusion::compose() {
        // The generic multiply-compose pass also used by eye-dome lighting:
        // output = scene color * mix(shade1, shade2, 0.3). Both scale textures are
        // the blurred SSAO result, so the mix reduces to that single factor.
        static const std::string name = "edl/edl_compose";
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

        GLboolean depth_mask = GL_TRUE;
        glGetBooleanv(GL_DEPTH_WRITEMASK, &depth_mask);
        const GLboolean depth_test = glIsEnabled(GL_DEPTH_TEST);
        const GLboolean blend = glIsEnabled(GL_BLEND);

        program->bind(); easy3d_debug_log_gl_error
        const unsigned int ao = ssao_texture();
        program->bind_texture("s2_S1", ao, 2);
        program->bind_texture("s2_S2", ao, 1);
        program->bind_texture("s2_C", projection_fbo_->color_texture(), 0);

        // The pass must not alter or be rejected by the destination depth buffer.
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        shape::draw_full_screen_quad(ShaderProgram::POSITION, ShaderProgram::TEXCOORD, 0.0f);
        glDepthMask(depth_mask);
        if (depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
        if (blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);

        program->release_texture(); easy3d_debug_log_gl_error
        program->release(); easy3d_debug_log_gl_error
    }

} // namespace easy3d

#endif // EASY3D_HAS_VULKAN
