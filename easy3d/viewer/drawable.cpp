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
#include <easy3d/viewer/texture.h>
#include <easy3d/viewer/opengl_error.h>
#include <easy3d/viewer/setting.h>
#include <easy3d/util/logging.h>


namespace easy3d {

    Material::Material()
            : ambient(setting::material_ambient), specular(setting::material_specular),
              shininess(setting::material_shininess) {

    }

    Material::Material(const vec3 &ambi, const vec3 &spec, float shin)
            : ambient(ambi), specular(spec), shininess(shin) {

    }

    Drawable::Drawable(const std::string &name, const Model *model)
            : name_(name)
            , model_(model)
            , visible_(true)
            , per_vertex_color_(true)
            , default_color_(0.8f, 0.8f, 0.8f, 1.0f)
            , lighting_(true)
            , lighting_two_sides_(setting::light_two_sides)
            , distinct_back_color_(setting::light_distinct_back_color)
            , back_color_(setting::light_back_color)
            , highlight_(false)
            , highlight_range_(-1, -1)
            , use_texture_(false)
            , texture_(nullptr)
            , texture_repeat_(1.0f)
            , texture_fractional_repeat_(0.0f)
            , num_vertices_(0)
            , num_indices_(0)
            , vertex_buffer_(0)
            , color_buffer_(0)
            , normal_buffer_(0)
            , texcoord_buffer_(0)
            , index_buffer_(0)
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
        if (vao_) {
            delete vao_;
            vao_ = nullptr;
        }

        if (texture_) {
            delete texture_;
            texture_ = nullptr;
        }

        VertexArrayObject::release_buffer(vertex_buffer_);
        VertexArrayObject::release_buffer(color_buffer_);
        VertexArrayObject::release_buffer(normal_buffer_);
        VertexArrayObject::release_buffer(texcoord_buffer_);
        VertexArrayObject::release_buffer(index_buffer_);
        VertexArrayObject::release_buffer(storage_buffer_);
        VertexArrayObject::release_buffer(selection_buffer_);
    }


    void Drawable::release_element_buffer() {
        VertexArrayObject::release_buffer(index_buffer_);
    }


    void Drawable::drawable_stats() const {
       std::cout << "\t" << name() << std::endl;

        if (vertex_buffer()) {
           std::cout << "\t\tVertex buffer:     " << num_vertices_ << " vertices, "
                      << num_vertices_ * sizeof(vec3) << " bytes" << std::endl;
        }
        if (normal_buffer()) {
           std::cout << "\t\tNormal buffer:     " << num_vertices_ << " normals, "
                      << num_vertices_ * sizeof(vec3) << " bytes" << std::endl;
        }
        if (color_buffer()) {
           std::cout << "\t\tColor buffer:      " << num_vertices_ << " colors, "
                      << num_vertices_ * sizeof(vec3) << " bytes" << std::endl;
        }
        if (texcoord_buffer()) {
           std::cout << "\t\tTexcoord buffer:   " << num_vertices_ << " texcoords, "
                      << num_vertices_ * sizeof(vec2) << " bytes" << std::endl;
        }
        if (index_buffer()) {
           std::cout << "\t\tIndex buffer:      " << num_indices_ << " indices, "
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
        //			LOG(INFO) << "selection buffer updated. time: " << t.time_string();
        //		}
        //	}
        //}
        //else {
        //       bool status = vao_->update_storage_buffer(selection_buffer_, 0, selections_.size() * sizeof(int), selections_.data());
        //	if (!status)
        //		LOG(ERROR) << "failed updating selection buffer";
        //	else {
        //		if (t.elapsed_seconds() > 0.1) {
        //			LOG(INFO) << "selection buffer updated. time: " << t.time_string();
        //		}
        //	}
        //}
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


    void Drawable::update_index_buffer(const std::vector<unsigned int> &indices) {
        assert(vao_);

        bool status = vao_->create_element_buffer(index_buffer_, indices.data(), indices.size() * sizeof(unsigned int));
        if (!status)
            num_indices_ = 0;
        else
            num_indices_ = indices.size();
    }


    void Drawable::fetch_selection_buffer() {
        //    vao_->get_buffer_data(GL_SHADER_STORAGE_BUFFER, selection_buffer_, 0, selections_.size() * sizeof(uint32_t), selections_.data());

        //starting from OpenGL 4.5, you can use the simpler version
        //vao_->get_named_buffer_data(selection_buffer_, 0, selections_.size() * sizeof(uint32_t), selections_.data());
    }


    void Drawable::gl_draw(bool with_storage_buffer /* = false */) const {
        vao_->bind();

        if (with_storage_buffer) {
            // Liangliang: I made stupid mistake here (confused by glBindBuffer() and glBindBufferBase())
            //glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssb);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, selection_buffer_);
            easy3d_debug_log_gl_error;

            GLbitfield barriers = GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT;
            if (index_buffer_ != 0)
                barriers |= GL_ELEMENT_ARRAY_BARRIER_BIT;

            glMemoryBarrier(barriers);
            easy3d_debug_log_gl_error;
        }

        // Primitives like lines and triangles can be drawn without the index buffer provided that
        // all vertices are in order (e.g., f1_v1, f1_v2, f1_v3, f2_v1, f2_v2, f2_v2). This requires
        // the shared vertices be duplicated in the vertex buffer.
        if (index_buffer_) {
            // index buffer must be bound if using glDrawElements()
            glDrawElements(type(), GLsizei(num_indices_), GL_UNSIGNED_INT, nullptr);
            easy3d_debug_log_gl_error;
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
