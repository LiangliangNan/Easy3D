/** \cond LICENSE
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
 \endcond */

#include <easy3d/renderer/drawable.h>

#include <algorithm>
#include <cassert>
#include <stdexcept>

#include <easy3d/core/model.h>
#include <easy3d/renderer/buffer.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/util/logging.h>
#include <easy3d/util/setting.h>
#include <easy3d/util/stop_watch.h>
#ifndef EASY3D_HAS_VULKAN
#include <easy3d/renderer/opengl.h>
#endif


namespace easy3d {

    float Drawable::impostor_radius(const Camera& camera, float diameter) {
        // Liangliang: this doesn't work when zooming puts the pivot in or behind the eye plane
        // return diameter * camera.world_pixel_ratio(camera.pivot_point()) * 0.5f;
        // Fixed on 2026-11-16:
        // Zooming can put the pivot in or behind the eye plane, where the world-per-pixel
        // ratio degenerates to zero. Nothing nearer than the near plane is visible, so that
        // depth is the floor: the radius must stay finite and positive on both backends.
        const float ratio = std::max(camera.world_pixel_ratio(camera.pivot_point()),
                                     camera.world_pixel_ratio(camera.position() +
                                                              camera.view_direction() * camera.z_near()));
        return diameter * ratio * 0.5f;
    }

#ifndef EASY3D_HAS_VULKAN
    float Drawable::viewport_pixel_scale(const Camera& camera) {
        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        const int w = camera.screen_width();
        return w > 0 ? float(viewport[2]) / float(w) : 1.0f;
    }
#endif

    unsigned int Drawable::vertex_buffer() const {
#ifdef EASY3D_HAS_VULKAN
        return 0;
#else
        const_cast<Drawable*>(this)->opengl_realize_buffers();
        return vertex_buffer_;
#endif
    }

    unsigned int Drawable::color_buffer() const {
#ifdef EASY3D_HAS_VULKAN
        return 0;
#else
        const_cast<Drawable*>(this)->opengl_realize_buffers();
        return color_buffer_;
#endif
    }

    unsigned int Drawable::normal_buffer() const {
#ifdef EASY3D_HAS_VULKAN
        return 0;
#else
        const_cast<Drawable*>(this)->opengl_realize_buffers();
        return normal_buffer_;
#endif
    }

    unsigned int Drawable::texcoord_buffer() const {
#ifdef EASY3D_HAS_VULKAN
        return 0;
#else
        const_cast<Drawable*>(this)->opengl_realize_buffers();
        return texcoord_buffer_;
#endif
    }

    unsigned int Drawable::element_buffer() const {
#ifdef EASY3D_HAS_VULKAN
        return 0;
#else
        const_cast<Drawable*>(this)->opengl_realize_buffers();
        return element_buffer_;
#endif
    }

    void Drawable::set_source_faces(const std::vector<int>& faces) {
        if (type() != DT_TRIANGLES)
            throw std::logic_error("face provenance requires triangle buffers");
        const auto count = (cpu_buffers_->indices.empty() ? cpu_buffers_->vertices.size()
                                                          : cpu_buffers_->indices.size()) / 3;
        if (!faces.empty() && faces.size() != count)
            throw std::invalid_argument("face provenance count does not match triangles");
        for (auto id : faces)
            if (id < -1)
                throw std::invalid_argument("invalid source face index");
        cpu_buffers_->source_faces = faces;
        ++cpu_buffers_->revision;
    }

    const Drawable::BufferData& Drawable::cpu_buffers() {
        if (update_needed_ || (!buffers_ready_ && (model_ || update_func_))) {
            update_buffers_internal();
            update_needed_ = false;
        }
        return *cpu_buffers_;
    }

    const Drawable::BufferData& Drawable::buffer_data() const {
        return const_cast<Drawable*>(this)->cpu_buffers();
    }

