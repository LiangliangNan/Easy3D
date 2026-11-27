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

#include <easy3d/renderer/dual_depth_peeling.h>

#ifdef EASY3D_HAS_VULKAN
// The Vulkan implementation routes through the portable frame renderer in the header.
#else

#include <algorithm>
#include <memory>
#include <utility>

#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/framebuffer_object.h>
#include <easy3d/renderer/opengl_error.h>
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/shape.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/transform.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/renderer/texture.h>
#include <easy3d/util/setting.h>

namespace easy3d {

    struct DualDepthPeeling::Resources {
        std::unique_ptr<FramebufferObject> layers[2];
        std::unique_ptr<FramebufferObject> accumulation;
    };

    DualDepthPeeling::DualDepthPeeling(Camera* cam)
        : camera_(cam)
        , width_(1024)
        , height_(768)
        , max_peels_(4)
        , stage_(DDP_Inactive)
        , resources_(nullptr)
        , front_source_(FrontA)
        , front_destination_(FrontB)
        , depth_source_(DepthA)
        , depth_destination_(DepthB)
        , current_peel_(0)
        , num_geom_passes_(0)
        , use_occlusion_query_(true)
        , occlusion_query_Id_(0)
        , num_written_pixels_(0)
        , occlusion_threshold_(0)
    {
        LOG(INFO) << "maximum number of peeling layers: " << max_peels_;
    }


    DualDepthPeeling::~DualDepthPeeling()
    {
        clear();
    }


    void DualDepthPeeling::clear() {
        delete_occlusion_query_id();
        delete resources_;
        resources_ = nullptr;
    }


    void DualDepthPeeling::init(int w, int h)
    {
        if (!resources_) {
            resources_ = new Resources;
            // Supersample only the peel layers; resolve into normal-resolution accumulation.
            for (auto &layer : resources_->layers) {
                layer.reset(new FramebufferObject(w * 2, h * 2, 0));
                layer->add_color_texture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, GL_NEAREST);
                layer->add_depth_texture(GL_DEPTH_COMPONENT32F, GL_NEAREST);
            }
            resources_->accumulation.reset(new FramebufferObject(w, h, 0));
            resources_->accumulation->add_color_texture(GL_RGBA16F, GL_RGBA, GL_HALF_FLOAT, GL_NEAREST);
        }
        for (auto &layer : resources_->layers)
            layer->ensure_size(w * 2, h * 2);
        resources_->accumulation->ensure_size(w, h);

        glGetFloatv(GL_COLOR_CLEAR_VALUE, bkg_color_);
        init_occlusion_query();

