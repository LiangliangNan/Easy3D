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


#include <easy3d/gui/picker_point_cloud.h>

#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/render_device.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/texture.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/renderer/transform.h>
#ifndef EASY3D_HAS_VULKAN
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/backends/opengl/query_state.h>
#include <easy3d/renderer/framebuffer_object.h>
#endif
#include <easy3d/util/logging.h>
#include <cstdint>
#include <limits>
#include <stdexcept>


namespace easy3d {

namespace {
    // Point clouds get a default points drawable named "vertices" when they are
    // attached to a renderer. Picking always follows this drawable, so the result
    // corresponds to what is actually on screen.
    std::shared_ptr<PointsDrawable> vertices_drawable(const Renderer* renderer) {
        if (renderer)
            for (const auto& drawable : renderer->points_drawables())
                if (drawable->name() == "vertices")
                    return drawable;
        return {};
    }

    // World-space clipping halfspaces matching ClippingPlane::set_program(): keep
    // the negative-normal side, optionally capped into a cross-section slab.
    std::vector<vec4> world_clipping_planes() {
        std::vector<vec4> result;
        const auto* plane = ClippingPlane::instance();
        if (!plane->is_enabled())
            return result;
        const vec3 n = plane->normal(), c = plane->center();
        result.push_back(vec4(-n, dot(n, c)));
        if (plane->cross_section())
            result.push_back(vec4(n, -dot(n, c) + plane->cross_section_width() * plane->scene_radius()));
        return graphics::normalize_clipping_planes(result);
    }

    // Snapshot used when the cloud has no compatible rendering resource. Positions
    // and identity IDs cover every source slot; deleted slots stay NaN so projection
    // and clip tests reject them.
    std::shared_ptr<const graphics::PointData> source_point_data(PointCloud* model) {
        const size_t count = model->points().size();
        if (count > UINT32_MAX)
            throw std::length_error("too many point source slots");
        const float nan = std::numeric_limits<float>::quiet_NaN();
        std::vector<vec3> positions(count, vec3(nan));
        std::vector<uint32_t> ids(count);
        for (size_t i = 0; i < count; ++i)
            ids[i] = uint32_t(i);
        for (auto vertex : model->vertices()) {
            positions[vertex.idx()] = model->position(vertex);
        }
        return std::make_shared<graphics::PointData>(std::move(positions), std::move(ids), uint32_t(count));
    }

    bool supports_point_data_command(const PointsDrawable& drawable) {
        const auto coloring = drawable.coloring_method();
        if (coloring != State::UNIFORM_COLOR && coloring != State::COLOR_PROPERTY &&
            coloring != State::TEXTURED && coloring != State::SCALAR_FIELD)
            return false;
        if (drawable.color().w != 1 || drawable.is_ao_enabled())
            return false;
        if ((coloring == State::TEXTURED || coloring == State::SCALAR_FIELD) &&
            (!drawable.texture() || !drawable.texture()->pixels()))
            return false;
        return drawable.impostor_type() == PointsDrawable::PLAIN ||
               drawable.impostor_type() == PointsDrawable::SPHERE ||
               drawable.impostor_type() == PointsDrawable::SURFEL;
    }
}

    PointCloudPicker::PointCloudPicker(const Camera *cam) : Picker(cam) {}

    PointCloudPicker::~PointCloudPicker() = default;