    Drawable::Drawable(const std::string &name, Model *model)
            : name_(name), model_(model), vao_(nullptr), num_vertices_(0), num_indices_(0),
              update_needed_(false), buffers_ready_(false), update_func_(nullptr), vertex_buffer_(0),
              color_buffer_(0), normal_buffer_(0), texcoord_buffer_(0), element_buffer_(0),
              highlight_texture_(0), highlight_primitives_(0), manipulator_(nullptr) {
#ifndef EASY3D_HAS_VULKAN
        opengl_initialize();
#endif
        cpu_buffers_.reset(new BufferData);
        material_ = Material(setting::material_ambient, setting::material_specular, setting::material_shininess);
    }


    Drawable::~Drawable() {
        clear();
    }


    void Drawable::set_model(Model *m) {
        if (model_ == m)
            return;
        model_ = m;
        buffers_ready_ = false;
        update_needed_ = m != nullptr;
        if (!m)
            clear();
    }


    const Box3 &Drawable::bounding_box() const {
        if (model_)
            return model_->bounding_box();
        else
            return bbox_;
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
#ifndef EASY3D_HAS_VULKAN
        ++point_revision_;
        ++buffer_revision_;
        opengl_snapshot_.reset();
        opengl_upload_needed_ = true;
        if (vertex_buffer_ || color_buffer_ || normal_buffer_ || texcoord_buffer_ || element_buffer_)
            opengl_release_buffers();
#endif
        client_colors_ = client_normals_ = client_texcoords_ = client_indices_ = false;
        reset_highlight_mask();
        num_vertices_ = 0;
        num_indices_ = 0;
        buffers_ready_ = false;
        bbox_.clear();
    }


    void Drawable::reset_highlight_mask() {
        highlight_strengths_.clear();
#ifndef EASY3D_HAS_VULKAN
        highlight_primitives_ = 0;
        opengl_highlight_needed_ = true;
#endif
    }


    void Drawable::update_highlight_mask(const std::vector<float> &strengths) {
        const std::size_t stride = type() == DT_TRIANGLES ? 3 : type() == DT_LINES ? 2 : 1;
        const std::size_t base = num_indices_ ? num_indices_ : num_vertices_;
        const std::size_t primitives = base / stride;
        if (!strengths.empty()) {
            if (strengths.size() != primitives)
                throw std::invalid_argument("highlight mask must have one strength per rendered primitive");
            for (const float s : strengths)
                if (!(s >= 0.0f && s <= 1.0f))
                    throw std::invalid_argument("highlight strengths must lie in [0, 1]");
        }
        if (strengths == highlight_strengths_)
            return;
        highlight_strengths_ = strengths;
        ++cpu_buffers_->revision;
#ifndef EASY3D_HAS_VULKAN
        opengl_highlight_needed_ = true;
#endif
    }


    void Drawable::disable_element_buffer() {
        cpu_buffers_->indices.clear();
        cpu_buffers_->source_faces.clear();
        if (!realizing_) client_indices_ = true;
        ++cpu_buffers_->revision;
#ifndef EASY3D_HAS_VULKAN
        ++point_revision_;
        ++buffer_revision_;
        opengl_snapshot_.reset();
        opengl_upload_needed_ = true;
#endif
        num_indices_ = 0;
        reset_highlight_mask();
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
        const bool keep_colors = client_colors_;
        const bool keep_normals = client_normals_;
        const bool keep_texcoords = client_texcoords_;
        const bool keep_indices = client_indices_;
        BufferData pending;
        if (keep_colors) pending.colors = cpu_buffers_->colors;
        if (keep_normals) pending.normals = cpu_buffers_->normals;
        if (keep_texcoords) pending.texcoords = cpu_buffers_->texcoords;
        if (keep_indices) pending.indices = cpu_buffers_->indices;
        const auto revision = cpu_buffers_->revision + 1;
        *cpu_buffers_ = BufferData();
        cpu_buffers_->revision = revision;
        num_vertices_ = num_indices_ = 0;
        realizing_ = true;
        if (update_func_)
            update_func_(model_, this);
        else
            buffer::update(model_, this);
        realizing_ = false;
        if (keep_colors) {
            cpu_buffers_->colors = std::move(pending.colors);
            ++cpu_buffers_->revision;
        }
        if (keep_normals) {
            cpu_buffers_->normals = std::move(pending.normals);
            ++cpu_buffers_->revision;
        }
        if (keep_texcoords) {
            cpu_buffers_->texcoords = std::move(pending.texcoords);
            ++cpu_buffers_->revision;
        }
        if (keep_indices) {
            cpu_buffers_->indices = std::move(pending.indices);
            num_indices_ = cpu_buffers_->indices.size();
            ++cpu_buffers_->revision;
        }
        buffers_ready_ = true;
#ifndef EASY3D_HAS_VULKAN
        opengl_upload_needed_ = true;
#endif

        LOG_IF(w.elapsed_seconds() > 0.5, INFO) << "updating rendering buffers for drawable '" << name()
                                                << "' took " << w.time_string();
    }


