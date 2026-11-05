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


#include <easy3d/gui/picker_point_cloud.h>

#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/clipping_plane.h>
#ifdef EASY3D_HAS_VULKAN
#include <easy3d/renderer/backends/vulkan/clipping_plane.h>
#include <stdexcept>
#else
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/backends/opengl/query_state.h>
#include <easy3d/renderer/framebuffer_object.h>
#endif
#include <easy3d/util/logging.h>


namespace easy3d {


    PointCloudPicker::PointCloudPicker(const Camera *cam)
            : Picker(cam)
            , hit_resolution_(15)
    {
        use_gpu_if_supported_ = true;
    }


#ifdef EASY3D_HAS_VULKAN
    namespace {
        std::shared_ptr<PointsDrawable> vertices_drawable(const Renderer* renderer) {
            if (renderer)
                for (const auto& drawable : renderer->points_drawables())
                    if (drawable->name() == "vertices")
                        return drawable;
            return {};
        }
    }
#endif

    PointCloud::Vertex PointCloudPicker::pick_vertex(PointCloud* model, int x, int y) {
        const int width = camera()->screenWidth(), height = camera()->screenHeight();
        if (!model || model->empty() || x < 0 || y < 0 || x >= width || y >= height)
            return PointCloud::Vertex();
        const auto* renderer = model->renderer();
        if (!renderer || !renderer->is_visible())
            return PointCloud::Vertex();
#ifdef EASY3D_HAS_VULKAN
        const auto drawable = vertices_drawable(renderer);
        if (!drawable || !drawable->is_visible())
            return PointCloud::Vertex();
        const auto command = draw_command(drawable, 0);
        const auto frame = render_device().render(width, height, {command});
        const auto hit = frame.ids[size_t(y) * width + x];
        const auto& ids = command.geometry->point_data()->source_ids();
        if (hit.object != 0 || hit.primitive >= ids.size() || ids[hit.primitive] >= model->points().size())
            return PointCloud::Vertex();
        const PointCloud::Vertex vertex(ids[hit.primitive]);
        return model->is_deleted(vertex) ? PointCloud::Vertex() : vertex;
#else
        auto drawable = renderer->get_points_drawable("vertices");
        if (!drawable || !drawable->is_visible())
            return PointCloud::Vertex();

        if (!use_gpu_if_supported_)
            return pick_vertex_cpu(model, x, y);

        const std::string name = "selection/selection_pointcloud_single_point_as_sphere_sprite";
        auto* program = ShaderManager::get_program(name);
        if (!program)
            program = ShaderManager::create_program_from_files(name, {{ShaderProgram::POSITION, "vtx_position"}});
        if (!program) {
            use_gpu_if_supported_ = false;
            return pick_vertex_cpu(model, x, y);
        }

        GLint viewport[4];
        glGetIntegerv(GL_VIEWPORT, viewport);
        const int w = viewport[2], h = viewport[3];
        if (w <= 0 || h <= 0)
            return PointCloud::Vertex();
        opengl::QueryState state;
        state.prepare(w, h);
        setup_framebuffer(w, h);
        if (!fbo_->bind())
            return PointCloud::Vertex();
        glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
        glClearColor(1, 1, 1, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_PROGRAM_POINT_SIZE);

        program->bind();
        program->set_uniform("sphere", drawable->impostor_type() != PointsDrawable::PLAIN)
               ->set_uniform("perspective", camera()->type() == Camera::PERSPECTIVE)
               ->set_uniform("MV", camera()->modelViewMatrix())
               ->set_uniform("PROJ", camera()->projectionMatrix())
               ->set_uniform("MANIP", drawable->manipulated_matrix())
               ->set_uniform("point_size", drawable->point_size())
               ->set_uniform("sphere_radius", drawable->point_size() * camera()->pixelGLRatio(camera()->pivotPoint()) * 0.5f)
               ->set_uniform("screen_width", w);
        ClippingPlane::instance()->set_program(program);
        drawable->gl_draw();

        int gl_x, gl_y;
        screen_to_opengl(x, y, gl_x, gl_y, w, h);
        unsigned char rgba[4];
        if (!fbo_->read_color(rgba, gl_x, gl_y))
            return PointCloud::Vertex();
        const PointCloud::Vertex vertex(color::encode(rgba[0], rgba[1], rgba[2], rgba[3]));
        return model->is_valid(vertex) && !model->is_deleted(vertex) ? vertex : PointCloud::Vertex();
#endif
    }

#ifndef EASY3D_HAS_VULKAN
    PointCloud::Vertex PointCloudPicker::pick_vertex_cpu(PointCloud* model, int x, int y) {
        const mat4 manip = model->manipulator() ? model->manipulator()->matrix() : mat4::identity();
        const float resolution = float(hit_resolution_);
        PointCloud::Vertex closest;
        float closest_depth = 1.0f;
        for (auto vertex : model->vertices()) {
            const auto pixel = project(manip * model->position(vertex));
            if (pixel.z >= 0 && pixel.z < closest_depth &&
                distance2(vec2(pixel.x, pixel.y), vec2(float(x), float(y))) <= resolution * resolution) {
                closest = vertex;
                closest_depth = pixel.z;
            }
        }
        return closest;
    }
#else
    void PointCloudPicker::pick_vertices(PointCloud* model, const Rect& rect, bool deselect) {
        Polygon2 polygon;
        polygon.push_back(vec2(rect.left(), rect.top()));
        polygon.push_back(vec2(rect.right(), rect.top()));
        polygon.push_back(vec2(rect.right(), rect.bottom()));
        polygon.push_back(vec2(rect.left(), rect.bottom()));
        pick_vertices(model, polygon, deselect);
    }

