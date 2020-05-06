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

#include <easy3d/viewer/drawable.h>

#include <cassert>

#include <easy3d/viewer/opengl.h>
#include <easy3d/viewer/model.h>
#include <easy3d/viewer/vertex_array_object.h>
#include <easy3d/viewer/shader_program.h>
#include <easy3d/viewer/texture_manager.h>
#include <easy3d/viewer/opengl_error.h>
#include <easy3d/viewer/renderer.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/stop_watch.h>


namespace easy3d {


    Drawable::Drawable(const std::string &name, Model *model)
            : name_(name)
            , model_(model)
            , num_vertices_(0)
            , num_indices_(0)
            , update_requested_(false)
            , update_func_(nullptr)
            , vertex_buffer_(0)
            , color_buffer_(0)
            , normal_buffer_(0)
            , texcoord_buffer_(0)
            , element_buffer_(0)
            , storage_buffer_(0)
            , current_storage_buffer_size_(0)
            , selection_buffer_(0)
            , current_selection_buffer_size_(0)
    {
        vao_ = new VertexArrayObject;
    }


    Drawable::~Drawable() {
        clear();
    }

    void Drawable::clear() {
        delete vao_;
        vao_ = nullptr;

        VertexArrayObject::release_buffer(vertex_buffer_);
        VertexArrayObject::release_buffer(color_buffer_);
        VertexArrayObject::release_buffer(normal_buffer_);
        VertexArrayObject::release_buffer(texcoord_buffer_);
        VertexArrayObject::release_buffer(element_buffer_);
        VertexArrayObject::release_buffer(storage_buffer_);
        VertexArrayObject::release_buffer(selection_buffer_);
    }


    void Drawable::release_element_buffer() {
        VertexArrayObject::release_buffer(element_buffer_);
    }


    void Drawable::buffer_stats(std::ostream &output) const {
        std::cout << "\t" << name() << std::endl;

        if (vertex_buffer()) {
            output << "\t\tVertex buffer:     " << num_vertices_ << " vertices, "
                   << num_vertices_ * sizeof(vec3) << " bytes" << std::endl;
        }
        if (normal_buffer()) {
            output << "\t\tNormal buffer:     " << num_vertices_ << " normals, "
                   << num_vertices_ * sizeof(vec3) << " bytes" << std::endl;
        }
        if (color_buffer()) {
            output << "\t\tColor buffer:      " << num_vertices_ << " colors, "
                   << num_vertices_ * sizeof(vec3) << " bytes" << std::endl;
        }
        if (texcoord_buffer()) {
            output << "\t\tTexcoord buffer:   " << num_vertices_ << " texcoords, "
                   << num_vertices_ * sizeof(vec2) << " bytes" << std::endl;
        }
        if (element_buffer()) {
            output << "\t\tIndex buffer:      " << num_indices_ << " indices, "
                   << num_indices_ * sizeof(unsigned int) << " bytes" << std::endl;
        }
    }


    void Drawable::update_storage_buffer(const void *data, std::size_t datasize, unsigned int index /* = 1*/) {
        assert(vao_);

        if (storage_buffer_ == 0 || datasize != current_storage_buffer_size_) {
            bool success = vao_->create_storage_buffer(storage_buffer_, index, data, datasize);
            LOG_IF(ERROR, !success) << "failed creating storage buffer";
        } else {
            bool success = vao_->update_storage_buffer(storage_buffer_, 0, datasize, data);
            LOG_IF(ERROR, !success) << "failed updating storage buffer";
        }
    }


    void Drawable::update_selection_buffer(unsigned int index/* = 1*/) {
        assert(vao_);

        //if (selection_buffer_ == 0 || selections_.size() != current_selection_buffer_size_) {
        //       bool status = vao_->create_storage_buffer(selection_buffer_, index, selections_.data(), selections_.size() * sizeof(int));
        //	if (!status)
        //		LOG(ERROR) << "failed creating selection buffer";
        //	else {
        //		current_selection_buffer_size_ = selections_.size();
        //		if (t.elapsed_seconds() > 0.1) {
        //			LOG(INFO) << "selection buffer updated. " << t.time_string();
        //		}
        //	}
        //}
        //else {
        //       bool status = vao_->update_storage_buffer(selection_buffer_, 0, selections_.size() * sizeof(int), selections_.data());
        //	if (!status)
        //		LOG(ERROR) << "failed updating selection buffer";
        //	else {
        //		if (t.elapsed_seconds() > 0.1) {
        //			LOG(INFO) << "selection buffer updated. " << t.time_string();
        //		}
        //	}
        //}
    }


