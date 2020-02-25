/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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
 */

#include <easy3d/viewer/drawable_points.h>
#include <easy3d/viewer/camera.h>
#include <easy3d/viewer/shader_program.h>
#include <easy3d/viewer/shader_manager.h>
#include <easy3d/viewer/texture.h>
#include <easy3d/viewer/setting.h>
#include <easy3d/viewer/opengl.h>
#include <easy3d/viewer/opengl_error.h>
#include <easy3d/viewer/clipping_plane.h>
#include <easy3d/util/logging.h>


namespace easy3d {

    PointsDrawable::PointsDrawable(const std::string &name /*= ""*/)
            : Drawable(name), point_size_(2.0f), impostor_type_(PLAIN) {
        default_color_ = vec3(0.0f, 1.0f, 0.0f);
    }


    DrawableType PointsDrawable::type() const {
        return DT_POINTS;
    }


    void PointsDrawable::draw(const Camera *camera, bool with_storage_buffer /* = false */) const {
        switch (impostor_type_) {
            case PLAIN:
                if (use_texture_ && texture_)
                    _draw_plain_points_with_texture(camera, with_storage_buffer);
                else
                    _draw_plain_points(camera, with_storage_buffer);
                break;

            case SPHERE:
#if 0
                if (use_texture_ && texture_)
                    _draw_spheres_with_texture_sprite(camera, with_storage_buffer);
                else
                    _draw_spheres_sprite(camera, with_storage_buffer);
#else
                if (use_texture_ && texture_)
                    _draw_spheres_with_texture_geometry(camera, with_storage_buffer);
                else
                    _draw_spheres_geometry(camera, with_storage_buffer);
#endif
                break;

            case SURFEL:
                if (use_texture_ && texture_)
                    _draw_surfels_with_texture(camera, with_storage_buffer);
                else
                    _draw_surfels(camera, with_storage_buffer);
                break;
        }
    }


