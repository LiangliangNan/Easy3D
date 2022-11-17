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

#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/texture_manager.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/util/setting.h>


namespace easy3d {


    LinesDrawable::LinesDrawable(const std::string &name, Model *model)
            : Drawable(name, model), line_width_(1.0f), impostor_type_(PLAIN) {
        lighting_two_sides_ = setting::lines_drawable_two_side_lighting;
        distinct_back_color_ = setting::lines_drawable_distinct_backside_color;
        back_color_ = setting::lines_drawable_backside_color;
        set_uniform_coloring(vec4(0.0f, 0.0f, 0.0f, 1.0f));
    }


    Drawable::Type LinesDrawable::type() const {
        return DT_LINES;
    }


    void LinesDrawable::draw(const Camera *camera /* = false */) const {
        if (update_needed_ || vertex_buffer_ == 0) {
            const_cast<LinesDrawable *>(this)->update_buffers_internal();
            const_cast<LinesDrawable *>(this)->update_needed_ = false;
        }

        switch (impostor_type_) {
            case PLAIN:
                if (texture() && (coloring_method() == State::SCALAR_FIELD || coloring_method() == State::TEXTURED))
                    _draw_plain_lines_with_texture(camera);
                else
                    _draw_plain_lines(camera);
                break;

            case CYLINDER:
                if (texture() && (coloring_method() == State::SCALAR_FIELD || coloring_method() == State::TEXTURED))
                    _draw_cylinders_with_texture(camera);
                else
                    _draw_cylinders(camera);
                break;

            case CONE:
                if (texture() && (coloring_method() == State::SCALAR_FIELD || coloring_method() == State::TEXTURED))
                    _draw_cones_with_texture(camera);
                else
                    _draw_cones(camera);
                break;
        }
    }


