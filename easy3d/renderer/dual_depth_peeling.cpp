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

#include <easy3d/renderer/dual_depth_peeling.h>

#include <algorithm>

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

//#define SAVE_INTERMEDIATE_FBO
//#define SHOW_DEBUG_INFO

namespace easy3d {

    DualDepthPeeling::DualDepthPeeling(Camera* cam)
        : camera_(cam)
        , width_(1024)
        , height_(768)
        , max_peels_(4)
        , stage_(DDP_Inactive)
        , fbo_(nullptr)
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
    }


    DualDepthPeeling::~DualDepthPeeling()
    {
        clear();
    }


    void DualDepthPeeling::clear() {
        delete fbo_;
        fbo_ = nullptr;
    }


    void DualDepthPeeling::init(int w, int h)
    {   
        if (!fbo_) {
            // Liangliang: multisampling causes artifacts on sharp edges.
            //             So it is suggested to disable multisampling with DDP.
            const int samples = 0;
            fbo_ = new FramebufferObject(w, h, samples);
            // Liangliang: must have the same order as in TextureName.
            // The depth has to be treated like a color attachment, since it's a 2 component min-max texture.
            fbo_->add_color_texture(GL_RG32F, GL_RG, GL_FLOAT);				// DepthA
            fbo_->add_color_texture(GL_RG32F, GL_RG, GL_FLOAT);				// DepthB

            fbo_->add_color_texture(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);	// BackTemp
            fbo_->add_color_texture(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);	// Back
            fbo_->add_color_texture(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);	// FrontA
            fbo_->add_color_texture(GL_RGBA8, GL_RGBA, GL_UNSIGNED_BYTE);	// FrontB
        }
        fbo_->ensure_size(w, h);

        //////////////////////////////////////////////////////////////////////////

        // store the background color that will be restored later
        glGetFloatv(GL_COLOR_CLEAR_VALUE, bkg_color_);

        init_occlusion_query();

        num_geom_passes_ = 0;
        current_peel_ = 0;

    #ifdef SAVE_INTERMEDIATE_FBO
        if (file_system::is_directory("snapshotBuffers")) {
            std::vector<std::string> files;
            file_system::get_files("snapshotBuffers", files, false);
            for (std::size_t i = 0; i < files.size(); ++i)
                file_system::delete_file(files[i]);
        }
        else
            file_system::create_directory("snapshotBuffers");
    #endif
    }


    void DualDepthPeeling::init_occlusion_query()
    {
        glGenQueries(1, &occlusion_query_Id_);

    #if 0// if you want to specify the occlusion ratio
        int num_pixels = fbo_->width() * fbo_->height();
        float occlusion_ratio = 0.05f;
        occlusion_threshold_ = static_cast<unsigned int>(num_pixels * occlusion_ratio);
    #else
        occlusion_threshold_ = 0;
    #endif

        num_written_pixels_ = occlusion_threshold_ + 1;
    }


    bool DualDepthPeeling::peeling_done() const
    {
        return current_peel_ >= max_peels_ ||
            num_written_pixels_ <= occlusion_threshold_;
    }


    void DualDepthPeeling::start_occlusion_query() const
    {
        // ES 3.0 only supports checking if *any* samples passed. We'll just use
        // that query to stop peeling once all frags are processed, and ignore the
        // requested occlusion ratio.
    #ifdef GL_ES_VERSION_3_0
        glBeginQuery(GL_ANY_SAMPLES_PASSED, occlusion_query_Id_);
    #else  // GL_ES_VERSION_3_0
        glBeginQuery(GL_SAMPLES_PASSED, occlusion_query_Id_);
    #endif // GL_ES_VERSION_3_0
    }


    void DualDepthPeeling::end_occlusion_query()
    {
    #ifdef GL_ES_VERSION_3_0
        glEndQuery(GL_ANY_SAMPLES_PASSED);
        GLuint anySamplesPassed;
        glGetQueryObjectuiv(occlusion_query_Id_, GL_QUERY_RESULT, &anySamplesPassed);
        num_written_pixels_ = anySamplesPassed ? occlusion_threshold_ + 1 : 0;
    #else  // GL_ES_VERSION_3_0
        glEndQuery(GL_SAMPLES_PASSED);
        glGetQueryObjectuiv(occlusion_query_Id_, GL_QUERY_RESULT, &num_written_pixels_);
    #endif // GL_ES_VERSION_3_0
    }


    void DualDepthPeeling::delete_occlusion_query_id()
    {
        glDeleteQueries(1, &occlusion_query_Id_);
    }


    void DualDepthPeeling::geometry_pass(const std::vector<TrianglesDrawable*>& surfaces) {
        if (stage_ == DDP_InitDepth) {
            static const std::string name = "transparency/dual_depth_peeling_init";
            ShaderProgram* program = ShaderManager::get_program(name);
            if (!program) {
                std::vector<ShaderProgram::Attribute> attributes = {
                    ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position")
                };
                program = ShaderManager::create_program_from_files(name, attributes);
            }
            if (!program)
                return;

            program->bind();
            const mat4& MVP = camera_->modelViewProjectionMatrix();
            program->set_uniform("MVP", MVP);
            for (auto d : surfaces) {
                if (d->is_visible()) {
                    program->set_uniform("MANIP", d->manipulated_matrix());
                    ClippingPlane::instance()->set_program(program);
                    ClippingPlane::instance()->set_discard_primitives(program, d->plane_clip_discard_primitive());
                    d->gl_draw();
                }
            }
            program->release();
        }
        else if (stage_ == DDP_Peel) {
            static const std::string name = "transparency/dual_depth_peeling_peel_rendering";
            ShaderProgram* program = ShaderManager::get_program(name);
            if (!program) {
                std::vector<ShaderProgram::Attribute> attributes = {
                        ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"),
                        ShaderProgram::Attribute(ShaderProgram::TEXCOORD, "vtx_texcoord"),
                        ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"),
                        ShaderProgram::Attribute(ShaderProgram::NORMAL, "vtx_normal")
                };
                std::vector<std::string> outputs;
                outputs.emplace_back("fragOutput0");
                outputs.emplace_back("fragOutput1");
                outputs.emplace_back("fragOutput2");
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
            program->bind_texture("DepthBlenderTex", fbo_->color_texture(depth_source_), 0);
            program->bind_texture("FrontBlenderTex", fbo_->color_texture(front_source_), 1);
            for (auto d : surfaces) {
                if (d->is_visible()) {
                    // transformation introduced by manipulation
                    const mat4 MANIP = d->manipulated_matrix();
                    // needs be padded when using uniform blocks
                    const mat3 NORMAL = transform::normal_matrix(MANIP);
                    program->set_uniform("MANIP", MANIP)
                            ->set_uniform( "NORMAL", NORMAL)
                            ->set_uniform("lighting", d->lighting())
                            ->set_uniform("smooth_shading", d->smooth_shading())
                            ->set_uniform("two_sides_lighting", d->lighting_two_sides())
                            ->set_uniform("distinct_back_color", d->distinct_back_color())
                            ->set_uniform("backside_color", d->back_color())
                            ->set_block_uniform("Material", "ambient", d->material().ambient)
                            ->set_block_uniform("Material", "specular", d->material().specular)
                            ->set_block_uniform("Material", "shininess", &d->material().shininess)
                            ->set_uniform("Alpha", d->opacity())
                            ->set_uniform("per_vertex_color", d->coloring_method() != State::UNIFORM_COLOR && d->color_buffer())
                            ->set_uniform("default_color", d->color())
                            ->set_uniform("selected", d->is_selected())
                            ->set_uniform("highlight_color", setting::highlight_color);

                    ClippingPlane::instance()->set_program(program);
                    ClippingPlane::instance()->set_discard_primitives(program, d->plane_clip_discard_primitive());

                    bool use_texture = (d->texture() && (d->coloring_method() == State::SCALAR_FIELD || d->coloring_method() == State::TEXTURED));
                    program->set_uniform("use_texture", use_texture);
                    if (use_texture)
                        program->bind_texture("textureID", d->texture()->id(), 2)
                                ->set_uniform("texture_repeat", d->texture_repeat())
                                ->set_uniform("fractional_repeat", d->texture_fractional_repeat());
                    d->gl_draw();
                    if (use_texture)
                        program->release_texture();
                }
            }
            program->release_texture();
            program->release();   easy3d_debug_log_gl_error
        }

        ++num_geom_passes_;
    }


    void DualDepthPeeling::init_depth_buffers(const std::vector<TrianglesDrawable*>& surfaces) {
        fbo_->bind();

        // This two render targets store the front and back colors
        // Clear to 0.0 and use MAX blending to filter written color
        // At most one front color and one back color can be written every pass
        // The source front buffer must be initialized, since it simply uses additive blending.
        // The back-blending may discard fragments, so the back peel accumulator needs
        // initialization as well.
        unsigned int targets[2] = {
            static_cast<unsigned int>(Back),
            static_cast<unsigned int>(front_source_)
        };
        fbo_->activate_draw_buffers(2, targets);	easy3d_debug_log_gl_error
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        // Render target depth_source_ stores (-minDepth, maxDepth, alphaMultiplier)
        fbo_->activate_draw_buffer(depth_source_);	easy3d_debug_log_gl_error
        glClearColor(-1, -1, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);
        glBlendEquation(GL_MAX);

        geometry_pass(surfaces);

    #ifdef SAVE_INTERMEDIATE_FBO
        fbo_->snapshot_color_ppm(depth_source_, "snapshotBuffers/01.ppm");
    #endif
    }


    void DualDepthPeeling::draw(const std::vector<TrianglesDrawable*>& surfaces) {
        if (surfaces.empty())
            return;

        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        width_ = viewport[2];
        height_ = viewport[3];
        init(width_, height_);

        //////////////////////////////////////////////////////////////////////////

        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);

        // ---------------------------------------------------------------------
        // 1. Initialize Min-Max Depth Buffer
        // ---------------------------------------------------------------------

        stage_ = DDP_InitDepth;
        init_depth_buffers(surfaces);

        // ---------------------------------------------------------------------
        // 2. Dual Depth Peeling + Blending
        // ---------------------------------------------------------------------

        stage_ = DDP_Peel;

        // Since we cannot blend the back colors in the geometry passes,
        // we use another render target to do the alpha blending
        fbo_->activate_draw_buffer(Back);	easy3d_debug_log_gl_error
        glClearColor(bkg_color_[0], bkg_color_[1], bkg_color_[2], 0);
        glClear(GL_COLOR_BUFFER_BIT);

        while (!peeling_done()) {
            peel(surfaces);
            swap_targets();
        }

        fbo_->release();

        // ---------------------------------------------------------------------
        // 3. Final Pass
        // ---------------------------------------------------------------------

        stage_ = DDP_Blend;
        blend_final_image();

        //////////////////////////////////////////////////////////////////////////

        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        delete_occlusion_query_id();
        stage_ = DDP_Inactive;

    #ifdef SHOW_DEBUG_INFO
        int w = fbo_->width();
        int h = fbo_->height();
        LOG(INFO) << "Depth peel done:"
            << "  - Number of peels: " << current_peel_
            << "  - Number of geometry passes: " << num_geom_passes_
            << "  - Occlusion Ratio: " << static_cast<float>(num_written_pixels_) / static_cast<float>(w * h);;
    #endif
    }


    void DualDepthPeeling::peel(const std::vector<TrianglesDrawable*>& surfaces) {
        // Initialize destination buffers to their minima, since we're MAX blending,
        // this ensures that valid outputs are captured.
        unsigned int destColorBuffers[2] = {
            static_cast<unsigned int>(front_destination_),
            static_cast<unsigned int>(BackTemp)
        };
        fbo_->activate_draw_buffers(2, destColorBuffers);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        fbo_->activate_draw_buffer(depth_destination_);
        glClearColor(-1, -1, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        unsigned int targets[3] = {
            static_cast<unsigned int>(depth_destination_),	//  RG32F MAX blending
            static_cast<unsigned int>(front_destination_),	//	RGBA MAX blending
            static_cast<unsigned int>(BackTemp)				//	RGBA MAX blending
        };
        fbo_->activate_draw_buffers(3, targets);
        glBlendEquation(GL_MAX);

        geometry_pass(surfaces);

    #ifdef SAVE_INTERMEDIATE_FBO
        fbo_->snapshot_color_ppm(depth_destination_, "snapshotBuffers/02.ppm");
        fbo_->snapshot_color_ppm(front_destination_, "snapshotBuffers/03.ppm");
        fbo_->snapshot_color_ppm(BackTemp, "snapshotBuffers/04.ppm");
    #endif

        // Full screen pass to alpha-blend the back color
        fbo_->activate_draw_buffer(Back);

        glBlendEquation(GL_FUNC_ADD);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        if (use_occlusion_query_)
            start_occlusion_query();

        static const std::string name = "transparency/dual_depth_peeling_blend";
        ShaderProgram* program = ShaderManager::get_program(name);
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes =  {
                ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"),
                ShaderProgram::Attribute(ShaderProgram::TEXCOORD, "vtx_texcoord")
            };
            program = ShaderManager::create_program_from_files(name, attributes);
        }
        if (!program)
            return;

        program->bind();
        program->bind_texture("TempTex", fbo_->color_texture(BackTemp), 0);
        shape::draw_full_screen_quad(ShaderProgram::POSITION, ShaderProgram::TEXCOORD, 0.0f);
        program->release_texture();
        program->release();

    #ifdef SAVE_INTERMEDIATE_FBO
        fbo_->snapshot_color_ppm(Back, "snapshotBuffers/05.ppm");
    #endif

        if (use_occlusion_query_) {
            end_occlusion_query();
            //LOG(INFO) << "samples written: " << num_written_pixels_;
        }

        ++current_peel_;
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

        // restore the default background color
        glClearColor(bkg_color_[0], bkg_color_[1], bkg_color_[2], bkg_color_[3]);

        program->bind();
        program->bind_texture("FrontBlenderTex", fbo_->color_texture(front_source_), 0);
        program->bind_texture("BackBlenderTex", fbo_->color_texture(Back), 1);
        shape::draw_full_screen_quad(ShaderProgram::POSITION, ShaderProgram::TEXCOORD, 0.0f);
        program->release_texture();
        program->release();
    }


} // namespace easy3d