    void PointsDrawable::_draw_plain_points(const Camera *camera, bool with_storage_buffer) const {
        if (vertex_buffer() == 0) {
            LOG_FIRST_N(ERROR, 1) << "vertex buffer not created (this is the first record)";
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

        glPointSize(point_size());

        program->bind();
        program->set_uniform("MVP", MVP)
                ->set_uniform("lighting", normal_buffer() && lighting())
                ->set_uniform("two_sides_lighting", lighting_two_sides())
                ->set_uniform("wLightPos", wLightPos)
                ->set_uniform("wCamPos", wCamPos)
                ->set_uniform("per_vertex_color", per_vertex_color() && color_buffer())
                ->set_uniform("default_color", default_color())
                ->set_uniform("distinct_back_color", distinct_back_color())
                ->set_uniform("backside_color", back_color())
                ->set_block_uniform("Material", "ambient", material().ambient)
                ->set_block_uniform("Material", "specular", material().specular)
                ->set_block_uniform("Material", "shininess", &material().shininess);

        if (setting::clipping_plane)
            setting::clipping_plane->set_program(program);

        gl_draw(with_storage_buffer);
        program->release();
    }


    void PointsDrawable::_draw_spheres_sprite(const Camera *camera, bool with_storage_buffer) const {
        if (vertex_buffer() == 0) {
            LOG_FIRST_N(ERROR, 1) << "vertex buffer not created (this is the first record)";
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
                ->set_uniform("screen_width", camera->screenWidth());

        float ratio = camera->pixelGLRatio(camera->pivotPoint());
        program->set_uniform("sphere_radius", point_size() * ratio)
                ->set_uniform("per_vertex_color", per_vertex_color() && color_buffer())
                ->set_uniform("default_color", default_color())
                ->set_uniform("eLightPos", setting::light_position)
                ->set_uniform("lighting", lighting());

        program->set_block_uniform("Material", "ambient", material().ambient)
                ->set_block_uniform("Material", "specular", material().specular)
                ->set_block_uniform("Material", "shininess", &material().shininess);

        if (setting::clipping_plane)
            setting::clipping_plane->set_program(program);

        gl_draw(with_storage_buffer);
        program->release();

        glDisable(GL_VERTEX_PROGRAM_POINT_SIZE); // starting from GL3.2, using GL_PROGRAM_POINT_SIZE
    }


    void PointsDrawable::_draw_spheres_geometry(const Camera *camera, bool with_storage_buffer) const {
        if (vertex_buffer() == 0) {
            LOG_FIRST_N(ERROR, 1) << "vertex buffer not created (this is the first record)";
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
                ->set_uniform("PROJ", camera->projectionMatrix());

        float ratio = camera->pixelGLRatio(camera->pivotPoint());
        program->set_uniform("sphere_radius", point_size() * ratio)
                ->set_uniform("per_vertex_color", per_vertex_color() && color_buffer())
                ->set_uniform("default_color", default_color())
                ->set_uniform("eLightPos", setting::light_position)
                ->set_uniform("lighting", lighting());

        program->set_block_uniform("Material", "ambient", material().ambient)
                ->set_block_uniform("Material", "specular", material().specular)
                ->set_block_uniform("Material", "shininess", &material().shininess);

        if (setting::clipping_plane)
            setting::clipping_plane->set_program(program);

        gl_draw(with_storage_buffer);
        program->release();
    }


    void PointsDrawable::_draw_plain_points_with_texture(const Camera *camera, bool with_storage_buffer) const {
        if (vertex_buffer() == 0) {
            LOG_FIRST_N(ERROR, 1) << "vertex buffer not created (this is the first record)";
            return;
        }
        if (texcoord_buffer() == 0) {
            LOG_FIRST_N(ERROR, 1) << "texcoord buffer not created (this is the first record)";
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

        glPointSize(point_size());

        program->bind();
        program->set_uniform("MVP", MVP)
                ->set_uniform("lighting", normal_buffer() && lighting())
                ->set_uniform("two_sides_lighting", lighting_two_sides())
                ->set_uniform("wLightPos", wLightPos)
                ->set_uniform("wCamPos", wCamPos)
                ->set_uniform("distinct_back_color", distinct_back_color())
                ->set_uniform("backside_color", back_color())
                ->set_block_uniform("Material", "ambient", material().ambient)
                ->set_block_uniform("Material", "specular", material().specular)
                ->set_block_uniform("Material", "shininess", &material().shininess)
                ->set_uniform("highlight", highlight())
                ->set_uniform("hightlight_id_min", highlight_range_.first)
                ->set_uniform("hightlight_id_max", highlight_range_.second)
                ->bind_texture("textureID", texture()->id(), 0);

        if (setting::clipping_plane)
            setting::clipping_plane->set_program(program);

        gl_draw(with_storage_buffer);
        program->release_texture();

        program->release();
    }


    void PointsDrawable::_draw_spheres_with_texture_sprite(const Camera *camera, bool with_storage_buffer) const {
        if (vertex_buffer() == 0) {
            LOG_FIRST_N(ERROR, 1) << "vertex buffer not created (this is the first record)";
            return;
        }
        if (texcoord_buffer() == 0) {
            LOG_FIRST_N(ERROR, 1) << "texcoord buffer not created (this is the first record)";
            return;
        }

    }


    void PointsDrawable::_draw_spheres_with_texture_geometry(const Camera *camera, bool with_storage_buffer) const {
        if (vertex_buffer() == 0) {
            LOG_FIRST_N(ERROR, 1) << "vertex buffer not created (this is the first record)";
            return;
        }
        if (texcoord_buffer() == 0) {
            LOG_FIRST_N(ERROR, 1) << "texcoord buffer not created (this is the first record)";
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
                ->set_uniform("PROJ", camera->projectionMatrix());

        float ratio = camera->pixelGLRatio(camera->pivotPoint());
        program->set_uniform("sphere_radius", point_size() * ratio)
                ->set_uniform("eLightPos", setting::light_position)
                ->set_uniform("lighting", lighting());

        program->set_block_uniform("Material", "ambient", material().ambient)
                ->set_block_uniform("Material", "specular", material().specular)
                ->set_block_uniform("Material", "shininess", &material().shininess);

        program->set_uniform("highlight", highlight())
                ->set_uniform("hightlight_id_min", highlight_range_.first)
                ->set_uniform("hightlight_id_max", highlight_range_.second);

        if (setting::clipping_plane)
            setting::clipping_plane->set_program(program);

        program->bind_texture("textureID", texture()->id(), 0);
        gl_draw(with_storage_buffer);
        program->release_texture();

        program->release();
    }


    void PointsDrawable::_draw_surfels(const Camera *camera, bool with_storage_buffer) const {
        if (vertex_buffer() == 0) {
            LOG_FIRST_N(ERROR, 1) << "vertex buffer not created (this is the first record)";
            return;
        }
        if (normal_buffer() == 0) {
            LOG_FIRST_N(ERROR, 1) << "normal buffer not created (this is the first record)";
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

        program->bind();
        program->set_uniform("MVP", MVP)
                ->set_uniform("per_vertex_color", per_vertex_color() && color_buffer())
                ->set_uniform("default_color", default_color());

        float ratio = camera->pixelGLRatio(camera->pivotPoint());
        program->set_uniform("radius", point_size() * ratio)
                ->set_uniform("wLightPos", wLightPos)
                ->set_uniform("wCamPos", wCamPos)
                ->set_uniform("lighting", lighting())
                ->set_uniform("two_sides_lighting", lighting_two_sides())
                ->set_uniform("distinct_back_color", distinct_back_color())
                ->set_uniform("backside_color", back_color());

        program->set_uniform("highlight", highlight())
                ->set_uniform("hightlight_id_min", highlight_range_.first)
                ->set_uniform("hightlight_id_max", highlight_range_.second);

        program->set_block_uniform("Material", "ambient", material().ambient)
                ->set_block_uniform("Material", "specular", material().specular)
                ->set_block_uniform("Material", "shininess", &material().shininess);

        if (setting::clipping_plane)
            setting::clipping_plane->set_program(program);

        gl_draw(with_storage_buffer);
        program->release();
    }


    void PointsDrawable::_draw_surfels_with_texture(const Camera *camera, bool with_storage_buffer) const {
        if (vertex_buffer() == 0) {
            LOG_FIRST_N(ERROR, 1) << "vertex buffer not created (this is the first record)";
            return;
        }
        if (normal_buffer() == 0) {
            LOG_FIRST_N(ERROR, 1) << "normal buffer not created (this is the first record)";
            return;
        }
        if (texcoord_buffer() == 0) {
            LOG_FIRST_N(ERROR, 1) << "texcoord buffer not created (this is the first record)";
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

        program->bind();
        program->set_uniform("MVP", MVP);

        float ratio = camera->pixelGLRatio(camera->pivotPoint());
        program->set_uniform("radius", point_size() * ratio)
                ->set_uniform("wLightPos", wLightPos)
                ->set_uniform("wCamPos", wCamPos)
                ->set_uniform("lighting", lighting())
                ->set_uniform("two_sides_lighting", lighting_two_sides())
                ->set_uniform("distinct_back_color", distinct_back_color())
                ->set_uniform("backside_color", back_color());

        program->set_uniform("highlight", highlight())
                ->set_uniform("hightlight_id_min", highlight_range_.first)
                ->set_uniform("hightlight_id_max", highlight_range_.second);

        if (setting::clipping_plane)
            setting::clipping_plane->set_program(program);

        program->bind_texture("textureID", texture()->id(), 0);
        gl_draw(with_storage_buffer);
        program->release_texture();

        program->release();
    }


}
