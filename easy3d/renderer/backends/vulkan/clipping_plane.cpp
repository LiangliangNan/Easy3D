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

#include "clipping_plane.h"
#include <easy3d/renderer/shape.h>
#include <easy3d/renderer/clipping_plane.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/manipulated_frame.h>

namespace easy3d {
    namespace vulkan {
        std::vector<vec4> ClippingPlaneRenderer::planes(const ClippingPlane &plane) {
            if (!plane.is_enabled())
                return {};
            const vec3 n = plane.normal(), c = plane.center();
            // Match ClippingPlane::set_program(): keep the negative-normal halfspace,
            // or a slab ending at the plane, rather than the opposite public Plane3.
            std::vector<vec4> result{vec4(-n, dot(n, c))};
            if (plane.cross_section())
                result.push_back(vec4(n, -dot(n, c) + plane.cross_section_width() * plane.scene_radius()));
            return graphics::normalize_clipping_planes(result);
        }
        std::vector<graphics::DrawCommand> ClippingPlaneRenderer::draw(const ClippingPlane &plane,
                                                                       const Camera &camera) {
            if (!plane.is_enabled() || !plane.is_visible())
                return {};
            const float r = plane.scene_radius();
            const bool geometry_changed = !face_ || radius_ != r;
            const std::vector<vec3> corners{vec3(-r, -r, 0), vec3(r, -r, 0), vec3(r, r, 0), vec3(-r, r, 0)};
            if (geometry_changed) {
                std::vector<graphics::Vertex> vertices;
                for (unsigned i : {0u, 1u, 2u, 0u, 2u, 3u}) {
                    graphics::Vertex vertex;
                    vertex.position = corners[i];
                    vertices.push_back(vertex);
                }
                face_ = std::make_shared<graphics::Geometry>(std::move(vertices));
                radius_ = r;
            }
            const mat4 mvp = camera.model_view_projection_matrix() * plane.manipulator()->frame()->matrix();
            bool changed = geometry_changed || width_ != camera.screen_width() || height_ != camera.screen_height();
            for (unsigned i = 0; i < 16; ++i)
                changed = changed || wire_mvp_[i] != mvp[i];
            if (changed) {
                const std::vector<vec3> edges{corners[0], corners[1], corners[1], corners[2],
                                              corners[2], corners[3], corners[3], corners[0]};
                wire_ = graphics::shape::draw_lines_3d(edges, std::vector<vec3>(8, vec3(1)), mvp, camera.screen_width(),
                                                       camera.screen_height());
                wire_mvp_ = mvp;
                width_ = camera.screen_width();
                height_ = camera.screen_height();
            }
            graphics::DrawCommand fill;
            fill.geometry = face_;
            fill.model_view_projection = mvp;
            fill.color = plane.color();
            fill.overlay = fill.overlay_depth_test = true;
            auto wire = wire_;
            wire.color = plane.color();
            return {fill, wire};
        }
    } // namespace vulkan
} // namespace easy3d
