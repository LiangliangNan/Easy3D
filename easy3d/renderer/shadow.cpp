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

#include <easy3d/renderer/shadow.h>
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/framebuffer_object.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/opengl_error.h>
#include <easy3d/renderer/transform.h>
#include <easy3d/renderer/frustum.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/util/setting.h>

// for debugging
//#define SHOW_SHADOW_MAP_AND_LIGHT_FRUSTUM

#ifdef SHOW_SHADOW_MAP_AND_LIGHT_FRUSTUM
#include <easy3d/renderer/shape.h>
#endif


namespace easy3d {

    Shadow::Shadow(Camera* cam)
        : camera_(cam)
        , fbo_(nullptr)
        , camera_frustum_(nullptr)
        , light_frustum_(nullptr)
        , shadow_map_size_(1024)
        , virtual_background_(true)
        , virtual_background_color_(setting::background_color)
        , virtual_background_drawable_(nullptr)
        , light_distance_(setting::effect_shadow_light_distance)
        , darkness_(setting::effect_shadow_darkness)
    {
    }


    Shadow::~Shadow()
    {
        clear();
    }


    void Shadow::clear() {
        delete fbo_;
        fbo_ = nullptr;

        delete camera_frustum_;
        camera_frustum_ = nullptr;

        delete light_frustum_;
        light_frustum_ = nullptr;

        delete virtual_background_drawable_;
        virtual_background_drawable_ = nullptr;
    }


    void Shadow::set_light_distance(float dist) {
        // 1.0 corresponds to degenerated light frustum
        light_distance_ = std::max(1.1f, dist);
    }


    void Shadow::set_darkness(float darkness) {
        darkness_ = darkness;
        if (darkness_ < 0.0f)
            darkness_ = 0.0f;
        if (darkness_ > 1.0f)
            darkness_ = 1.0f;
    }


    void Shadow::ensure_fbo() {
        if (!fbo_) {
            const int samples = 0;
            fbo_ = new FramebufferObject(shadow_map_size_, shadow_map_size_, samples);
            fbo_->add_depth_texture(GL_DEPTH_COMPONENT32F, GL_LINEAR, GL_COMPARE_REF_TO_TEXTURE, GL_LEQUAL);
        }
        fbo_->ensure_size(shadow_map_size_, shadow_map_size_);
    }


    void Shadow::init()
    {
        ensure_fbo();

        compute_camera_frustum();

        compute_light_frustum();

        if (virtual_background_)
            update_virtual_background();
        else {
            // in perspective mode, the background is managed by the user
        }

        light_view_matrix_ = light_frustum_->view_matrix();
        light_projection_matrix_ = light_frustum_->projection_matrix();

        // Multiplying the vertex's position by light MVP (i.e., light_projection_matrix_ * light_view_matrix_) will
        // give homogeneous coordinates, which are in [-1,1] ; but texture sampling must be done in [0,1]. This can
        // be fixed by tweaking the fetch coordinates directly in the fragment shader, but it's more efficient to
        // multiply the homogeneous coordinates by the following matrix, which simply divides  coordinates by 2
        // ( the diagonal: [-1,1] -> [-0.5, 0.5] ) and translates them ( the lower row: [-0.5, 0.5] -> [0,1] ).
        const mat4 clip2Tex(
            vec4(0.5f, 0.0f, 0.0f, 0.0f),
            vec4(0.0f, 0.5f, 0.0f, 0.0f),
            vec4(0.0f, 0.0f, 0.5f, 0.0f),
            vec4(0.5f, 0.5f, 0.5f, 1.0f)
        );
        shadow_matrix_ = clip2Tex * light_projection_matrix_ * light_view_matrix_;
    }