    void LinesDrawable::_draw_plain_lines(const Camera *camera) const {
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
            // transformation introduced by manipulation
            const mat4 MANIP = manipulated_matrix();

            program->bind();
            program->set_uniform("MVP", MVP)
                    ->set_uniform("MANIP", MANIP)
                    ->set_uniform("per_vertex_color", coloring_method() != State::UNIFORM_COLOR && color_buffer())
                    ->set_uniform("default_color", color());

            program->set_uniform("highlight", highlight())
                    ->set_uniform("highlight_id_min", highlight_range().first)
                    ->set_uniform("highlight_id_max", highlight_range().second)
                    ->set_uniform("selected", is_selected())
                    ->set_uniform("highlight_color", setting::highlight_color);

            ClippingPlane::instance()->set_program(program);
            ClippingPlane::instance()->set_discard_primitives(program, plane_clip_discard_primitive());

            gl_draw();
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
                    ->set_uniform("PROJ", camera->projectionMatrix())
                    ->set_uniform("MANIP", manipulated_matrix());

            float ratio = camera->pixelGLRatio(camera->pivotPoint());
            program->set_uniform("radius", line_width_ * ratio * 0.5f)  // 0.5f from width -> radius
                    ->set_uniform("default_color", color())
                    ->set_uniform("per_vertex_color", coloring_method() != State::UNIFORM_COLOR && color_buffer());

            program->set_uniform("selected", is_selected());
            program->set_uniform("highlight_color", setting::highlight_color);

            ClippingPlane::instance()->set_program(program);
            ClippingPlane::instance()->set_discard_primitives(program, plane_clip_discard_primitive());

            gl_draw();
            program->release();
        }
    }


    void LinesDrawable::_draw_cylinders(const Camera *camera) const {
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
                ->set_uniform("PROJ", camera->projectionMatrix())
                ->set_uniform("MANIP", manipulated_matrix());

        float ratio = camera->pixelGLRatio(camera->pivotPoint());
        program->set_uniform("radius", line_width_ * ratio * 0.5f)  // 0.5f from width -> radius
                ->set_uniform("default_color", color())
                ->set_uniform("per_vertex_color", coloring_method() != State::UNIFORM_COLOR && color_buffer())
                ->set_uniform("eLightPos", setting::light_position)
                ->set_uniform("lighting", lighting());

        program->set_block_uniform("Material", "ambient", material().ambient)
                ->set_block_uniform("Material", "specular", material().specular)
                ->set_block_uniform("Material", "shininess", &material().shininess);

        program->set_uniform("selected", is_selected());
        program->set_uniform("highlight_color", setting::highlight_color);

        ClippingPlane::instance()->set_program(program);
        ClippingPlane::instance()->set_discard_primitives(program, plane_clip_discard_primitive());

        gl_draw();
        program->release();
    }


    void LinesDrawable::_draw_cones(const Camera *camera) const {
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
                ->set_uniform("PROJ", camera->projectionMatrix())
                ->set_uniform("MANIP", manipulated_matrix());

        float ratio = camera->pixelGLRatio(camera->pivotPoint());
        program->set_uniform("radius", line_width() * ratio * 0.5f)  // 0.5f from width -> radius
                ->set_uniform("default_color", color())
                ->set_uniform("per_vertex_color", coloring_method() != State::UNIFORM_COLOR && color_buffer())
                ->set_uniform("eLightPos", setting::light_position)
                ->set_uniform("lighting", lighting())
                ->set_block_uniform("Material", "ambient", material().ambient)
                ->set_block_uniform("Material", "specular", material().specular)
                ->set_block_uniform("Material", "shininess", &material().shininess);

        program->set_uniform("selected", is_selected());
        program->set_uniform("highlight_color", setting::highlight_color);

        ClippingPlane::instance()->set_program(program);
        ClippingPlane::instance()->set_discard_primitives(program, plane_clip_discard_primitive());

        gl_draw();
        program->release();
    }


    void LinesDrawable::_draw_plain_lines_with_texture(const Camera *camera) const {
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
            // transformation introduced by manipulation
            const mat4 MANIP = manipulated_matrix();

            program->bind();
            program->set_uniform("MVP", MVP)
                    ->set_uniform("MANIP", MANIP);

            program->set_uniform("highlight", highlight())
                    ->set_uniform("highlight_id_min", highlight_range().first)
                    ->set_uniform("highlight_id_max", highlight_range().second)
                    ->set_uniform("selected", is_selected())
                    ->set_uniform("highlight_color", setting::highlight_color);

            ClippingPlane::instance()->set_program(program);
            ClippingPlane::instance()->set_discard_primitives(program, plane_clip_discard_primitive());

            program->bind_texture("textureID", texture()->id(), 0);
            gl_draw();
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
                    ->set_uniform("PROJ", camera->projectionMatrix())
                    ->set_uniform("MANIP", manipulated_matrix());
            float ratio = camera->pixelGLRatio(camera->pivotPoint());
            program->set_uniform("radius", line_width() * ratio * 0.5f);  // 0.5f from width -> radius

            program->set_uniform("selected", is_selected());
            program->set_uniform("highlight_color", setting::highlight_color);

            ClippingPlane::instance()->set_program(program);
            ClippingPlane::instance()->set_discard_primitives(program, plane_clip_discard_primitive());

            program->bind_texture("textureID", texture()->id(), 0);
            gl_draw();
            program->release_texture();

            program->release();
        }
    }


    void LinesDrawable::_draw_cylinders_with_texture(const Camera *camera) const {
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
                ->set_uniform("PROJ", camera->projectionMatrix())
                ->set_uniform("MANIP", manipulated_matrix());

        float ratio = camera->pixelGLRatio(camera->pivotPoint());
        program->set_uniform("radius", line_width_ * ratio * 0.5f)  // 0.5f from width -> radius
                ->set_uniform("eLightPos", setting::light_position)
                ->set_uniform("lighting", lighting());

        program->set_block_uniform("Material", "ambient", material().ambient)
                ->set_block_uniform("Material", "specular", material().specular)
                ->set_block_uniform("Material", "shininess", &material().shininess);

        program->set_uniform("selected", is_selected());
        program->set_uniform("highlight_color", setting::highlight_color);

        ClippingPlane::instance()->set_program(program);
        ClippingPlane::instance()->set_discard_primitives(program, plane_clip_discard_primitive());

        program->bind_texture("textureID", texture()->id(), 0);
        gl_draw();
        program->release_texture();

        program->release();
    }


    void LinesDrawable::_draw_cones_with_texture(const Camera *camera) const {
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
                ->set_uniform("PROJ", camera->projectionMatrix())
                ->set_uniform("MANIP", manipulated_matrix());

        float ratio = camera->pixelGLRatio(camera->pivotPoint());
        program->set_uniform("radius", line_width_ * ratio * 0.5f)  // 0.5f from width -> radius
                ->set_uniform("eLightPos", setting::light_position)
                ->set_uniform("lighting", lighting());

        program->set_block_uniform("Material", "ambient", material().ambient)
                ->set_block_uniform("Material", "specular", material().specular)
                ->set_block_uniform("Material", "shininess", &material().shininess);

        program->set_uniform("selected", is_selected());
        program->set_uniform("highlight_color", setting::highlight_color);

        ClippingPlane::instance()->set_program(program);
        ClippingPlane::instance()->set_discard_primitives(program, plane_clip_discard_primitive());

        program->bind_texture("textureID", texture()->id(), 0);

        gl_draw();
        program->release_texture();

        program->release();
    }


}
