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


#include <easy3d/viewer/drawable_lines.h>
#include <easy3d/viewer/camera.h>
#include <easy3d/viewer/shader_program.h>
#include <easy3d/viewer/shader_manager.h>
#include <easy3d/viewer/setting.h>


namespace easy3d {


    LinesDrawable::LinesDrawable(const std::string& name /* = ""*/)
        : Drawable(name)
        , impostor_type_(IT_NULL)
        , impostor_thickness_(1.0f)
    {
        default_color_ = vec3(0.0f, 0.0f, 0.0f);
    }


    DrawableType LinesDrawable::type() const {
        return DT_LINES;
    }


    void LinesDrawable::draw(const Camera* camera, bool  with_storage_buffer /* = false */) const {
        switch (impostor_type_) {
        case IT_NULL:
            if (texture_)
                _draw_plain_lines_with_texture(camera, with_storage_buffer);
            else
                _draw_plain_lines(camera, with_storage_buffer);
            break;

        case IT_CYLINDERS:
            if (texture_)
                _draw_cylinders_with_texture(camera, with_storage_buffer);
            else
                _draw_cylinders(camera, with_storage_buffer);
            break;

        case IT_CONES:
            if (texture_)
                _draw_cones_with_texture(camera, with_storage_buffer);
            else
                _draw_cones(camera, with_storage_buffer);
            break;
        }
    }


    void LinesDrawable::_draw_plain_lines(const Camera* camera, bool with_storage_buffer) const {
        ShaderProgram* program = ShaderManager::get_program("lines/lines_plain_color");
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes;
            attributes.push_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
            attributes.push_back(ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"));
            program = ShaderManager::create_program_from_files("lines/lines_plain_color", attributes);
        }

        if (!program)
            return;

        const mat4& MVP = camera->modelViewProjectionMatrix();
        program->bind();
        program->set_uniform("MVP", MVP);
        program->set_uniform("per_vertex_color", per_vertex_color() && color_buffer());
        program->set_uniform("default_color", default_color());
        gl_draw(with_storage_buffer);
        program->release();
    }


    void LinesDrawable::_draw_cylinders(const Camera* camera, bool with_storage_buffer) const {
        ShaderProgram* program = ShaderManager::get_program("lines/lines_cylinders_color");
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes;
            attributes.push_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
            attributes.push_back(ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"));
            std::vector<std::string> outputs;
            program = ShaderManager::create_program_from_files("lines/lines_cylinders_color", attributes, outputs, true);
        }
        if (!program)
            return;

        program->bind();
        program->set_uniform("perspective", camera->type() == Camera::PERSPECTIVE);

        program->set_uniform("MV", camera->modelViewMatrix());
        program->set_uniform("PROJ", camera->projectionMatrix());

        float ratio = camera->pixelGLRatio(camera->sceneCenter());
        program->set_uniform("radius", impostor_thickness() * ratio);

        program->set_uniform("default_color", default_color());
        program->set_uniform("per_vertex_color", per_vertex_color() && color_buffer());

        program->set_block_uniform("Lighting", "eLightPos", setting::light_position);
        program->set_block_uniform("Lighting", "ambient", setting::light_ambient);
        program->set_block_uniform("Lighting", "specular", setting::light_specular);
        program->set_block_uniform("Lighting", "shininess", &setting::light_shininess);

        gl_draw(with_storage_buffer);
        program->release();
    }


    void LinesDrawable::_draw_cones(const Camera* camera, bool with_storage_buffer) const {
        ShaderProgram* program = ShaderManager::get_program("lines/lines_cones_color");
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes;
            attributes.push_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
            attributes.push_back(ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"));
            std::vector<std::string> outputs;
            program = ShaderManager::create_program_from_files("lines/lines_cones_color", attributes, outputs, true);
        }
        if (!program)
            return;

        program->bind();
        program->set_uniform("perspective", camera->type() == Camera::PERSPECTIVE);

        program->set_uniform("MV", camera->modelViewMatrix());
        program->set_uniform("PROJ", camera->projectionMatrix());

        float ratio = camera->pixelGLRatio(camera->sceneCenter());
        program->set_uniform("radius", impostor_thickness() * ratio);

        program->set_uniform("default_color", default_color());
        program->set_uniform("per_vertex_color", per_vertex_color() && color_buffer());

        program->set_block_uniform("Lighting", "eLightPos", setting::light_position);
        program->set_block_uniform("Lighting", "ambient", setting::light_ambient);
        program->set_block_uniform("Lighting", "specular", setting::light_specular);
        program->set_block_uniform("Lighting", "shininess", &setting::light_shininess);

        gl_draw(with_storage_buffer);
        program->release();
    }


    void LinesDrawable::_draw_plain_lines_with_texture(const Camera* camera, bool with_storage_buffer) const {

    }


    void LinesDrawable::_draw_cylinders_with_texture(const Camera* camera, bool with_storage_buffer) const {

    }


    void LinesDrawable::_draw_cones_with_texture(const Camera *camera, bool with_storage_buffer) const {

    }


}
