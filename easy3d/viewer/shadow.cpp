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


#include <easy3d/viewer/shadow.h>
#include <easy3d/viewer/shader_manager.h>
#include <easy3d/viewer/shader_program.h>
#include <easy3d/viewer/framebuffer_object.h>
#include <easy3d/viewer/camera.h>
#include <easy3d/viewer/opengl_error.h>
#include <easy3d/viewer/transform.h>
#include <easy3d/viewer/primitives.h>
#include <easy3d/viewer/frustum.h>
#include <easy3d/viewer/drawable.h>
#include <easy3d/viewer/setting.h>


//#define SHOW_SHADOW_MAP_AND_LIGHT_FRUSTUM

namespace easy3d {

    Shadow::Shadow(Camera* cam)
        : camera_(cam)
        , fbo_(nullptr)
        , camera_frustum_(nullptr)
        , light_frustum_(nullptr)
        , shadow_map_size_(1024)
        , orhto_background_(false)
        , light_distance_(50.0f)
        , darkness_(0.6f)
    {
    }


    Shadow::~Shadow()
    {
        clear();
    }


    void Shadow::clear()
    {
        if (fbo_) {
            delete fbo_;
            fbo_ = nullptr;
        }

        if (camera_frustum_) {
            delete camera_frustum_;
            camera_frustum_ = nullptr;
        }

        if (light_frustum_) {
            delete light_frustum_;
            light_frustum_ = nullptr;
        }
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

        if (orhto_background_)
            update_ortho_background();
        else {
            // in perspective mode, the background is managed by the user
        }

        light_view_matrix_ = light_frustum_->view_matrix();
        light_projection_matrix_ = light_frustum_->projection_matrix();

        // Multiplying the vertex’ position by light MVP (i.e., light_projection_matrix_ * light_view_matrix_) will
        // give homogeneous coordinates, which are in [-1,1] ; but texture sampling must be done in [0,1]. This can
        // be fixed by tweaking the fetch coordinates directly in the fragment shader but it’s more efficient to
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
        int w = viewport[2];
        int h = viewport[3];

        init();

        // generate shadow map

        glViewport(0, 0, shadow_map_size_, shadow_map_size_);
        shadow_map_pass(surfaces);          easy3d_debug_gl_error;

        // rendering

        glViewport(0, 0, w, h);
        render_pass(surfaces);      easy3d_debug_gl_error;

    #ifdef SHOW_SHADOW_MAP_AND_LIGHT_FRUSTUM
        draw_shadow_map(w, h);      easy3d_debug_gl_error;
        draw_light_frustum();		easy3d_debug_gl_error;
    #endif
    }


    void Shadow::shadow_map_pass(const std::vector<TrianglesDrawable*>& surfaces)
    {
        static const std::string name = "shadow/shadow_generate";
        ShaderProgram* program = ShaderManager::get_program(name);
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes;
            attributes.push_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
            std::vector<std::string> outputs;
            program = ShaderManager::create_program_from_files(name, attributes, outputs, false);
        }
        if (!program)
            return;