    void Shadow::draw(const std::vector<TrianglesDrawable*>& surfaces)
    {
        int viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        const int w = viewport[2];
        const int h = viewport[3];

        init();

        // generate shadow map
        glViewport(0, 0, shadow_map_size_, shadow_map_size_);
        shadow_map_pass(surfaces);          easy3d_debug_log_gl_error

        // rendering
        glViewport(0, 0, w, h);
        render_pass(surfaces);      easy3d_debug_log_gl_error

        // for debugging
    #ifdef SHOW_SHADOW_MAP_AND_LIGHT_FRUSTUM
        const float dpi_scale = static_cast<float>(w) / camera_->screenWidth();
        const float offset_x = 20.0f;
        const float offset_y = 40.0f;
        const float size = 200.0f;
        const Rect quad(offset_x * dpi_scale, (offset_x + size) * dpi_scale, offset_y * dpi_scale, (offset_y + size) * dpi_scale);
        shape::draw_depth_texture(quad, fbo_->depth_texture(), w, h, -0.9f);
        shape::draw_quad_wire(quad, vec4(1.0f, 0.0f, 0.0f, 1.0f), w, h, -0.99f);   easy3d_debug_log_gl_error
        draw_light_frustum();		easy3d_debug_log_gl_error
    #endif
    }


    void Shadow::shadow_map_pass(const std::vector<TrianglesDrawable*>& surfaces)
    {
        static const std::string name = "shadow/shadow_generate";
        ShaderProgram* program = ShaderManager::get_program(name);
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes;
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
            program = ShaderManager::create_program_from_files(name, attributes, std::vector<std::string>(), false);
        }
        if (!program)
            return;

        fbo_->bind();
        fbo_->deactivate_draw_buffers();
        glClear(GL_DEPTH_BUFFER_BIT);
        program->bind();
        program->set_uniform("MVP", light_projection_matrix_ * light_view_matrix_);	easy3d_debug_log_gl_error
        for (auto d : surfaces) {
            if (d->is_visible()) {
                ClippingPlane::instance()->set_program(program);
                ClippingPlane::instance()->set_discard_primitives(program, d->plane_clip_discard_primitive());
                d->gl_draw();
            }
        }

        // We can ignore background (background only receives shadows)
        //background_->draw(false);

