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
#include <stdexcept>

#include <easy3d/core/model.h>
#include <easy3d/renderer/buffer.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/setting.h>
#include <easy3d/util/stop_watch.h>


namespace easy3d {

    void Drawable::enable_cpu_buffers() {
        if (vertex_buffer_ || normal_buffer_ || color_buffer_ || texcoord_buffer_ || element_buffer_)
            throw std::logic_error("cannot transfer an uploaded OpenGL drawable to CPU buffers");
        if (!cpu_buffers_) cpu_buffers_.reset(new BufferData);
        cpu_only_ = true;
    }

    const Drawable::BufferData& Drawable::cpu_buffers() {
        if (!cpu_buffers_) throw std::logic_error("CPU buffers were not enabled");
        if (update_needed_ || (!num_vertices_ && (model_ || update_func_))) {
            update_buffers_internal();
            update_needed_ = false;
        }
        return *cpu_buffers_;
    }

    Drawable::Drawable(const std::string &name, Model *model)
            : name_(name), model_(model), vao_(nullptr), num_vertices_(0), num_indices_(0),
              update_needed_(false), update_func_(nullptr), vertex_buffer_(0), color_buffer_(0), normal_buffer_(0),
              texcoord_buffer_(0), element_buffer_(0), manipulator_(nullptr) {
#ifdef EASY3D_RENDERER_OPENGL
        opengl_initialize();
#else
        enable_cpu_buffers();
#endif
        material_ = Material(setting::material_ambient, setting::material_specular, setting::material_shininess);
    }


    Drawable::~Drawable() {
        clear();
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
        if (cpu_buffers_) {
            const auto revision = cpu_buffers_->revision + 1;
            *cpu_buffers_ = BufferData();
            cpu_buffers_->revision = revision;
        }
        if (vertex_buffer_ || color_buffer_ || normal_buffer_ || texcoord_buffer_ || element_buffer_)
            opengl_release_buffers();

        num_vertices_ = 0;
        num_indices_ = 0;
        bbox_.clear();
    }


    void Drawable::disable_element_buffer() {
        if (cpu_buffers_) { cpu_buffers_->indices.clear(); ++cpu_buffers_->revision; }
        if (element_buffer_) opengl_disable_element_buffer();
        num_indices_ = 0;
    }


    void Drawable::update_buffers_internal() {
        if (!model_ && !update_func_) {
            LOG_N_TIMES(3, ERROR) << "drawable not associated with a model and no update function provided. " << COUNTER;
            return;
        } else if (model_ && model_->points().empty()) {
            clear();
            LOG_N_TIMES(3, WARNING) << "model has no valid geometry. " << COUNTER;
            return;
        }

        StopWatch w;
        if (cpu_buffers_) {
            // Extraction may change which attributes exist (e.g. removal of a
            // color/normal property). Do not retain arrays from the old snapshot.
            const auto revision = cpu_buffers_->revision + 1;
            *cpu_buffers_ = BufferData();
            cpu_buffers_->revision = revision;
            num_vertices_ = num_indices_ = 0;
        }
        if (update_func_)
            update_func_(model_, this);
        else
            buffer::update(model_, this);

        LOG_IF(w.elapsed_seconds() > 0.5, INFO) << "updating rendering buffers for drawable '" << name()
                                                << "' took " << w.time_string();
    }


    void Drawable::update_vertex_buffer(const std::vector<vec3> &vertices, bool dynamic) {
        if (cpu_buffers_) {
            cpu_buffers_->vertices = vertices;
            ++cpu_buffers_->revision;
            num_vertices_ = vertices.size();
            bbox_.clear();
            for (const auto& p : vertices) bbox_.grow(p);
            return;
        }
        opengl_update_vertex_buffer(vertices, dynamic);
    }

    void Drawable::update_color_buffer(const std::vector<vec3> &colors, bool dynamic) {
        if (cpu_buffers_) { cpu_buffers_->colors = colors; ++cpu_buffers_->revision; return; }
        opengl_update_color_buffer(colors, dynamic);
    }

    void Drawable::update_normal_buffer(const std::vector<vec3> &normals, bool dynamic) {
        if (cpu_buffers_) { cpu_buffers_->normals = normals; ++cpu_buffers_->revision; return; }
        opengl_update_normal_buffer(normals, dynamic);
    }

    void Drawable::update_texcoord_buffer(const std::vector<vec2> &texcoords, bool dynamic) {
        if (cpu_buffers_) { cpu_buffers_->texcoords = texcoords; ++cpu_buffers_->revision; return; }
        opengl_update_texcoord_buffer(texcoords, dynamic);
    }

    void Drawable::update_element_buffer(const std::vector<unsigned int> &indices) {
        if (cpu_buffers_) {
            cpu_buffers_->indices = indices; num_indices_ = indices.size(); ++cpu_buffers_->revision; return;
        }
        opengl_update_element_buffer(indices);
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




    Manipulator* Drawable::manipulator() {
        if (manipulator_)
            return manipulator_.get();
        else if (model_)
            return model_->manipulator();
        else
            return nullptr;
    }


    const Manipulator* Drawable::manipulator() const {
        if (manipulator_)
            return manipulator_.get();
        else if (model_)
            return model_->manipulator();
        else
            return nullptr;
    }


    mat4 Drawable::manipulated_matrix() const {
        if (manipulator_)
            return manipulator_->matrix();
        else if (model_ && model_->manipulator())
            return model_->manipulator()->matrix();
        else
            return mat4::identity();
    }

}