        fbo_->bind();
        fbo_->deactivate_draw_buffers();
        glClear(GL_DEPTH_BUFFER_BIT);
        program->bind();
        program->set_uniform("MVP", light_projection_matrix_ * light_view_matrix_);	easy3d_debug_gl_error;
        for (auto d : surfaces) {
            if (d->is_visible()) {
                d->draw(false);
            }
        }
        program->release();
        fbo_->release();
    }


    void Shadow::render_pass(const std::vector<TrianglesDrawable*>& surfaces) {
        static const std::string name = "shadow/shadow_rendering";
        ShaderProgram* program = ShaderManager::get_program(name);
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes = {
                ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"),
                ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"),
                ShaderProgram::Attribute(ShaderProgram::NORMAL, "vtx_normal")
            };
            std::vector<std::string> outputs;
            program = ShaderManager::create_program_from_files(name, attributes, outputs, false);
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
        program->set_uniform("darkness", darkness_);                       easy3d_debug_gl_error;
        program->bind_texture("shadowMap", fbo_->depth_texture(), 0);      easy3d_debug_gl_error;
        for (auto d : surfaces) {
            if (d->is_visible()) {
                program->set_uniform("default_color", d->default_color());				easy3d_debug_gl_error;
                program->set_uniform("per_vertex_color", d->per_vertex_color() && d->color_buffer());
                d->draw(false);
            }
        }
        program->release_texture();
        program->release();

        // draw ground plane here...
    }


    void Shadow::compute_camera_frustum() {
        if (!camera_frustum_)
            camera_frustum_ = new Frustum(Frustum::PERSPECTIVE);
        // NOTE: fov is in radians and the 0.2f factor is important because we might get artifacts at the screen borders.
        float fov = static_cast<float>(camera_->fieldOfView()) + 0.2f;
        float ar = static_cast<float>(camera_->aspectRatio());
        float neard = static_cast<float>(camera_->zNear());
        float fard = static_cast<float>(camera_->zFar());
        camera_frustum_->set_perspective(fov, ar, neard, fard);
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
        vec3 dir = -setting::light_position;
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

        if (!orhto_background_) {
            // The light frustum at its far end should contain the camera's frustum, and
            // at its near end should contain all relevant shadow casters. This makes
            // sure that all visible regions in the camera view will get right shadows.
            const std::vector<vec3>& points = camera_frustum_->vertices();

            // note that only the z-component is need and thus the multiplication can be simplified
            //p.z = view[2] * points[0].x + view[6] * points[0].y + view[10] * points[0].z + view[14];
            for (int i = 0; i < 8; i++) {
                const vec4& p = light_view * vec4(points[i], 1.0f);
                //if (p.z > maxZ) maxZ = p.z;  // not necessary for including the camera frustum
                if (p.z < minZ) minZ = p.z;  // minZ is related to far plane
            }
        }

         // note the inversion because the light looks at the neg. z axis
        float zNear = -maxZ;
        float zFar = -minZ;

        // Liangliang: Limit the Z range so as to improves the z-buffer precision
        //             (maybe slightly, but I didn't notice the difference).
        zFar = std::min(zFar, zNear + radius * 10.0f);

        //----------- determine the X and Y range --------------------------

        float xMin, xMax, yMin, yMax;
        xMin = yMin = FLT_MAX;
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
        if (p.x - radius < xMin) xMin = p.x - radius;
        if (p.y + radius > yMax) yMax = p.y + radius;
        if (p.y - radius < yMin) yMin = p.y - radius;

        // make sure zNear is positive
        zNear = zNear > 0.001 ? zNear : 0.001f;
        // The light frustum needs to enclose the whole range in X and Y directions.
        light_frustum_->set_frustum(xMax*2.0f, yMax*2.0f, zNear, zFar);
    }


    void Shadow::update_ortho_background() {
//        vec3 normal = RenderingEngine::light_position(); // the background's normal in VC.
//        const mat3& trans = opengl::normal_matrix(inverse(camera_->modelViewMatrix()));
//        normal = normalize(trans * normal);		  // the background's normal in WC.
//        float dist = scene_bbox_.diagonal() * 0.001f;
//        const vec3& center = light_frustum_->far_center() + normal * dist;

//        RenderingEngine::ground_plane()->set_center(center);
//        RenderingEngine::ground_plane()->set_normal(normal);
//        RenderingEngine::ground_plane()->set_up_vector(light_frustum_->up_vector());
//        RenderingEngine::ground_plane()->set_size(light_frustum_->far_width());
    }


    void Shadow::draw_light_frustum() {
        ShaderProgram* program = ShaderManager::get_program("lines_color");
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes = {
                ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"),
                ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color")
            };
            program = ShaderManager::create_program_from_files("lines_color", attributes);
        }
        if (!program)
          return;

        vec3 normal = setting::light_position; // the background's normal in VC.
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

        static const unsigned int indices[24] = {
            0, 1, 1, 2, 2, 3, 3, 0, // near
            4, 5, 5, 6, 6, 7, 7, 4, // far
            4, 8, 5, 8, 6, 8, 7, 8  // side
        };

        LinesDrawable frustum("frustum");
        frustum.update_vertex_buffer(points);
        frustum.update_index_buffer(indices, 24);

        program->bind();
        program->set_uniform("MVP", camera_->modelViewProjectionMatrix());
        program->set_uniform("per_vertex_color", false);
        program->set_uniform("default_color", vec3(1.0, 0.0, 0.0));
        frustum.draw(false);
        program->release();
    }


    void Shadow::draw_shadow_map(int w, int h) {
        static const std::string quad_name = "screen_space/textured_quad";
        ShaderProgram* program = ShaderManager::get_program(quad_name);
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes = {
                ShaderProgram::Attribute(ShaderProgram::POSITION, "vertexMC"),
                ShaderProgram::Attribute(ShaderProgram::TEXCOORD, "tcoordMC")
            };
            std::vector<std::string> outputs;
            program = ShaderManager::create_program_from_files(quad_name, attributes, outputs, false);
        }
		if (!program)
			return;

        program->bind();			easy3d_debug_gl_error;
        program->bind_texture("textureID", fbo_->depth_texture(), 0);
        opengl::draw_quad(ShaderProgram::POSITION, ShaderProgram::TEXCOORD, 200, 10, 200, 200, w, h, -0.9f); easy3d_debug_gl_error;
        program->release_texture();
        program->release();		easy3d_debug_gl_error;

        // draw the boundary
        static const std::string wire_name = "screen_space/lines_color";
        program = ShaderManager::get_program(wire_name);
		if (!program) {
            std::vector<ShaderProgram::Attribute> attributes = {
                ShaderProgram::Attribute(ShaderProgram::POSITION, "vertexMC"),
                ShaderProgram::Attribute(ShaderProgram::COLOR, "vertexColor")
            };
            program = ShaderManager::create_program_from_files(wire_name, attributes);
		}
		if (!program)
			return; 

		program->bind();		easy3d_debug_gl_error;
		program->set_uniform("per_vertex_color", false);
		program->set_uniform("default_color", vec3(0.0f, 0.0f, 0.0f));
        opengl::draw_quad_wire(ShaderProgram::POSITION, 200, 10, 200, 200, w, h, -1.0f); easy3d_debug_gl_error;
        program->release();
    }

}
