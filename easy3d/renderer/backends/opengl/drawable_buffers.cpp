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

#include <easy3d/renderer/drawable.h>
#include <easy3d/renderer/vertex_array_object.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/opengl_error.h>
#include <easy3d/core/model.h>
#include <easy3d/util/logging.h>
#include <cassert>
#include <stdexcept>
namespace easy3d {
    void Drawable::opengl_initialize() { vao_.reset(new VertexArrayObject); }
    void Drawable::opengl_release_buffers() {
        VertexArrayObject::release_buffer(vertex_buffer_);
        VertexArrayObject::release_buffer(color_buffer_);
        VertexArrayObject::release_buffer(normal_buffer_);
        VertexArrayObject::release_buffer(texcoord_buffer_);
        VertexArrayObject::release_buffer(element_buffer_);
    }
    void Drawable::opengl_disable_element_buffer() { VertexArrayObject::release_buffer(element_buffer_); }
    void Drawable::opengl_update_vertex_buffer(const std::vector<vec3> &vertices, bool dynamic) {
        assert(vao_);

        bool success = vao_->create_array_buffer(vertex_buffer_, ShaderProgram::POSITION, vertices.data(),
                                                 vertices.size() * sizeof(vec3), 3, dynamic);

        LOG_IF(!success, ERROR) << "failed creating vertex buffer";

        if (!success)
            num_vertices_ = 0;
        else {
            num_vertices_ = vertices.size();

            if (model())
                bbox_ = model()->bounding_box();
            else {
                // update bounding box
                bbox_.clear();
                for (const auto &p : vertices)
                    bbox_.grow(p);
            }
        }
    }


    void Drawable::opengl_update_color_buffer(const std::vector<vec3> &colors, bool dynamic) {
        assert(vao_);

        bool success = vao_->create_array_buffer(color_buffer_, ShaderProgram::COLOR, colors.data(),
                                                 colors.size() * sizeof(vec3), 3, dynamic);
        LOG_IF(!success, ERROR) << "failed updating color buffer";
    }


    void Drawable::opengl_update_normal_buffer(const std::vector<vec3> &normals, bool dynamic) {
        assert(vao_);
        bool success = vao_->create_array_buffer(normal_buffer_, ShaderProgram::NORMAL, normals.data(),
                                                 normals.size() * sizeof(vec3), 3, dynamic);
        LOG_IF(!success, ERROR) << "failed updating normal buffer";
    }


    void Drawable::opengl_update_texcoord_buffer(const std::vector<vec2> &texcoords, bool dynamic) {
        assert(vao_);

        bool success = vao_->create_array_buffer(texcoord_buffer_, ShaderProgram::TEXCOORD, texcoords.data(),
                                                 texcoords.size() * sizeof(vec2), 2, dynamic);
        LOG_IF(!success, ERROR) << "failed updating texcoord buffer";
    }


    void Drawable::opengl_update_element_buffer(const std::vector<unsigned int> &indices) {
        assert(vao_);

        bool status = vao_->create_element_buffer(element_buffer_, indices.data(), indices.size() * sizeof(unsigned int));
        if (!status)
            num_indices_ = 0;
        else
            num_indices_ = indices.size();
    }


    void Drawable::gl_draw() const {
        if (cpu_buffers_) throw std::logic_error("gl_draw() is OpenGL-only; this drawable uses CPU buffers");
        if (update_needed_ || vertex_buffer_ == 0) {
            const_cast<Drawable *>(this)->update_buffers_internal();
            const_cast<Drawable *>(this)->update_needed_ = false;
        }

#ifndef NDEBUG
        LOG_IF_FIRST_N(1, num_indices_ > 0 && num_vertices_ == 0, ERROR)
            << "element buffer provided but vertex buffer filled with 0 vertices";
#endif

        vao_->bind();

        if (element_buffer_) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_);
            easy3d_debug_log_gl_error

            // index buffer must be bound if using glDrawElements()
            glDrawElements(type(), GLsizei(num_indices_), GL_UNSIGNED_INT, nullptr);
            easy3d_debug_log_gl_error

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        } else
            glDrawArrays(type(), 0, GLsizei(num_vertices_));
        easy3d_debug_log_gl_error

        vao_->release();
        easy3d_debug_log_gl_error
    }

}
