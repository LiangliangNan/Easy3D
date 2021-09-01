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

#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/texture_manager.h>
#include <easy3d/renderer/setting.h>
#include <easy3d/renderer/opengl.h>
#include <easy3d/renderer/opengl_error.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/renderer/transform.h>
#include <easy3d/util/logging.h>


namespace easy3d {

    PointsDrawable::PointsDrawable(const std::string &name /*= ""*/, Model* model)
            : Drawable(name, model), point_size_(2.0f), impostor_type_(PLAIN)
    {
        lighting_two_sides_ = setting::points_drawable_two_side_lighting;
        distinct_back_color_ = setting::points_drawable_distinct_backside_color;
        back_color_ = setting::points_drawable_backside_color;
        set_uniform_coloring(vec4(0.0f, 1.0f, 0.0f, 1.0f));
    }


    Drawable::Type PointsDrawable::type() const {
        return DT_POINTS;
    }


    void PointsDrawable::draw(const Camera *camera /* = false */) const {
        if (update_needed_ || vertex_buffer_ == 0)
            const_cast<PointsDrawable*>(this)->internal_update_buffers();

        switch (impostor_type_) {
            case PLAIN:
                if (texture() && (coloring_method() == State::SCALAR_FIELD || coloring_method() == State::TEXTURED))
                    _draw_plain_points_with_texture(camera);
                else
                    _draw_plain_points(camera);
                break;

            case SPHERE:
#if 0
                if (texture() && (coloring_method() == State::SCALAR_FIELD || coloring_method() == State::TEXTURED))
                    _draw_spheres_with_texture_sprite(camera);
                else
                    _draw_spheres_sprite(camera);
#else
                if (texture() && (coloring_method() == State::SCALAR_FIELD || coloring_method() == State::TEXTURED))
                    _draw_spheres_with_texture_geometry(camera);
                else
                    _draw_spheres_geometry(camera);
#endif
                break;

            case SURFEL:
                if (texture() && (coloring_method() == State::SCALAR_FIELD || coloring_method() == State::TEXTURED))
                    _draw_surfels_with_texture(camera);
                else
                    _draw_surfels(camera);
                break;
        }
    }