        front_source_ = FrontA;
        front_destination_ = FrontB;
        depth_source_ = DepthA;
        depth_destination_ = DepthB;
        num_geom_passes_ = 0;
        current_peel_ = 0;
    }


    void DualDepthPeeling::init_occlusion_query()
    {
        delete_occlusion_query_id();
        glGenQueries(1, &occlusion_query_Id_);
        occlusion_threshold_ = 0;
        num_written_pixels_ = occlusion_threshold_ + 1;
    }


    bool DualDepthPeeling::peeling_done() const
    {
        const int limit = max_peels_ == 0 ? 16 : max_peels_;
        return current_peel_ >= limit ||
               (use_occlusion_query_ && num_written_pixels_ <= occlusion_threshold_);
    }


    void DualDepthPeeling::start_occlusion_query() const
    {
    #ifdef GL_ES_VERSION_3_0
        glBeginQuery(GL_ANY_SAMPLES_PASSED, occlusion_query_Id_);
    #else
        glBeginQuery(GL_SAMPLES_PASSED, occlusion_query_Id_);
    #endif
    }


    void DualDepthPeeling::end_occlusion_query()
    {
    #ifdef GL_ES_VERSION_3_0
        glEndQuery(GL_ANY_SAMPLES_PASSED);
        GLuint any_samples_passed;
        glGetQueryObjectuiv(occlusion_query_Id_, GL_QUERY_RESULT, &any_samples_passed);
        num_written_pixels_ = any_samples_passed ? occlusion_threshold_ + 1 : 0;
    #else
        glEndQuery(GL_SAMPLES_PASSED);
        glGetQueryObjectuiv(occlusion_query_Id_, GL_QUERY_RESULT, &num_written_pixels_);
    #endif
    }


    void DualDepthPeeling::delete_occlusion_query_id()
    {
        if (occlusion_query_Id_) {
            glDeleteQueries(1, &occlusion_query_Id_);
            occlusion_query_Id_ = 0;
        }
    }


    void DualDepthPeeling::geometry_pass(const std::vector<TrianglesDrawable*>& surfaces) {
        if (stage_ != DDP_Peel)
            return;

        static const std::string name = "transparency/dual_depth_peeling_peel_rendering";
        ShaderProgram* program = ShaderManager::get_program(name);
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes = {
                ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"),
                ShaderProgram::Attribute(ShaderProgram::TEXCOORD, "vtx_texcoord"),
                ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"),
                ShaderProgram::Attribute(ShaderProgram::NORMAL, "vtx_normal")
            };
            program = ShaderManager::create_program_from_files(name, attributes);
        }
        if (!program)
            return;

        const unsigned previous = static_cast<unsigned>(front_source_ - FrontA);
        const mat4& MVP = camera_->model_view_projection_matrix();
        const vec3& wCamPos = camera_->position();
        const mat4& MV = camera_->model_view_matrix();
        const vec4& wLightPos = inverse(MV) * setting::light_position;

        program->bind();
        program->set_uniform("wLightPos", wLightPos);
        program->set_uniform("wCamPos", wCamPos);
        program->bind_texture("PreviousDepthTex", resources_->layers[previous]->depth_texture(), 0);
        for (auto d : surfaces) {
            if (!d->is_visible())
                continue;

            const mat4 MANIP = d->manipulated_matrix();
            const mat3 NORMAL = transform::normal_matrix(MANIP);
            program->set_uniform("MANIP", MANIP)
                    ->set_uniform("MVP_MANIP", MVP * MANIP)
                    ->set_uniform("NORMAL", NORMAL)
                    ->set_uniform("lighting", d->lighting())
                    ->set_uniform("smooth_shading", d->smooth_shading())
                    ->set_uniform("two_sides_lighting", d->lighting_two_sides())
                    ->set_uniform("distinct_back_color", d->distinct_back_color())
                    ->set_uniform("backside_color", d->back_color())
                    ->set_block_uniform("Material", "ambient", d->material().ambient)
                    ->set_block_uniform("Material", "specular", d->material().specular)
                    ->set_block_uniform("Material", "shininess", &d->material().shininess)
                    ->set_uniform("Alpha", d->opacity())
                    ->set_uniform("per_vertex_color", d->coloring_method() != State::UNIFORM_COLOR && !d->buffer_data().colors.empty())
                    ->set_uniform("default_color", d->color())
                    ->set_uniform("selected", d->is_selected())
                    ->set_uniform("highlight_color", setting::highlight_color);

            ClippingPlane::instance()->set_program(program);
            ClippingPlane::instance()->set_discard_primitives(program, d->plane_clip_discard_primitive());

            const bool use_texture = d->texture() &&
                (d->coloring_method() == State::SCALAR_FIELD || d->coloring_method() == State::TEXTURED);
            program->set_uniform("use_texture", use_texture);
            if (use_texture)
                program->bind_texture("textureID", d->texture()->id(), 2)
                        ->set_uniform("texture_repeat", d->texture_repeat())
                        ->set_uniform("fractional_repeat", d->texture_fractional_repeat());
            d->gl_draw();
            if (use_texture)
                program->release_texture();
        }
        program->release_texture();
        program->release();
        ++num_geom_passes_;
    }


    void DualDepthPeeling::init_depth_buffers(const std::vector<TrianglesDrawable*>&) {
        glViewport(0, 0, width_ * 2, height_ * 2);
        glDisable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_SCISSOR_TEST);
        // OpenGL depth clears are masked by GL_DEPTH_WRITEMASK.
        glDepthMask(GL_TRUE);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

        for (auto &layer : resources_->layers) {
            layer->bind();
            layer->activate_draw_buffer(0);
            glClearColor(0, 0, 0, 0);
            glClearDepth(0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        }
        glDepthMask(GL_FALSE);

        resources_->accumulation->bind();
        resources_->accumulation->activate_draw_buffer(0);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
    }


    void DualDepthPeeling::draw(const std::vector<TrianglesDrawable*>& surfaces) {
        if (surfaces.empty())
            return;

        GLint viewport[4];
        GLint previous_draw_fbo;
        GLint previous_read_fbo;
        GLboolean depth_test;
        GLboolean blend;
        GLboolean scissor_test;
        GLboolean depth_mask;
        GLboolean color_mask[4];
        GLenum depth_func;
        GLint blend_src_rgb, blend_dst_rgb, blend_src_alpha, blend_dst_alpha;
        GLenum blend_equation_rgb, blend_equation_alpha;
        GLfloat clear_color[4];
        GLdouble clear_depth;
        glGetIntegerv(GL_VIEWPORT, viewport);
        glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &previous_draw_fbo);
        glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &previous_read_fbo);
        depth_test = glIsEnabled(GL_DEPTH_TEST);
        blend = glIsEnabled(GL_BLEND);
        scissor_test = glIsEnabled(GL_SCISSOR_TEST);
        glGetBooleanv(GL_DEPTH_WRITEMASK, &depth_mask);
        glGetBooleanv(GL_COLOR_WRITEMASK, color_mask);
        glGetIntegerv(GL_DEPTH_FUNC, reinterpret_cast<GLint*>(&depth_func));
        glGetIntegerv(GL_BLEND_SRC_RGB, &blend_src_rgb);
        glGetIntegerv(GL_BLEND_DST_RGB, &blend_dst_rgb);
        glGetIntegerv(GL_BLEND_SRC_ALPHA, &blend_src_alpha);
        glGetIntegerv(GL_BLEND_DST_ALPHA, &blend_dst_alpha);
        glGetIntegerv(GL_BLEND_EQUATION_RGB, reinterpret_cast<GLint*>(&blend_equation_rgb));
        glGetIntegerv(GL_BLEND_EQUATION_ALPHA, reinterpret_cast<GLint*>(&blend_equation_alpha));
        glGetFloatv(GL_COLOR_CLEAR_VALUE, clear_color);
        glGetDoublev(GL_DEPTH_CLEAR_VALUE, &clear_depth);

        width_ = viewport[2];
        height_ = viewport[3];
        init(width_, height_);

        stage_ = DDP_InitDepth;
        init_depth_buffers(surfaces);

        stage_ = DDP_Peel;
        while (!peeling_done()) {
            peel(surfaces);
            swap_targets();
        }

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(previous_draw_fbo));
        glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(previous_read_fbo));
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        if (scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
        stage_ = DDP_Blend;
        blend_final_image();

        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(previous_draw_fbo));
        glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(previous_read_fbo));
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        if (blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
        if (depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
        if (scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
        glDepthMask(depth_mask);
        glColorMask(color_mask[0], color_mask[1], color_mask[2], color_mask[3]);
        glDepthFunc(depth_func);
        glBlendEquationSeparate(blend_equation_rgb, blend_equation_alpha);
        glBlendFuncSeparate(static_cast<GLenum>(blend_src_rgb), static_cast<GLenum>(blend_dst_rgb),
                            static_cast<GLenum>(blend_src_alpha), static_cast<GLenum>(blend_dst_alpha));
        glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
        glClearDepth(clear_depth);

        delete_occlusion_query_id();
        stage_ = DDP_Inactive;
    }


    void DualDepthPeeling::peel(const std::vector<TrianglesDrawable*>& surfaces) {
        stage_ = DDP_Peel;
        ++current_peel_;

        const unsigned target = static_cast<unsigned>(front_destination_ - FrontA);
        auto &layer = resources_->layers[target];
        layer->bind();
        layer->activate_draw_buffer(0);
        glViewport(0, 0, width_ * 2, height_ * 2);
        glDisable(GL_SCISSOR_TEST);
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glDepthMask(GL_TRUE);
        glClearColor(0, 0, 0, 0);
        glClearDepth(1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        geometry_pass(surfaces);

        resources_->accumulation->bind();
        resources_->accumulation->activate_draw_buffer(0);
        glViewport(0, 0, width_, height_);
        // New layers are farther away, so place them under the existing premultiplied result.
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFuncSeparate(GL_ONE_MINUS_DST_ALPHA, GL_ONE,
                            GL_ONE_MINUS_DST_ALPHA, GL_ONE);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);

        if (use_occlusion_query_)
            start_occlusion_query();

        static const std::string name = "transparency/dual_depth_peeling_blend";
        ShaderProgram* program = ShaderManager::get_program(name);
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes = {
                ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"),
                ShaderProgram::Attribute(ShaderProgram::TEXCOORD, "vtx_texcoord")
            };
            program = ShaderManager::create_program_from_files(name, attributes);
        }
        if (program) {
            program->bind();
            program->bind_texture("TempTex", layer->color_texture(0), 0);
            shape::draw_full_screen_quad(ShaderProgram::POSITION, ShaderProgram::TEXCOORD, 0.0f);
            program->release_texture();
            program->release();
        }

        if (use_occlusion_query_)
            end_occlusion_query();
    }


    void DualDepthPeeling::swap_targets() {
        std::swap(front_source_, front_destination_);
        std::swap(depth_source_, depth_destination_);
    }


    void DualDepthPeeling::blend_final_image() {
        static const std::string name = "transparency/dual_depth_peeling_final";
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

        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        glEnable(GL_BLEND);
        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

        program->bind();
        program->bind_texture("AccumulatedTex", resources_->accumulation->color_texture(0), 0);
        shape::draw_full_screen_quad(ShaderProgram::POSITION, ShaderProgram::TEXCOORD, 0.0f);
        program->release_texture();
        program->release();
    }


} // namespace easy3d

#endif // EASY3D_HAS_VULKAN
