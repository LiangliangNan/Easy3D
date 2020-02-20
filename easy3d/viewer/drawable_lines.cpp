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

#include <easy3d/viewer/drawable_lines.h>
#include <easy3d/viewer/camera.h>
#include <easy3d/viewer/shader_program.h>
#include <easy3d/viewer/shader_manager.h>
#include <easy3d/viewer/texture.h>
#include <easy3d/viewer/setting.h>
#include <easy3d/viewer/clipping_plane.h>
#include <easy3d/util/logging.h>


namespace easy3d {


    LinesDrawable::LinesDrawable(const std::string &name /* = ""*/)
            : Drawable(name), line_width_(1.0f), impostor_type_(PLAIN) {
        default_color_ = vec3(0.0f, 0.0f, 0.0f);
    }


    DrawableType LinesDrawable::type() const {
        return DT_LINES;
    }


    void LinesDrawable::draw(const Camera *camera, bool with_storage_buffer /* = false */) const {
        switch (impostor_type_) {
            case PLAIN:
                if (use_texture_ && texture_)
                    _draw_plain_lines_with_texture(camera, with_storage_buffer);
                else
                    _draw_plain_lines(camera, with_storage_buffer);
                break;

            case CYLINDER:
                if (use_texture_ && texture_)
                    _draw_cylinders_with_texture(camera, with_storage_buffer);
                else
                    _draw_cylinders(camera, with_storage_buffer);
                break;

            case CONE:
                if (use_texture_ && texture_)
                    _draw_cones_with_texture(camera, with_storage_buffer);
                else
                    _draw_cones(camera, with_storage_buffer);
                break;
        }
    }


    void LinesDrawable::_draw_plain_lines(const Camera *camera, bool with_storage_buffer) const {
        if (vertex_buffer() == 0) {
            LOG_FIRST_N(ERROR, 1) << "vertex buffer not created (this is the first record)";
            return;
        }

        if (line_width() <= 1) {
            ShaderProgram *program = ShaderManager::get_program("lines/lines_plain_color");
            if (!program) {
                std::vector<ShaderProgram::Attribute> attributes;
                attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
                attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"));
                program = ShaderManager::create_program_from_files("lines/lines_plain_color", attributes);
            }
            if (!program)
                return;

            const mat4 &MVP = camera->modelViewProjectionMatrix();
            program->bind();
            program->set_uniform("MVP", MVP)
                    ->set_uniform("per_vertex_color", per_vertex_color() && color_buffer())
                    ->set_uniform("default_color", default_color());

            if (setting::clipping_plane)
                setting::clipping_plane->set_program(program);

            gl_draw(with_storage_buffer);
            program->release();
        } else {  // use geometry shader to be able to control the line width
            ShaderProgram *program = ShaderManager::get_program("lines/lines_plain_color_width_control");
            if (!program) {
                std::vector<ShaderProgram::Attribute> attributes;
                attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
                attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"));
                program = ShaderManager::create_program_from_files("lines/lines_plain_color_width_control", attributes,
                                                                   std::vector<std::string>(), true);
            }
            if (!program)
                return;

            program->bind();
            program->set_uniform("perspective", camera->type() == Camera::PERSPECTIVE)
                    ->set_uniform("invMV", inverse(camera->modelViewMatrix()))
                    ->set_uniform("MV", camera->modelViewMatrix())
                    ->set_uniform("PROJ", camera->projectionMatrix());

            float ratio = camera->pixelGLRatio(camera->pivotPoint());
            program->set_uniform("radius", line_width_ * ratio)
                    ->set_uniform("default_color", default_color())
                    ->set_uniform("per_vertex_color", per_vertex_color() && color_buffer());

            if (setting::clipping_plane)
                setting::clipping_plane->set_program(program);

            gl_draw(with_storage_buffer);
            program->release();
        }
    }


    void LinesDrawable::_draw_cylinders(const Camera *camera, bool with_storage_buffer) const {
        if (vertex_buffer() == 0) {
            LOG_FIRST_N(ERROR, 1) << "vertex buffer not created (this is the first record)";
            return;
        }

        ShaderProgram *program = ShaderManager::get_program("lines/lines_cylinders_color");
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes;
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"));
            program = ShaderManager::create_program_from_files("lines/lines_cylinders_color", attributes,
                                                               std::vector<std::string>(), true);
        }
        if (!program)
            return;