    void PointCloudPicker::pick_vertices(PointCloud* model, const Polygon2& polygon, bool deselect) {
        const int width = camera()->screenWidth(), height = camera()->screenHeight();
        if (!model || model->empty() || polygon.size() < 3 || width <= 0 || height <= 0)
            return;
        const auto drawable = vertices_drawable(model->renderer());
        const mat4 transform = drawable ? drawable->manipulated_matrix() :
            model->manipulator() ? model->manipulator()->matrix() : mat4::identity();
        graphics::PointSelectionRequest request;
        if (drawable)
            request.points = draw_command(drawable, 0).geometry->point_data();

        // A cloud used independently of a renderer has no drawing buffer to share.
        if (!request.points || request.points->source_count() != model->points().size()) {
            std::vector<vec3> points;
            std::vector<uint32_t> ids;
            points.reserve(model->n_vertices());
            ids.reserve(model->n_vertices());
            for (auto vertex : model->vertices()) {
                points.push_back(model->position(vertex));
                ids.push_back(vertex.idx());
            }
            if (model->points().size() > UINT32_MAX)
                throw std::length_error("too many point source slots");
            request.points = std::make_shared<graphics::PointData>(
                std::move(points), std::move(ids), uint32_t(model->points().size()));
        }
        request.width = width;
        request.height = height;
        request.polygon.assign(polygon.begin(), polygon.end());
        request.model_view_projection = camera()->modelViewProjectionMatrix() * transform;
        request.deselect = deselect;
        for (const auto& plane : vulkan::ClippingPlaneRenderer::planes(*ClippingPlane::instance()))
            request.local_clipping_planes.push_back(transpose(transform) * plane);
        auto selected = model->vertex_property<bool>("v:select");
        request.words.resize((size_t(request.points->source_count()) + 31) / 32);
        for (auto vertex : model->vertices())
            if (selected[vertex])
                request.words[vertex.idx() >> 5] |= 1u << (vertex.idx() & 31);
        const auto words = render_device().select_points(request);
        for (auto vertex : model->vertices())
            selected[vertex] = (words[vertex.idx() >> 5] & (1u << (vertex.idx() & 31))) != 0;
    }
#endif

#ifndef EASY3D_HAS_VULKAN
    void PointCloudPicker::pick_vertices(PointCloud *model, const Rect& rect, bool deselect) {
        const int win_width = camera()->screenWidth(), win_height = camera()->screenHeight();
        if (!model || win_width < 2 || win_height < 2)
            return;

        float xmin = rect.left() / static_cast<float>(win_width - 1);
        float ymin = 1.0f - rect.top() / static_cast<float>(win_height - 1);
        float xmax = rect.right() / static_cast<float>(win_width - 1);
        float ymax = 1.0f - rect.bottom() / static_cast<float>(win_height - 1);
        if (xmin > xmax) std::swap(xmin, xmax);
        if (ymin > ymax) std::swap(ymin, ymax);

        const auto &points = model->get_vertex_property<vec3>("v:point").vector();
        const mat4 MANIP = model->manipulator() ? model->manipulator()->matrix() : mat4::identity();
        const mat4 &m = camera()->modelViewProjectionMatrix() * MANIP;

        auto &select = model->vertex_property<bool>("v:select").vector();

        for (auto vertex : model->vertices()) {
            const auto i = vertex.idx();
            const vec3 &p = points[i];
            float x = m[0] * p.x + m[4] * p.y + m[8] * p.z + m[12];
            float y = m[1] * p.x + m[5] * p.y + m[9] * p.z + m[13];
            //float z = m[2] * p.x + m[6] * p.y + m[10] * p.z + m[14]; // I don't need z
            float w = m[3] * p.x + m[7] * p.y + m[11] * p.z + m[15];
            x /= w;
            y /= w;
            x = 0.5f * x + 0.5f;
            y = 0.5f * y + 0.5f;

            if (x >= xmin && x <= xmax && y >= ymin && y <= ymax)
                select[i] = !deselect;
        }

        auto count = std::count(select.begin(), select.end(), true);
        LOG(INFO) << "current selection: " << count << " points";
    }