    void PointsDrawable::_draw_plain_points(const Camera *camera) const {
        if (vertex_buffer() == 0) {
            LOG_N_TIMES(3, ERROR) << "drawable \'" << name() << "\': vertex buffer not created. " << COUNTER;
            return;
        }

        ShaderProgram *program = ShaderManager::get_program("points/points_plain_color");
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes;
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"));
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::NORMAL, "vtx_normal"));
            program = ShaderManager::create_program_from_files("points/points_plain_color", attributes);
        }

        if (!program)
            return;

        const mat4 &MVP = camera->modelViewProjectionMatrix();
        // camera position is defined in world coordinate system.
        const vec3 &wCamPos = camera->position();
        // it can also be computed as follows:
        //const vec3& wCamPos = invMV * vec4(0, 0, 0, 1);
        const mat4 &MV = camera->modelViewMatrix();
        const vec4 &wLightPos = inverse(MV) * setting::light_position;

        // transformation introduced by manipulation
        const mat4 MANIP = manipulated_matrix();
        // needs be padded when using uniform blocks
        const mat3 NORMAL = transform::normal_matrix(MANIP);

        glPointSize(point_size());

        program->bind();
        program->set_uniform("MVP", MVP)
                ->set_uniform("MANIP", MANIP)
                ->set_uniform( "NORMAL", NORMAL)
                ->set_uniform("lighting", normal_buffer() && lighting())
                ->set_uniform("two_sides_lighting",lighting_two_sides())
                ->set_uniform("wLightPos", wLightPos)
                ->set_uniform("wCamPos", wCamPos)
                ->set_uniform("per_vertex_color",coloring_method() != State::UNIFORM_COLOR && color_buffer())
                ->set_uniform("default_color",color())
                ->set_uniform("distinct_back_color",distinct_back_color())
                ->set_uniform("backside_color",back_color())
                ->set_block_uniform("Material", "ambient",material().ambient)
                ->set_block_uniform("Material", "specular",material().specular)
                ->set_block_uniform("Material", "shininess", &material().shininess);

        program->set_uniform("highlight",highlight())
                ->set_uniform("highlight_id_min",highlight_range().first)
                ->set_uniform("highlight_id_max",highlight_range().second);

        program->set_uniform("selected", is_selected());

        if (setting::clipping_plane)
            setting::clipping_plane->set_program(program);

        gl_draw();
        program->release();
    }


    void PointsDrawable::_draw_spheres_sprite(const Camera *camera) const {
        if (vertex_buffer() == 0) {
            LOG_N_TIMES(3, ERROR) << "drawable \'" << name() << "\': vertex buffer not created. " << COUNTER;
            return;
        }

        ShaderProgram *program = ShaderManager::get_program("points/points_spheres_sprite_color");
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes;
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"));
            program = ShaderManager::create_program_from_files("points/points_spheres_sprite_color", attributes);
        }

        if (!program)
            return;

        easy3d_debug_log_gl_error;
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE); // starting from GL3.2, using GL_PROGRAM_POINT_SIZE

        program->bind();

        program->set_uniform("perspective", camera->type() == Camera::PERSPECTIVE)
                ->set_uniform("MV", camera->modelViewMatrix())
                ->set_uniform("PROJ", camera->projectionMatrix())
                ->set_uniform("MANIP", manipulated_matrix())
                ->set_uniform("screen_width", camera->screenWidth());

        float ratio = camera->pixelGLRatio(camera->pivotPoint());
        program->set_uniform("sphere_radius", point_size() * ratio * 0.5f)  // 0.5f from size -> radius
                ->set_uniform("per_vertex_color",coloring_method() != State::UNIFORM_COLOR && color_buffer())
                ->set_uniform("default_color",color())
                ->set_uniform("eLightPos", setting::light_position)
                ->set_uniform("lighting",lighting());

        program->set_block_uniform("Material", "ambient",material().ambient)
                ->set_block_uniform("Material", "specular",material().specular)
                ->set_block_uniform("Material", "shininess", &material().shininess);

        program->set_uniform("highlight",highlight())
                ->set_uniform("highlight_id_min",highlight_range().first)
                ->set_uniform("highlight_id_max",highlight_range().second);

        program->set_uniform("selected", is_selected());

        if (setting::clipping_plane)
            setting::clipping_plane->set_program(program);

        gl_draw();
        program->release();

        glDisable(GL_VERTEX_PROGRAM_POINT_SIZE); // starting from GL3.2, using GL_PROGRAM_POINT_SIZE
    }


    void PointsDrawable::_draw_spheres_geometry(const Camera *camera) const {
        if (vertex_buffer() == 0) {
            LOG_N_TIMES(3, ERROR) << "drawable \'" << name() << "\': vertex buffer not created. " << COUNTER;
            return;
        }

        ShaderProgram *program = ShaderManager::get_program("points/points_spheres_geometry_color");
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes;
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"));
            program = ShaderManager::create_program_from_files("points/points_spheres_geometry_color", attributes,
                                                               std::vector<std::string>(), true);
        }
        if (!program)
            return;

        easy3d_debug_log_gl_error;

        program->bind();
        program->set_uniform("perspective", camera->type() == Camera::PERSPECTIVE)
                ->set_uniform("MV", camera->modelViewMatrix())
                ->set_uniform("PROJ", camera->projectionMatrix())
                ->set_uniform("MANIP", manipulated_matrix());

        float ratio = camera->pixelGLRatio(camera->pivotPoint());
        program->set_uniform("sphere_radius", point_size() * ratio * 0.5f)  // 0.5f from size -> radius
                ->set_uniform("per_vertex_color",coloring_method() != State::UNIFORM_COLOR && color_buffer())
                ->set_uniform("default_color",color())
                ->set_uniform("eLightPos", setting::light_position)
                ->set_uniform("lighting",lighting());

        program->set_block_uniform("Material", "ambient",material().ambient)
                ->set_block_uniform("Material", "specular",material().specular)
                ->set_block_uniform("Material", "shininess", &material().shininess);

        program->set_uniform("highlight", highlight())
                ->set_uniform("highlight_id_min", highlight_range().first)
                ->set_uniform("highlight_id_max", highlight_range().second);

        program->set_uniform("selected", is_selected());

        if (setting::clipping_plane)
            setting::clipping_plane->set_program(program);

        gl_draw();
        program->release();
    }


    void PointsDrawable::_draw_plain_points_with_texture(const Camera *camera) const {
        if (vertex_buffer() == 0) {
            LOG_N_TIMES(3, ERROR) << "drawable \'" << name() << "\': vertex buffer not created. " << COUNTER;
            return;
        }
        if (texcoord_buffer() == 0) {
            LOG_N_TIMES(3, ERROR) << "texcoord buffer not created. " << COUNTER;
            return;
        }

        ShaderProgram *program = ShaderManager::get_program("points/points_plain_texture");
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes;
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::TEXCOORD, "vtx_texcoord"));
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::NORMAL, "vtx_normal"));
            program = ShaderManager::create_program_from_files("points/points_plain_texture", attributes);
        }

        if (!program)
            return;

        const mat4 &MVP = camera->modelViewProjectionMatrix();
        // camera position is defined in world coordinate system.
        const vec3 &wCamPos = camera->position();
        // it can also be computed as follows:
        //const vec3& wCamPos = invMV * vec4(0, 0, 0, 1);
        const mat4 &MV = camera->modelViewMatrix();
        const vec4 &wLightPos = inverse(MV) * setting::light_position;

        // transformation introduced by manipulation
        const mat4 MANIP = manipulated_matrix();
        // needs be padded when using uniform blocks
        const mat3 NORMAL = transform::normal_matrix(MANIP);

        glPointSize(point_size());

        program->bind();
        program->set_uniform("MVP", MVP)
                ->set_uniform("MANIP", MANIP)
                ->set_uniform( "NORMAL", NORMAL)
                ->set_uniform("lighting", normal_buffer() && lighting())
                ->set_uniform("two_sides_lighting",lighting_two_sides())
                ->set_uniform("wLightPos", wLightPos)
                ->set_uniform("wCamPos", wCamPos)
                ->set_uniform("distinct_back_color",distinct_back_color())
                ->set_uniform("backside_color",back_color())
                ->set_block_uniform("Material", "ambient",material().ambient)
                ->set_block_uniform("Material", "specular",material().specular)
                ->set_block_uniform("Material", "shininess", &material().shininess)
                ->bind_texture("textureID",texture()->id(), 0);

        program->set_uniform("selected", is_selected());


        program->set_uniform("highlight",highlight())
                ->set_uniform("highlight_id_min",highlight_range().first)
                ->set_uniform("highlight_id_max",highlight_range().second);

        if (setting::clipping_plane)
            setting::clipping_plane->set_program(program);

        gl_draw();
        program->release_texture();

        program->release();
    }


    void PointsDrawable::_draw_spheres_with_texture_sprite(const Camera *camera) const {
        if (vertex_buffer() == 0) {
            LOG_N_TIMES(3, ERROR) << "drawable \'" << name() << "\': vertex buffer not created. " << COUNTER;
            return;
        }
        if (texcoord_buffer() == 0) {
            LOG_N_TIMES(3, ERROR) << "texcoord buffer not created. " << COUNTER;
            return;
        }

    }


    void PointsDrawable::_draw_spheres_with_texture_geometry(const Camera *camera) const {
        if (vertex_buffer() == 0) {
            LOG_N_TIMES(3, ERROR) << "drawable \'" << name() << "\': vertex buffer not created. " << COUNTER;
            return;
        }
        if (texcoord_buffer() == 0) {
            LOG_N_TIMES(3, ERROR) << "texcoord buffer not created. " << COUNTER;
            return;
        }

        ShaderProgram *program = ShaderManager::get_program("points/points_spheres_geometry_texture");
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes;
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::TEXCOORD, "vtx_texcoord"));
            program = ShaderManager::create_program_from_files("points/points_spheres_geometry_texture", attributes,
                                                               std::vector<std::string>(), true);
        }
        if (!program)
            return;

        easy3d_debug_log_gl_error;

        program->bind();
        program->set_uniform("perspective", camera->type() == Camera::PERSPECTIVE)
                ->set_uniform("MV", camera->modelViewMatrix())
                ->set_uniform("PROJ", camera->projectionMatrix())
                ->set_uniform("MANIP", manipulated_matrix());

        float ratio = camera->pixelGLRatio(camera->pivotPoint());
        program->set_uniform("sphere_radius", point_size() * ratio * 0.5f)  // 0.5f from size -> radius
                ->set_uniform("eLightPos", setting::light_position)
                ->set_uniform("lighting",lighting());

        program->set_block_uniform("Material", "ambient",material().ambient)
                ->set_block_uniform("Material", "specular",material().specular)
                ->set_block_uniform("Material", "shininess", &material().shininess);

        program->set_uniform("highlight",highlight())
                ->set_uniform("highlight_id_min",highlight_range().first)
                ->set_uniform("highlight_id_max",highlight_range().second);

        program->set_uniform("selected", is_selected());

        if (setting::clipping_plane)
            setting::clipping_plane->set_program(program);

        program->bind_texture("textureID",texture()->id(), 0);
        gl_draw();
        program->release_texture();

        program->release();
    }


    void PointsDrawable::_draw_surfels(const Camera *camera) const {
        if (vertex_buffer() == 0) {
            LOG_N_TIMES(3, ERROR) << "drawable \'" << name() << "\': vertex buffer not created. " << COUNTER;
            return;
        }
        if (normal_buffer() == 0) {
            LOG_N_TIMES(3, ERROR) << "normal buffer not created. " << COUNTER;
            return;
        }

        ShaderProgram *program = ShaderManager::get_program("points/points_surfel_color");
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes;
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::NORMAL, "vtx_normal"));
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"));
            program = ShaderManager::create_program_from_files("points/points_surfel_color", attributes,
                                                               std::vector<std::string>(), true);
        }
        if (!program)
            return;

        easy3d_debug_log_gl_error;

        const mat4 &MVP = camera->modelViewProjectionMatrix();
        // camera position is defined in world coordinate system.
        const vec3 &wCamPos = camera->position();
        const vec4 &wLightPos = inverse(camera->modelViewMatrix()) * setting::light_position;

        // transformation introduced by manipulation
        const mat4 MANIP = manipulated_matrix();
        // needs be padded when using uniform blocks
        const mat3 NORMAL = transform::normal_matrix(MANIP);

        program->bind();
        program->set_uniform("MVP", MVP)
                ->set_uniform("MANIP", MANIP)
                ->set_uniform( "NORMAL", NORMAL)
                ->set_uniform("per_vertex_color",coloring_method() != State::UNIFORM_COLOR && color_buffer())
                ->set_uniform("default_color",color());

        float ratio = camera->pixelGLRatio(camera->pivotPoint());
        program->set_uniform("radius", point_size() * ratio * 0.5f)  // 0.5f from size -> radius
                ->set_uniform("wLightPos", wLightPos)
                ->set_uniform("wCamPos", wCamPos)
                ->set_uniform("lighting",lighting())
                ->set_uniform("two_sides_lighting",lighting_two_sides())
                ->set_uniform("distinct_back_color",distinct_back_color())
                ->set_uniform("backside_color",back_color());

        program->set_uniform("highlight",highlight())
                ->set_uniform("highlight_id_min",highlight_range().first)
                ->set_uniform("highlight_id_max",highlight_range().second);

        program->set_block_uniform("Material", "ambient",material().ambient)
                ->set_block_uniform("Material", "specular",material().specular)
                ->set_block_uniform("Material", "shininess", &material().shininess);

        program->set_uniform("selected", is_selected());

        if (setting::clipping_plane)
            setting::clipping_plane->set_program(program);

        gl_draw();
        program->release();
    }


    void PointsDrawable::_draw_surfels_with_texture(const Camera *camera) const {
        if (vertex_buffer() == 0) {
            LOG_N_TIMES(3, ERROR) << "drawable \'" << name() << "\': vertex buffer not created. " << COUNTER;
            return;
        }
        if (normal_buffer() == 0) {
            LOG_N_TIMES(3, ERROR) << "normal buffer not created. " << COUNTER;
            return;
        }
        if (texcoord_buffer() == 0) {
            LOG_N_TIMES(3, ERROR) << "texcoord buffer not created. " << COUNTER;
            return;
        }

        ShaderProgram *program = ShaderManager::get_program("points/points_surfel_texture");
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes;
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::NORMAL, "vtx_normal"));
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::TEXCOORD, "vtx_texcoord"));
            program = ShaderManager::create_program_from_files("points/points_surfel_texture", attributes,
                                                               std::vector<std::string>(), true);
        }
        if (!program)
            return;

        easy3d_debug_log_gl_error;

        const mat4 &MVP = camera->modelViewProjectionMatrix();
        // camera position is defined in world coordinate system.
        const vec3 &wCamPos = camera->position();
        const vec4 &wLightPos = inverse(camera->modelViewMatrix()) * setting::light_position;

        // transformation introduced by manipulation
        const mat4 MANIP = manipulated_matrix();
        // needs be padded when using uniform blocks
        const mat3 NORMAL = transform::normal_matrix(MANIP);

        program->bind();
        program->set_uniform("MVP", MVP)
                ->set_uniform("MANIP", MANIP)
                ->set_uniform( "NORMAL", NORMAL);

        float ratio = camera->pixelGLRatio(camera->pivotPoint());
        program->set_uniform("radius", point_size() * ratio * 0.5f)  // 0.5f from size -> radius
                ->set_uniform("wLightPos", wLightPos)
                ->set_uniform("wCamPos", wCamPos)
                ->set_uniform("lighting",lighting())
                ->set_uniform("two_sides_lighting",lighting_two_sides())
                ->set_uniform("distinct_back_color",distinct_back_color())
                ->set_uniform("backside_color",back_color());

        program->set_uniform("highlight",highlight())
                ->set_uniform("highlight_id_min",highlight_range().first)
                ->set_uniform("highlight_id_max",highlight_range().second);

        program->set_uniform("selected", is_selected());

        if (setting::clipping_plane)
            setting::clipping_plane->set_program(program);

        program->bind_texture("textureID",texture()->id(), 0);
        gl_draw();
        program->release_texture();

        program->release();
    }


}
