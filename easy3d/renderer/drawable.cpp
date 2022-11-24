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

#include <cassert>

#include <easy3d/core/model.h>
#include <easy3d/renderer/opengl.h>
#include <easy3d/renderer/vertex_array_object.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/texture_manager.h>
#include <easy3d/renderer/opengl_error.h>
#include <easy3d/renderer/buffer.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/setting.h>
#include <easy3d/util/stop_watch.h>


namespace easy3d {

    Drawable::Drawable(const std::string &name, Model *model)
            : name_(name), model_(model), vao_(nullptr), num_vertices_(0), num_indices_(0),
              update_needed_(false), update_func_(nullptr), vertex_buffer_(0), color_buffer_(0), normal_buffer_(0),
              texcoord_buffer_(0), element_buffer_(0), manipulator_(nullptr) {
        vao_ = new VertexArrayObject;
        material_ = Material(setting::material_ambient, setting::material_specular, setting::material_shininess);
    }


    Drawable::~Drawable() {
        clear();
        delete vao_;
        delete manipulator_;
    }


    const Box3 &Drawable::bounding_box() const {
        if (model_)
            return model_->bounding_box();
        else
            return bbox_;
    }


    void Drawable::buffer_stats(std::ostream &output) const {
        if (vertex_buffer()) {
            output << "\t" << name() << std::endl;
            output << "\t\tvertex buffer:     " << num_vertices_ << " vertices, "
                   << num_vertices_ * sizeof(vec3) << " bytes" << std::endl;
        }
        if (normal_buffer()) {
            output << "\t\tnormal buffer:     " << num_vertices_ << " normals, "
                   << num_vertices_ * sizeof(vec3) << " bytes" << std::endl;
        }
        if (color_buffer()) {
            output << "\t\tcolor buffer:      " << num_vertices_ << " colors, "
                   << num_vertices_ * sizeof(vec3) << " bytes" << std::endl;
        }
        if (texcoord_buffer()) {
            output << "\t\ttexcoord buffer:   " << num_vertices_ << " texcoords, "
                   << num_vertices_ * sizeof(vec2) << " bytes" << std::endl;
        }
        if (element_buffer()) {
            output << "\t\tindex buffer:      " << num_indices_ << " indices, "
                   << num_indices_ * sizeof(unsigned int) << " bytes" << std::endl;
        }
    }


    void Drawable::update() {
        bbox_.clear();
        update_needed_ = true;
    }


    void Drawable::clear() {
        VertexArrayObject::release_buffer(vertex_buffer_);
        VertexArrayObject::release_buffer(color_buffer_);
        VertexArrayObject::release_buffer(normal_buffer_);
        VertexArrayObject::release_buffer(texcoord_buffer_);
        VertexArrayObject::release_buffer(element_buffer_);

        num_vertices_ = 0;
        num_indices_ = 0;
        bbox_.clear();
    }


    void Drawable::disable_element_buffer() {
        VertexArrayObject::release_buffer(element_buffer_);
        num_indices_ = 0;
    }


    void Drawable::update_buffers_internal() {
        if (!model_ && !update_func_) {
            LOG_N_TIMES(3, ERROR)
                << "do not know how to update rendering buffers: drawable not associated with a model and no update function specified. " << COUNTER;
            return;
        } else if (model_ && model_->points().empty()) {
            clear();
            LOG_N_TIMES(3, WARNING) << "model has no valid geometry. " << COUNTER;
            return;
        }

        StopWatch w;
        if (update_func_)
            update_func_(model_, this);
        else
            buffer::update(model_, this);

        LOG_IF(w.elapsed_seconds() > 0.5, INFO) << "updating rendering buffers for drawable '" << name()
                                                << "' took " << w.time_string();
    }


    void Drawable::update_vertex_buffer(const std::vector<vec3> &vertices, bool dynamic) {
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


    void Drawable::update_color_buffer(const std::vector<vec3> &colors, bool dynamic) {
        assert(vao_);

        bool success = vao_->create_array_buffer(color_buffer_, ShaderProgram::COLOR, colors.data(),
                                                 colors.size() * sizeof(vec3), 3, dynamic);
        LOG_IF(!success, ERROR) << "failed updating color buffer";
    }


    void Drawable::update_normal_buffer(const std::vector<vec3> &normals, bool dynamic) {
        assert(vao_);
        bool success = vao_->create_array_buffer(normal_buffer_, ShaderProgram::NORMAL, normals.data(),
                                                 normals.size() * sizeof(vec3), 3, dynamic);
        LOG_IF(!success, ERROR) << "failed updating normal buffer";
    }


    void Drawable::update_texcoord_buffer(const std::vector<vec2> &texcoords, bool dynamic) {
        assert(vao_);

        bool success = vao_->create_array_buffer(texcoord_buffer_, ShaderProgram::TEXCOORD, texcoords.data(),
                                                 texcoords.size() * sizeof(vec2), 2, dynamic);
        LOG_IF(!success, ERROR) << "failed updating texcoord buffer";
    }


    void Drawable::update_element_buffer(const std::vector<unsigned int> &indices) {
        assert(vao_);

        bool status = vao_->create_element_buffer(element_buffer_, indices.data(), indices.size() * sizeof(unsigned int));
        if (!status)
            num_indices_ = 0;
        else
            num_indices_ = indices.size();
    }


    void Drawable::update_element_buffer(const std::vector< std::vector<unsigned int> > &indices) {
        assert(vao_);

        if (type() == DT_POINTS) {
            LOG(WARNING) << "element buffer is not needed for PointsDrawable";
            return;
        }

        unsigned int num = 3;
        if (type() == DT_LINES)
            num = 2;

        std::vector<unsigned int> elements;
        for (const auto& array : indices)
            elements.insert(elements.end(), array.begin(), array.end());

        update_element_buffer(elements);
    }


    void Drawable::gl_draw() const {
        if (update_needed_ || vertex_buffer_ == 0) {
            const_cast<Drawable *>(this)->update_buffers_internal();
            const_cast<Drawable *>(this)->update_needed_ = false;
        }

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


    Manipulator* Drawable::manipulator() {
        if (manipulator_)
            return manipulator_;
        else if (model_)
            return model_->manipulator();
        else
            return nullptr;
    }


    const Manipulator* Drawable::manipulator() const {
        if (manipulator_)
            return manipulator_;
        else if (model_)
            return model_->manipulator();
        else
            return nullptr;
    }


    mat4 Drawable::manipulated_matrix() const {
        if (manipulator_)
            return manipulator()->matrix();
        else if (model_ && model_->manipulator())
            return model_->manipulator()->matrix();
        else
            return mat4::identity();
    }

}