    void Drawable::update_vertex_buffer(const std::vector<vec3> &vertices, bool dynamic) {
        update_needed_ = false;
        cpu_buffers_->vertices = vertices;
        cpu_buffers_->source_faces.clear();
        ++cpu_buffers_->revision;
        num_vertices_ = vertices.size();
        bbox_.clear();
        if (model_)
            bbox_ = model_->bounding_box();
        else
            for (const auto &p : vertices)
                bbox_.grow(p);
#ifndef EASY3D_HAS_VULKAN
        ++point_revision_;
        ++buffer_revision_;
        opengl_snapshot_.reset();
        opengl_upload_needed_ = true;
        dynamic_vertices_ = dynamic;
#endif
        buffers_ready_ = true;
        reset_highlight_mask();
    }

    void Drawable::update_color_buffer(const std::vector<vec3> &colors, bool dynamic) {
        cpu_buffers_->colors = colors;
        if (!realizing_) client_colors_ = true;
        ++cpu_buffers_->revision;
#ifndef EASY3D_HAS_VULKAN
        ++buffer_revision_;
        opengl_snapshot_.reset();
        opengl_upload_needed_ = true;
        dynamic_colors_ = dynamic;
#endif
    }

    void Drawable::update_normal_buffer(const std::vector<vec3> &normals, bool dynamic) {
        cpu_buffers_->normals = normals;
        if (!realizing_) client_normals_ = true;
        ++cpu_buffers_->revision;
#ifndef EASY3D_HAS_VULKAN
        ++buffer_revision_;
        opengl_snapshot_.reset();
        opengl_upload_needed_ = true;
        dynamic_normals_ = dynamic;
#endif
    }

    void Drawable::update_texcoord_buffer(const std::vector<vec2> &texcoords, bool dynamic) {
        cpu_buffers_->texcoords = texcoords;
        if (!realizing_) client_texcoords_ = true;
        ++cpu_buffers_->revision;
#ifndef EASY3D_HAS_VULKAN
        ++buffer_revision_;
        opengl_snapshot_.reset();
        opengl_upload_needed_ = true;
        dynamic_texcoords_ = dynamic;
#endif
    }

    void Drawable::update_element_buffer(const std::vector<unsigned int> &indices) {
        cpu_buffers_->indices = indices;
        cpu_buffers_->source_faces.clear();
        if (!realizing_) client_indices_ = true;
        num_indices_ = indices.size();
        ++cpu_buffers_->revision;
#ifndef EASY3D_HAS_VULKAN
        ++point_revision_;
        ++buffer_revision_;
        opengl_snapshot_.reset();
        opengl_upload_needed_ = true;
#endif
        reset_highlight_mask();
    }

    void Drawable::update_element_buffer(const std::vector< std::vector<unsigned int> > &indices) {
        assert(cpu_buffers_ || vao_);

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

#ifdef EASY3D_HAS_VULKAN
namespace easy3d {
void Drawable::gl_draw() const {
    throw std::logic_error("gl_draw() is unavailable in Vulkan; render through Viewer or OffScreen");
}
}
#endif