    PointCloud::Vertex PointCloudPicker::pick_vertex(PointCloud* model, int x, int y) {
        const int width = camera()->screen_width(), height = camera()->screen_height();
        if (!model || model->empty() || x < 0 || y < 0 || x >= width || y >= height)
            return PointCloud::Vertex();
        const auto* renderer = model->renderer();
        if (!renderer || !renderer->is_visible())
            return PointCloud::Vertex();
        const auto drawable = vertices_drawable(renderer);
        if (!drawable || !drawable->is_visible())
            return PointCloud::Vertex();
#ifndef EASY3D_HAS_VULKAN
        // The OpenGL adapter rejects analytic impostor draws that also carry clipping
        // planes (a documented adapter limitation), so clipped impostor picks keep the
        // exact native selection pass until the adapter supports analytic clipping.
        const bool analytic = drawable->impostor_type() != PointsDrawable::PLAIN;
        const bool clipped = ClippingPlane::instance()->is_enabled();
        if (analytic && clipped) {
            const std::string name = "selection/selection_pointcloud_single_point_as_sphere_sprite";
            auto* program = ShaderManager::get_program(name);
            if (!program)
                program = ShaderManager::create_program_from_files(name, {{ShaderProgram::POSITION, "vtx_position"}});
            if (!program)
                throw std::runtime_error("point picking requires the selection shader '" + name + "'");
            GLint viewport[4];
            glGetIntegerv(GL_VIEWPORT, viewport);
            const int w = viewport[2], h = viewport[3];
            if (w <= 0 || h <= 0)
                return PointCloud::Vertex();
            opengl::QueryState state;
            state.prepare(w, h);
            FramebufferObject target(w, h, 0);
            if (!target.add_color_buffer() || !target.add_depth_buffer(GL_DEPTH_COMPONENT32F) ||
                !target.is_valid() || !target.bind())
                return PointCloud::Vertex();
            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glClearColor(1, 1, 1, 1);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_PROGRAM_POINT_SIZE);
            program->bind();
            program->set_uniform("sphere", drawable->impostor_type() != PointsDrawable::PLAIN)
                   ->set_uniform("perspective", camera()->type() == Camera::PERSPECTIVE)
                   ->set_uniform("MV", camera()->model_view_matrix())
                   ->set_uniform("PROJ", camera()->projection_matrix())
                   ->set_uniform("MANIP", drawable->manipulated_matrix())
                   ->set_uniform("point_size", drawable->point_size())
                   ->set_uniform("sphere_radius", drawable->point_size() * camera()->world_pixel_ratio(camera()->pivot_point()) * 0.5f)
                   ->set_uniform("screen_width", w);
            ClippingPlane::instance()->set_program(program);
            drawable->gl_draw();
            int gl_x, gl_y;
            screen_to_opengl(x, y, gl_x, gl_y, w, h);
            unsigned char rgba[4];
            if (!target.read_color(rgba, gl_x, gl_y))
                return PointCloud::Vertex();
            const PointCloud::Vertex picked(color::encode(rgba[0], rgba[1], rgba[2], rgba[3]));
            return model->is_valid(picked) && !model->is_deleted(picked) ? picked : PointCloud::Vertex();
        }
#endif
        // A single pick is a rendered pixel query: rasterize the point drawable
        // with a source-vertex ID in the color target and sample that pixel.
        const auto command = draw_command(drawable, 0);
        const auto hit = render_device().sample_pixel(width, height, {command}, x, y).id;
        const auto& ids = command.geometry->point_data()->source_ids();
        if (hit.object != 0 || hit.primitive >= ids.size() || ids[hit.primitive] >= model->points().size())
            return PointCloud::Vertex();
        const PointCloud::Vertex vertex(ids[hit.primitive]);
        return model->is_deleted(vertex) ? PointCloud::Vertex() : vertex;
    }

    void PointCloudPicker::pick_vertices(PointCloud* model, const Rect& rect, bool deselect) {
        Polygon2 polygon;
        polygon.push_back(vec2(rect.left(), rect.top()));
        polygon.push_back(vec2(rect.right(), rect.top()));
        polygon.push_back(vec2(rect.right(), rect.bottom()));
        polygon.push_back(vec2(rect.left(), rect.bottom()));
        pick_vertices(model, polygon, deselect);
    }

    void PointCloudPicker::pick_vertices(PointCloud* model, const Polygon2& polygon, bool deselect) {
        const int width = camera()->screen_width(), height = camera()->screen_height();
        if (!model || model->empty() || polygon.size() < 3 || width <= 0 || height <= 0)
            return;
        const auto* renderer = model->renderer();
        if (renderer && !renderer->is_visible())
            return;
        const auto drawable = vertices_drawable(renderer);
        if (drawable && !drawable->is_visible())
            return;

        const mat4 transform = drawable ? drawable->manipulated_matrix() :
            model->manipulator() ? model->manipulator()->matrix() : mat4::identity();

        // Region selection projects each point center and sets its bit in the packed
        // "v:select" mask. It is a through-selection: depth does not matter, so points
        // behind other geometry are still selected.
        graphics::PointSelectionRequest request;
        if (drawable && supports_point_data_command(*drawable))
            request.points = draw_command(drawable, 0).geometry->point_data();
        if (!request.points || request.points->source_count() != model->points().size())
            request.points = source_point_data(model);
        request.width = width;
        request.height = height;
        request.polygon.assign(polygon.begin(), polygon.end());
        request.model_view_projection = camera()->model_view_projection_matrix() * transform;
        request.deselect = deselect;
        // Clipping is stored in world space; the shaders test positions before
        // the model transform, so move each plane into the drawable's local frame.
        for (const auto& plane : world_clipping_planes())
            request.local_clipping_planes.push_back(transpose(transform) * plane);

        auto selected = model->vertex_property<bool>("v:select");
        request.words.resize((size_t(request.points->source_count()) + 31) / 32, 0);
        for (auto vertex : model->vertices()) {
            const auto id = vertex.idx();
            if (id < request.points->source_count() && selected[vertex])
                request.words[id >> 5] |= 1u << (id & 31);
        }

        const auto words = render_device().select_points(request);
        size_t count = 0;
        for (auto vertex : model->vertices()) {
            const auto id = vertex.idx();
            const bool bit = id < request.points->source_count() &&
                (words[id >> 5] & (1u << (id & 31))) != 0;
            selected[vertex] = bit;
            if (bit)
                ++count;
        }
        LOG(INFO) << "current selection: " << count << " points";
    }
}