        program->bind();
        program->set_uniform("perspective", camera->type() == Camera::PERSPECTIVE)
                ->set_uniform("invMV", inverse(camera->modelViewMatrix()))
                ->set_uniform("MV", camera->modelViewMatrix())
                ->set_uniform("PROJ", camera->projectionMatrix());

        float ratio = camera->pixelGLRatio(camera->pivotPoint());
        program->set_uniform("radius", line_width_ * ratio)
                ->set_uniform("default_color", default_color())
                ->set_uniform("per_vertex_color", per_vertex_color() && color_buffer())
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


    void LinesDrawable::_draw_cones(const Camera *camera, bool with_storage_buffer) const {
        if (vertex_buffer() == 0) {
            LOG_FIRST_N(ERROR, 1) << "vertex buffer not created (this is the first record)";
            return;
        }

        ShaderProgram *program = ShaderManager::get_program("lines/lines_cones_color");
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes;
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"));
            program = ShaderManager::create_program_from_files("lines/lines_cones_color", attributes,
                                                               std::vector<std::string>(), true);
        }
        if (!program)
            return;

        program->bind();
        program->set_uniform("perspective", camera->type() == Camera::PERSPECTIVE)
                ->set_uniform("MV", camera->modelViewMatrix())
                ->set_uniform("invMV", inverse(camera->modelViewMatrix()))
                ->set_uniform("PROJ", camera->projectionMatrix());

        float ratio = camera->pixelGLRatio(camera->pivotPoint());
        program->set_uniform("radius", line_width() * ratio)
                ->set_uniform("default_color", default_color())
                ->set_uniform("per_vertex_color", per_vertex_color() && color_buffer())
                ->set_uniform("eLightPos", setting::light_position)
                ->set_uniform("lighting", lighting()),

                program->set_block_uniform("Material", "ambient", material().ambient)
                        ->set_block_uniform("Material", "specular", material().specular)
                        ->set_block_uniform("Material", "shininess", &material().shininess);

        if (setting::clipping_plane)
            setting::clipping_plane->set_program(program);

        gl_draw(with_storage_buffer);
        program->release();
    }


    void LinesDrawable::_draw_plain_lines_with_texture(const Camera *camera, bool with_storage_buffer) const {
        if (vertex_buffer() == 0) {
            LOG_FIRST_N(ERROR, 1) << "vertex buffer not created (this is the first record)";
            return;
        }
        if (texcoord_buffer() == 0) {
            LOG_FIRST_N(ERROR, 1) << "texcoord buffer not created (this is the first record)";
            return;
        }

        if (line_width() <= 1) {
            ShaderProgram *program = ShaderManager::get_program("lines/lines_plain_texture");
            if (!program) {
                std::vector<ShaderProgram::Attribute> attributes;
                attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
                attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::TEXCOORD, "vtx_texcoord"));
                program = ShaderManager::create_program_from_files("lines/lines_plain_texture", attributes);
            }
            if (!program)
                return;

            const mat4 &MVP = camera->modelViewProjectionMatrix();
            program->bind();
            program->set_uniform("MVP", MVP);