    void PointCloudPicker::pick_vertices(PointCloud *model, const Polygon2 &plg, bool deselect) {
        const int win_width = camera()->screenWidth(), win_height = camera()->screenHeight();
        if (!model || plg.size() < 3 || win_width < 2 || win_height < 2)
            return;

        std::vector<vec2> region; // the transformed selection region
        for (std::size_t i = 0; i < plg.size(); ++i) {
            const vec2 &p = plg[i];
            const float x = p.x / static_cast<float>(win_width - 1);
            const float y = 1.0f - p.y / static_cast<float>(win_height - 1);
            region.emplace_back(vec2(x, y));
        }

        const Box2& box = plg.bbox();
        float xmin = box.min_point().x / static_cast<float>(win_width - 1);
        float ymin = 1.0f - box.min_point().y / static_cast<float>(win_height - 1);
        float xmax = box.max_point().x / static_cast<float>(win_width - 1);
        float ymax = 1.0f - box.max_point().y / static_cast<float>(win_height - 1);
        if (xmin > xmax) std::swap(xmin, xmax);
        if (ymin > ymax) std::swap(ymin, ymax);

        const auto &points = model->get_vertex_property<vec3>("v:point").vector();
        const mat4 MANIP = model->manipulator() ? model->manipulator()->matrix() : mat4::identity();
        const mat4 &m = camera()->modelViewProjectionMatrix() * MANIP;

        auto& select = model->vertex_property<bool>("v:select").vector();

        for (auto vertex : model->vertices()) {
            const auto i = vertex.idx();
            const vec3 &p = points[i];
            float x = m[0] * p.x + m[4] * p.y + m[8] * p.z + m[12];
            float y = m[1] * p.x + m[5] * p.y + m[9] * p.z + m[13];
            //float z = m[2] * p.x + m[6] * p.y + m[10] * p.z + m[14]; // I don't need z
            float w = m[3] * p.x + m[7] * p.y + m[11] * p.z + m[15];
            x /= w;
            y /= w;
            x = 0.5f * x + 0.5f;
            y = 0.5f * y + 0.5f;

            if (x >= xmin && x <= xmax && y >= ymin && y <= ymax) {
                if (geom::point_in_polygon(vec2(x, y), region))
                    select[i] = !deselect;
            }
        }

        auto count = std::count(select.begin(), select.end(), true);
        LOG(INFO) << "current selection: " << count << " points";
    }

#endif

}