    void Drawable::update_buffers() {
        update_requested_ = true;
    }


    void Drawable::internal_update_buffers() {
        if (!model_ && !update_func_) {
            LOG(ERROR) << "failed updating buffers: drawable not associated with a model and no update function has been specified.";
            return;
        }

        StopWatch w;

        if (update_func_)
            update_func_(model_, this);
        else
            renderer::update_buffers(model_, this);

        LOG_IF(INFO, w.elapsed_seconds() > 1.0) << "update rendering buffers took " << w.time_string();
        
        update_requested_ = false;
    }


    void Drawable::update_vertex_buffer(const std::vector<vec3> &vertices) {
        assert(vao_);

        bool success = vao_->create_array_buffer(vertex_buffer_, ShaderProgram::POSITION, vertices.data(),
                                                 vertices.size() * sizeof(vec3), 3);

        LOG_IF(ERROR, !success) << "failed creating vertex buffer";

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
                    bbox_.add_point(p);
            }
        }
    }


    void Drawable::update_color_buffer(const std::vector<vec3> &colors) {
        assert(vao_);

        bool success = vao_->create_array_buffer(color_buffer_, ShaderProgram::COLOR, colors.data(),
                                                 colors.size() * sizeof(vec3), 3);
        LOG_IF(ERROR, !success) << "failed updating color buffer";
    }


    void Drawable::update_normal_buffer(const std::vector<vec3> &normals) {
        assert(vao_);
        bool success = vao_->create_array_buffer(normal_buffer_, ShaderProgram::NORMAL, normals.data(),
                                                 normals.size() * sizeof(vec3), 3);
        LOG_IF(ERROR, !success) << "failed updating normal buffer";
    }


    void Drawable::update_texcoord_buffer(const std::vector<vec2> &texcoords) {
        assert(vao_);

        bool success = vao_->create_array_buffer(texcoord_buffer_, ShaderProgram::TEXCOORD, texcoords.data(),
                                                 texcoords.size() * sizeof(vec2), 2);
        LOG_IF(ERROR, !success) << "failed updating texcoord buffer";
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


    void Drawable::fetch_selection_buffer() {
        //    vao_->get_buffer_data(GL_SHADER_STORAGE_BUFFER, selection_buffer_, 0, selections_.size() * sizeof(uint32_t), selections_.data());

        //starting from OpenGL 4.5, you can use the simpler version
        //vao_->get_named_buffer_data(selection_buffer_, 0, selections_.size() * sizeof(uint32_t), selections_.data());
    }


    void Drawable::gl_draw(bool with_storage_buffer /* = false */) const {
        if (update_requested_ || vertex_buffer_ == 0)
            const_cast<Drawable*>(this)->internal_update_buffers();

        vao_->bind();

        if (with_storage_buffer) {
            // Liangliang: I made stupid mistake here (confused by glBindBuffer() and glBindBufferBase())
            //glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssb);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, selection_buffer_);
            easy3d_debug_log_gl_error;

            GLbitfield barriers = GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT;
            if (element_buffer_ != 0)
                barriers |= GL_ELEMENT_ARRAY_BARRIER_BIT;

            glMemoryBarrier(barriers);
            easy3d_debug_log_gl_error;
        }

        if (element_buffer_) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_buffer_);	easy3d_debug_log_gl_error;

            // index buffer must be bound if using glDrawElements()
            glDrawElements(type(), GLsizei(num_indices_), GL_UNSIGNED_INT, nullptr);    easy3d_debug_log_gl_error;
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);	easy3d_debug_log_gl_error;
        } else
            glDrawArrays(type(), 0, GLsizei(num_vertices_));
        easy3d_debug_log_gl_error;

        if (with_storage_buffer) {
            // Liangliang: I made stupid mistake here (confused by glBindBuffer() and glBindBufferBase())
            //glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);
            easy3d_debug_log_gl_error;
        }

        vao_->release();
        easy3d_debug_log_gl_error;
    }

}