//      program->set_uniform("highlight", highlight())
//                ->set_uniform("hightlight_id_min", highlight_range_.first)
//                ->set_uniform("hightlight_id_max", highlight_range_.second);
            if (setting::clipping_plane)
                setting::clipping_plane->set_program(program);

            program->bind_texture("textureID", texture()->id(), 0);
            gl_draw(with_storage_buffer);
            program->release_texture();

            program->release();
        } else {  // use geometry shader to be able to control the line width
            ShaderProgram *program = ShaderManager::get_program("lines/lines_plain_texture_width_control");
            if (!program) {
                std::vector<ShaderProgram::Attribute> attributes;
                attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
                attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::TEXCOORD, "vtx_texcoord"));
                program = ShaderManager::create_program_from_files("lines/lines_plain_texture_width_control",
                                                                   attributes, std::vector<std::string>(), true);
            }
            if (!program)
                return;

            program->bind();
            program->set_uniform("MV", camera->modelViewMatrix())
                    ->set_uniform("invMV", inverse(camera->modelViewMatrix()))
                    ->set_uniform("PROJ", camera->projectionMatrix());
            float ratio = camera->pixelGLRatio(camera->pivotPoint());
            program->set_uniform("radius", line_width() * ratio);

            //      program->set_uniform("highlight", highlight())
            //                ->set_uniform("hightlight_id_min", highlight_range_.first)
            //                ->set_uniform("hightlight_id_max", highlight_range_.second);
            if (setting::clipping_plane)
                setting::clipping_plane->set_program(program);

            program->bind_texture("textureID", texture()->id(), 0);
            gl_draw(with_storage_buffer);
            program->release_texture();

            program->release();
        }
    }


    void LinesDrawable::_draw_cylinders_with_texture(const Camera *camera, bool with_storage_buffer) const {
        if (vertex_buffer() == 0) {
            LOG_FIRST_N(ERROR, 1) << "vertex buffer not created (this is the first record)";
            return;
        }
        if (texcoord_buffer() == 0) {
            LOG_FIRST_N(ERROR, 1) << "texcoord buffer not created (this is the first record)";
            return;
        }

        ShaderProgram *program = ShaderManager::get_program("lines/lines_cylinders_texture");
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes;
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::TEXCOORD, "vtx_texcoord"));
            program = ShaderManager::create_program_from_files("lines/lines_cylinders_texture", attributes,
                                                               std::vector<std::string>(), true);
        }
        if (!program)
            return;

        program->bind();
        program->set_uniform("perspective", camera->type() == Camera::PERSPECTIVE)
                ->set_uniform("invMV", inverse(camera->modelViewMatrix()))
                ->set_uniform("MV", camera->modelViewMatrix())
                ->set_uniform("PROJ", camera->projectionMatrix());

        float ratio = camera->pixelGLRatio(camera->pivotPoint());
        program->set_uniform("radius", line_width_ * ratio)
                ->set_uniform("eLightPos", setting::light_position)
                ->set_uniform("lighting", lighting());

        program->set_block_uniform("Material", "ambient", material().ambient)
                ->set_block_uniform("Material", "specular", material().specular)
                ->set_block_uniform("Material", "shininess", &material().shininess);

//      program->set_uniform("highlight", highlight())
//                ->set_uniform("hightlight_id_min", highlight_range_.first)
//                ->set_uniform("hightlight_id_max", highlight_range_.second);
        if (setting::clipping_plane)
            setting::clipping_plane->set_program(program);

        program->bind_texture("textureID", texture()->id(), 0);
        gl_draw(with_storage_buffer);
        program->release_texture();

        program->release();
    }


    void LinesDrawable::_draw_cones_with_texture(const Camera *camera, bool with_storage_buffer) const {
        if (vertex_buffer() == 0) {
            LOG_FIRST_N(ERROR, 1) << "vertex buffer not created (this is the first record)";
            return;
        }
        if (texcoord_buffer() == 0) {
            LOG_FIRST_N(ERROR, 1) << "texcoord buffer not created (this is the first record)";
            return;
        }

        ShaderProgram *program = ShaderManager::get_program("lines/lines_cones_texture");
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes;
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::TEXCOORD, "vtx_texcoord"));
            program = ShaderManager::create_program_from_files("lines/lines_cones_texture", attributes,
                                                               std::vector<std::string>(), true);
        }
        if (!program)
            return;

        program->bind();
        program->set_uniform("perspective", camera->type() == Camera::PERSPECTIVE)
                ->set_uniform("invMV", inverse(camera->modelViewMatrix()))
                ->set_uniform("MV", camera->modelViewMatrix())
                ->set_uniform("PROJ", camera->projectionMatrix());

        float ratio = camera->pixelGLRatio(camera->pivotPoint());
        program->set_uniform("radius", line_width_ * ratio)
                ->set_uniform("eLightPos", setting::light_position)
                ->set_uniform("lighting", lighting());

        program->set_block_uniform("Material", "ambient", material().ambient)
                ->set_block_uniform("Material", "specular", material().specular)
                ->set_block_uniform("Material", "shininess", &material().shininess);

        if (setting::clipping_plane)
            setting::clipping_plane->set_program(program);

        program->bind_texture("textureID", texture()->id(), 0);
//      program->set_uniform("highlight", highlight())
//                ->set_uniform("hightlight_id_min", highlight_range_.first)
//                ->set_uniform("hightlight_id_max", highlight_range_.second);

        gl_draw(with_storage_buffer);
        program->release_texture();

        program->release();
    }


}
