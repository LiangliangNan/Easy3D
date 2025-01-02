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

#include <easy3d/renderer/drawable_lines_2D.h>
#include <easy3d/renderer/vertex_array_object.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/opengl_error.h>


namespace easy3d {


    LinesDrawable2D::LinesDrawable2D(const std::string &name)
            : Drawable(name, nullptr)
    {
        set_uniform_coloring(vec4(0.0f, 0.0f, 1.0f, 1.0f));
    }


    Drawable::Type LinesDrawable2D::type() const {
        return DT_LINES;
    }


    void LinesDrawable2D::update_vertex_buffer(const std::vector<vec2> &vertices, int width, int height, bool dynamic) {
        assert(vao_);

        // convert from screen coordinates to Normalized Device Coordinates (NDC)
        std::vector<vec2> points(vertices.size());
        for (std::size_t i = 0; i < vertices.size(); ++i) {
            const vec2 &p = vertices[i];
            const float x = 2.0f * p.x / static_cast<float>(width) - 1.0f;
            const float y = 1.0f - 2.0f * p.y / static_cast<float>(height);
            points[i] = vec2(x, y);
        }
        bool success = vao_->create_array_buffer(vertex_buffer_, ShaderProgram::POSITION, points.data(),
                                                 points.size() * sizeof(vec2), 2, dynamic);
        LOG_IF(!success, ERROR) << "failed creating vertex buffer";
        num_vertices_ = success ? points.size() : 0;
    }


    void LinesDrawable2D::draw(const Camera *camera) const {
        if (update_needed_ || vertex_buffer_ == 0) {
            const_cast<LinesDrawable2D *>(this)->update_buffers_internal();
            const_cast<LinesDrawable2D *>(this)->update_needed_ = false;
        }

        // create the shader program for visualizing the dividing lines
        const std::string name = "screen_space/screen_space_color";
        ShaderProgram *program = ShaderManager::get_program(name);
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes = {
                    ShaderProgram::Attribute(ShaderProgram::POSITION, "ndc_position")
            };
            program = ShaderManager::create_program_from_files(name, attributes);
        }
        if (!program) {
            LOG(ERROR) << "shader doesn't exist: " << name;
            return;
        }

        const float depth = -1.0f; // on top of everything
        program->bind();
        program->set_uniform("screen_color", color_);
        program->set_uniform("depth", depth);

        gl_draw();

        program->release();
        easy3d_debug_log_gl_error
    }

}