        program->release();
        fbo_->release();
    }


    void Shadow::render_pass(const std::vector<TrianglesDrawable*>& surfaces) {
        static const std::string name = "shadow/shadow_rendering";
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

        const mat4& MVP = camera_->modelViewProjectionMatrix();
        // camera position is defined in world coordinate system.
        const vec3& wCamPos = camera_->position();

        program->bind();
        program->set_uniform("MVP", MVP);
        program->set_uniform("SHADOW", shadow_matrix_);                    easy3d_debug_log_gl_error
        program->set_uniform("wLightPos", light_pos_);                     easy3d_debug_log_gl_error
        program->set_uniform("wCamPos", wCamPos);                          easy3d_debug_log_gl_error
        program->set_uniform("darkness", darkness_);                       easy3d_debug_log_gl_error
        program->bind_texture("shadowMap", fbo_->depth_texture(), 0);      easy3d_debug_log_gl_error
        for (auto d : surfaces) {
            if (d->is_visible()) {
                // transformation introduced by manipulation
                const mat4 MANIP = d->manipulated_matrix();
                // needs be padded when using uniform blocks
                const mat3 NORMAL = transform::normal_matrix(MANIP);
                program->set_uniform("MANIP", MANIP)
                        ->set_uniform( "NORMAL", NORMAL)
                        ->set_uniform("smooth_shading", d->smooth_shading())
                        ->set_block_uniform("Material", "ambient", d->material().ambient)
                        ->set_block_uniform("Material", "specular", d->material().specular)
                        ->set_block_uniform("Material", "shininess", &d->material().shininess)
                        ->set_uniform("default_color", d->color())
                        ->set_uniform("per_vertex_color", d->coloring_method() != State::UNIFORM_COLOR && d->color_buffer())
                        ->set_uniform("is_background", false)
                        ->set_uniform("selected", d->is_selected())
                        ->set_uniform("highlight_color", setting::highlight_color);

                ClippingPlane::instance()->set_program(program);
                ClippingPlane::instance()->set_discard_primitives(program, d->plane_clip_discard_primitive());

                d->gl_draw();
            }
        }

        // draw the background plane
        program->set_uniform("default_color", virtual_background_color_);				easy3d_debug_log_gl_error
        program->set_uniform("per_vertex_color", false);
        program->set_uniform("is_background", true);
        virtual_background_drawable_->gl_draw();

        program->release_texture();
        program->release();

        // draw ground plane here...
    }


    void Shadow::compute_camera_frustum() {
        if (!camera_frustum_)
            camera_frustum_ = new Frustum(Frustum::PERSPECTIVE);
        // NOTE: fov is in radians and the 0.2f factor is important because we might get artifacts at the screen borders.
        auto fov = static_cast<float>(camera_->fieldOfView()) + 0.2f;
        auto ar = static_cast<float>(camera_->aspectRatio());
        auto znear = static_cast<float>(camera_->zNear());
        auto zfar = static_cast<float>(camera_->zFar());
        camera_frustum_->set_perspective(fov, ar, znear, zfar);
        const vec3& pos = camera_->position();
        const vec3& at = camera_->position() + camera_->viewDirection();
        const vec3& up = camera_->upVector();
        camera_frustum_->orient(pos, at, up);
    }


    void Shadow::compute_light_frustum() {
        //--------- the light's view matrix ---------------------------

        if (!light_frustum_)
            light_frustum_ = new Frustum(Frustum::PERSPECTIVE);

		const vec3& at = camera_->sceneCenter();
        vec3 dir = -setting::light_position.xyz();
        const mat3& trans = transform::normal_matrix(inverse(camera_->modelViewMatrix()));
        dir = normalize(trans * dir); // the light's direction in WC.
        float radius = camera_->sceneRadius();
        light_pos_ = at - dir * radius * light_distance_;
        const vec3& up = trans * vec3(0, 1, 0);
        light_frustum_->orient(light_pos_, at, up);

        //----------- determine the Z range --------------------------

        float maxZ = -FLT_MAX;
        float minZ = FLT_MAX;
        const mat4& light_view = light_frustum_->view_matrix();

        // make sure all relevant shadow casters are included
        vec3 p = light_view * at;
        if (p.z + radius > maxZ)
            maxZ = p.z + radius;  // maxZ is related to near plane
        if (p.z - radius < minZ)
            minZ = p.z - radius;  // not necessary for including the camera frustum

        if (!virtual_background_) {
            // The light frustum at its far end should contain the camera's frustum, and
            // at its near end should contain all relevant shadow casters. This makes
            // sure that all visible regions in the camera view will get right shadows.
            const std::vector<vec3>& points = camera_frustum_->vertices();

            // note that only the z-component is need and thus the multiplication can be simplified
            //p.z = view[2] * points[0].x + view[6] * points[0].y + view[10] * points[0].z + view[14];
            for (int i = 0; i < 8; i++) {
                const vec4& q = light_view * vec4(points[i], 1.0f);
                //if (q.z > maxZ) maxZ = q.z;  // not necessary for including the camera frustum
                if (q.z < minZ) minZ = q.z;  // minZ is related to far plane
            }
        }

         // note the inversion because the light looks at the neg. z axis
        float zNear = -maxZ;
        float zFar = -minZ;

        // Liangliang: Limit the Z range to improve the z-buffer precision
        //      (maybe slightly, but I didn't notice the difference).
        zFar = std::min(zFar, zNear + radius * 10.0f);

        //----------- determine the X and Y range --------------------------

        float xMax, yMax;
        xMax = yMax = -FLT_MAX;

        // set the projection matrix with the new z-bounds
        // gluPerspective(LIGHT_FOV, 1.0, maxZ, minZ); // for point lights
        light_frustum_->set_ortho(-1.0, 1.0, -1.0, 1.0, zNear, zFar);
        const mat4& shade_mvp = light_frustum_->projection_matrix() * light_view;

    #if 0
        // Liangliang: To determine the X and Y range, including the entire camera frustum will result in
        //			   large views (i.e. low shadow map resolution). I want the minimum affected region
        //			   to ensure the best resolution for the shadow map. Other regions in the camera frustum
        //             will have their projections out of the shadow map and thus cannot be in the shadow
        //             (this can be very easily checked in fragment shader).
        const std::vector<vec3>& points = camera_frustum_->vertices();
        for (int i = 0; i < 8; i++) {
            p = shade_mvp * points[i];
            if (p.x > xMax) xMax = p.x;
            if (p.x < xMin) xMin = p.x;
            if (p.y > yMax) yMax = p.y;
            if (p.y < yMin) yMin = p.y;
        }
    #endif

        // find the extends of all shadow casters' bbox as projected in light's homogeneous coordinates
        p = shade_mvp * at;
        if (p.x + radius > xMax) xMax = p.x + radius;
        if (p.y + radius > yMax) yMax = p.y + radius;

        // make sure zNear is positive
        zNear = zNear > 0.001 ? zNear : 0.001f;
        // The light frustum needs to enclose the whole range in X and Y directions.
        light_frustum_->set_frustum(xMax*2.0f, yMax*2.0f, zNear, zFar);
    }


    void Shadow::update_virtual_background() {
        vec3 normal = vec3(setting::light_position); // the background's normal in VC.
        const mat3& trans = transform::normal_matrix(inverse(camera_->modelViewMatrix()));
        normal = normalize(trans * normal);		  // the background's normal in WC.
        float dist = camera_->sceneRadius() * 0.002f;
        const vec3& center = light_frustum_->far_center() + normal * dist;

        vec3 up = light_frustum_->up_vector();
        vec3 dx = cross(up, normal);
        float size = light_frustum_->far_width();
        dx = normalize(dx) * size;
        vec3 dy = normalize(up) * size;
        float tc = 1.0f;

        std::vector<vec3> vertices;
        std::vector<vec2> texcoords;
        std::vector<unsigned int> indices;
        vec3 a(center - dx - dy);	vertices.push_back(a);	texcoords.emplace_back(vec2(-tc, -tc));
        vec3 b(center + dx - dy);	vertices.push_back(b);	texcoords.emplace_back(vec2(+tc, -tc));
        vec3 c(center + dx + dy);	vertices.push_back(c);	texcoords.emplace_back(vec2(+tc, +tc));
        vec3 d(center - dx + dy);	vertices.push_back(d);	texcoords.emplace_back(vec2(-tc, +tc));
        indices.push_back(0); indices.push_back(1); indices.push_back(2); // I will render the quad as two triangles
        indices.push_back(0); indices.push_back(2); indices.push_back(3);

        std::vector<vec3> normals;
        normals.resize(4, normal);

        if (!virtual_background_drawable_)
            virtual_background_drawable_ = new TrianglesDrawable;
        virtual_background_drawable_->update_vertex_buffer(vertices);
        virtual_background_drawable_->update_texcoord_buffer(texcoords);
        virtual_background_drawable_->update_element_buffer(indices);
        virtual_background_drawable_->update_normal_buffer(normals);
    }


    void Shadow::draw_light_frustum() {
        vec3 normal = setting::light_position.xyz(); // the background's normal in VC.
        const mat3& trans = transform::normal_matrix(inverse(camera_->modelViewMatrix()));
        normal = normalize(trans * normal);		  // the background's normal in WC.
        float dist = camera_->sceneRadius() * 0.004f;
        const vec3& offset = normal * dist;

        std::vector<vec3> points = light_frustum_->vertices();
        points[4] += offset;
        points[5] += offset;
        points[6] += offset;
        points[7] += offset;
        points.push_back(light_pos_); // 8th point

        static const std::vector<unsigned int> indices {
            0, 1, 1, 2, 2, 3, 3, 0, // near
            4, 5, 5, 6, 6, 7, 7, 4, // far
            4, 8, 5, 8, 6, 8, 7, 8  // side
        };

        LinesDrawable frustum("frustum");
        frustum.update_vertex_buffer(points);
        frustum.update_element_buffer(indices);
        frustum.set_uniform_coloring(vec4(0.0, 0.0, 1.0, 1.0));
        frustum.draw(camera_);
    }

}
