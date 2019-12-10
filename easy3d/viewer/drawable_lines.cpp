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


namespace easy3d {

    DrawableType LinesDrawable::type() const {
        return DT_LINES;
    }


    void LinesDrawable::draw(const Camera* camera, bool  with_storage_buffer /* = false */) const {
        ShaderProgram* program = ShaderManager::get_program("lines_color");
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes;
            attributes.push_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
            attributes.push_back(ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"));
            program = ShaderManager::create_program_from_files("lines_color", attributes);
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

}
